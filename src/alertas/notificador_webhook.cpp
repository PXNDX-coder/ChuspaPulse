#include "inotificador.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <thread>
#include <utility>

namespace ChuspaPulse::alertas {
namespace {

/**
 * @brief Notificador que envía cada alerta como JSON a un webhook HTTP genérico.
 */
class NotificadorWebhook : public INotificador {
public:
    NotificadorWebhook(const std::string& url, int timeoutSegundos)
        : timeout_(timeoutSegundos) {
        // Separa "http://host:puerto" de la ruta "/algo".
        const auto finEsquema = url.find("://");
        const auto inicioRuta =
            url.find('/', finEsquema == std::string::npos ? 0 : finEsquema + 3);

        if (inicioRuta == std::string::npos) {
            base_ = url;
            ruta_ = "/";
        } else {
            base_ = url.substr(0, inicioRuta);
            ruta_ = url.substr(inicioRuta);
        }
    }

    void notificar(const EventoAlerta& evento) override {
        std::string cuerpo = nlohmann::json{
            {"timestamp", evento.timestamp},
            {"metrica", evento.metrica},
            {"umbral", evento.umbral},
            {"valor", evento.valor},
            {"mensaje", evento.mensaje},
        }.dump();

        try {
            // Hilo aparte: el sampler no espera la respuesta del webhook.
            std::thread([base = base_, ruta = ruta_, timeout = timeout_,
                         cuerpo = std::move(cuerpo)]() {
                try {
                    httplib::Client cliente(base);
                    cliente.set_connection_timeout(timeout, 0);
                    cliente.set_read_timeout(timeout, 0);
                    cliente.set_write_timeout(timeout, 0);
                    cliente.Post(ruta, cuerpo, "application/json");
                } catch (...) {
                    // Un fallo del webhook nunca debe afectar al agente.
                }
            }).detach();
        } catch (...) {
            // No se pudo crear el hilo: se omite la notificación.
        }
    }

private:
    std::string base_;
    std::string ruta_;
    int timeout_;
};

} // namespace

std::shared_ptr<INotificador> crearNotificadorWebhook(const std::string& url,
                                                      int timeoutSegundos) {
    return std::make_shared<NotificadorWebhook>(url, timeoutSegundos);
}

} // namespace ChuspaPulse::alertas
