#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace ChuspaPulse::alertas {

/**
 * @brief Datos de una alerta disparada que se envían a un notificador externo.
 */
struct EventoAlerta {
    std::int64_t timestamp{0}; ///< Momento del disparo (epoch, segundos)
    std::string metrica;       ///< Nombre de la métrica evaluada
    double umbral{0.0};        ///< Umbral configurado
    double valor{0.0};         ///< Valor de la métrica al dispararse
    std::string mensaje;       ///< Mensaje descriptivo de la alerta
};

/**
 * @brief Canal externo de notificación de alertas (webhook, correo, etc.).
 *
 * Las implementaciones no deben bloquear al llamador ni lanzar excepciones:
 * se invocan desde el ciclo del sampler.
 */
class INotificador {
public:
    virtual ~INotificador() = default;

    /**
     * @brief Envía la alerta al canal externo.
     * @param evento Datos de la alerta disparada.
     */
    virtual void notificar(const EventoAlerta& evento) = 0;
};

/**
 * @brief Crea un notificador que hace POST con el JSON de la alerta a una URL.
 *
 * El envío se hace en un hilo aparte; si el webhook falla o no responde,
 * la notificación se descarta sin afectar al agente.
 *
 * @param url URL del webhook (http://host[:puerto][/ruta]).
 * @param timeoutSegundos Tiempo máximo de conexión, escritura y lectura.
 * @return Notificador listo para usar.
 */
std::shared_ptr<INotificador> crearNotificadorWebhook(const std::string& url,
                                                      int timeoutSegundos = 3);

} // namespace ChuspaPulse::alertas
