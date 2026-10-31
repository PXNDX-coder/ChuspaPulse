#include "schema.hpp"

namespace chuspapulse::storage {
/**
 * @brief Inicializa el esquema de la base de datos SQLite.
 *
 * Crea la tabla `snapshots` con todas las columnas necesarias para
 * almacenar métricas del sistema (CPU, memoria, disco y red), y el
 * índice `idx_snapshots_timestamp` para optimizar las consultas por
 * rango de tiempo. La función es idempotente: puede invocarse múltiples
 * veces sobre la misma base de datos sin producir errores.
 *
 * @param db Referencia a la base de datos SQLite ya abierta.
 * @throws SQLite::Exception si la ejecución del SQL falla por un error
 *         interno de la base de datos.
 * También crea la tabla alerts para el historial de alertas disparadas
 */
// Refactorizacion de la conexion por parametro
void inicializarEsquema(SQLite::Database& db) {

    db.exec(
        "CREATE TABLE IF NOT EXISTS snapshots ("
        "    timestamp INTEGER NOT NULL,"
        "    nombre    TEXT    NOT NULL,"
        "    valor     REAL    NOT NULL,"
        "    unidad    TEXT    NOT NULL"
        ");"
    );

    db.exec(
        "CREATE INDEX IF NOT EXISTS idx_snapshots_timestamp "
        "ON snapshots(timestamp);"
    );

    db.exec(
        "CREATE TABLE IF NOT EXISTS alerts ("
        "    id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    timestamp INTEGER NOT NULL,"
        "    metrica   TEXT    NOT NULL,"
        "    umbral    REAL    NOT NULL,"
        "    valor     REAL    NOT NULL,"
        "    mensaje   TEXT    NOT NULL"
        ");"
    );

    db.exec(
        "CREATE INDEX IF NOT EXISTS idx_alerts_timestamp "
        "ON alerts(timestamp);"
    );
}

} // namespace chuspapulse::storage
