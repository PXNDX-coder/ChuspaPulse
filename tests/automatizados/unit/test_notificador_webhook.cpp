#include <gtest/gtest.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include "alertas/inotificador.hpp"

using namespace ChuspaPulse::alertas;

namespace {

/// Servidor HTTP local que simula el webhook y guarda el último POST recibido.
class WebhookSimulado {
public:
    WebhookSimulado() {
        servidor_.Post("/hook", [this](const httplib::Request& req, httplib::Response& res) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                cuerpo_ = req.body;
                tipo_ = req.get_header_value("Content-Type");
                recibido_ = true;
            }
            cv_.notify_one();
            res.status = 200;
        });

        puerto_ = servidor_.bind_to_any_port("127.0.0.1");
        hilo_ = std::thread([this] { servidor_.listen_after_bind(); });
        while (!servidor_.is_running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    ~WebhookSimulado() {
        servidor_.stop();
        if (hilo_.joinable()) {
            hilo_.join();
        }
    }

    std::string url() const {
        return "http://127.0.0.1:" + std::to_string(puerto_) + "/hook";
    }

    /// Espera hasta `limite` a que llegue un POST.
    bool esperarPost(std::chrono::seconds limite) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, limite, [this] { return recibido_; });
    }

    std::string cuerpo() {
        std::lock_guard<std::mutex> lock(mutex_);
        return cuerpo_;
    }

    std::string tipo() {
        std::lock_guard<std::mutex> lock(mutex_);
        return tipo_;
    }

private:
    httplib::Server servidor_;
    std::thread hilo_;
    int puerto_{0};
    std::mutex mutex_;
    std::condition_variable cv_;
    std::string cuerpo_;
    std::string tipo_;
    bool recibido_{false};
};

EventoAlerta eventoDePrueba() {
    EventoAlerta evento;
    evento.timestamp = 1700000000;
    evento.metrica = "cpu";
    evento.umbral = 80.0;
    evento.valor = 95.5;
    evento.mensaje = "Alerta: cpu > 80.000000";
    return evento;
}

} // namespace

TEST(NotificadorWebhook, EnviaPostConElJsonDeLaAlerta) {
    WebhookSimulado webhook;
    auto notificador = crearNotificadorWebhook(webhook.url());

    notificador->notificar(eventoDePrueba());

    ASSERT_TRUE(webhook.esperarPost(std::chrono::seconds(5)))
        << "El webhook no recibió el POST";
    EXPECT_EQ(webhook.tipo(), "application/json");

    const auto json = nlohmann::json::parse(webhook.cuerpo());
    EXPECT_EQ(json["timestamp"].get<std::int64_t>(), 1700000000);
    EXPECT_EQ(json["metrica"].get<std::string>(), "cpu");
    EXPECT_DOUBLE_EQ(json["umbral"].get<double>(), 80.0);
    EXPECT_DOUBLE_EQ(json["valor"].get<double>(), 95.5);
    EXPECT_EQ(json["mensaje"].get<std::string>(), "Alerta: cpu > 80.000000");
}

TEST(NotificadorWebhook, NoBloqueaSiElWebhookNoExiste) {
    // Puerto 1: no hay ningún servidor escuchando.
    auto notificador = crearNotificadorWebhook("http://127.0.0.1:1/hook", 1);

    const auto inicio = std::chrono::steady_clock::now();
    EXPECT_NO_THROW(notificador->notificar(eventoDePrueba()));
    const auto duracion = std::chrono::steady_clock::now() - inicio;

    EXPECT_LT(duracion, std::chrono::milliseconds(100));
}

TEST(NotificadorWebhook, NoBloqueaSiElWebhookTardaEnResponder) {
    httplib::Server lento;
    lento.Post("/hook", [](const httplib::Request&, httplib::Response& res) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        res.status = 200;
    });
    const int puerto = lento.bind_to_any_port("127.0.0.1");
    std::thread hilo([&] { lento.listen_after_bind(); });
    while (!lento.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    auto notificador = crearNotificadorWebhook(
        "http://127.0.0.1:" + std::to_string(puerto) + "/hook", 1);

    const auto inicio = std::chrono::steady_clock::now();
    notificador->notificar(eventoDePrueba());
    const auto duracion = std::chrono::steady_clock::now() - inicio;

    EXPECT_LT(duracion, std::chrono::milliseconds(100));

    lento.stop();
    hilo.join();
}
