#pragma once

#include <SQLiteCpp/Database.h>

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
 */
void inicializarEsquema(SQLite::Database& db);

} // namespace chuspapulse::storage
