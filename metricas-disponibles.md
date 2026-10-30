# Métricas disponibles en ChuspaPulse

Esta página lista todas las métricas que recopila y expone el sistema, con su representación JSON, tipo, unidad, rango, fuente y disponibilidad.

| Nombre de la métrica           | Campo JSON               | Tipo      | Unidad        | Rango típico      | Fuente en Linux       | Disponibilidad       |
|--------------------------------|--------------------------|-----------|---------------|-------------------|-----------------------|----------------------|
| Uso total de CPU               | `cpu.usage_pct`          | float     | %             | 0.0 – 100.0       | `/proc/stat`          | Siempre              |
| Tiempo de usuario CPU          | `cpu.user_s`             | float     | segundos      | ≥ 0               | `/proc/stat`          | Siempre              |
| Tiempo de sistema CPU          | `cpu.system_s`          | float     | segundos      | ≥ 0               | `/proc/stat`          | Siempre              |
| Núcleos físicos detectados     | `cpu.cores`              | uint64    | —             | ≥ 1               | `/proc/cpuinfo`       | Siempre              |
| Memoria RAM total              | `ram.total_bytes`        | uint64    | bytes         | ≥ 1 MiB           | `/proc/meminfo`       | Siempre              |
| Memoria RAM libre              | `ram.free_bytes`         | uint64    | bytes         | ≥ 0               | `/proc/meminfo`       | Siempre              |
| Memoria RAM en uso             | `ram.used_pct`           | float     | %             | 0.0 – 100.0       | `/proc/meminfo`       | Siempre              |
| Memoria RAM en búferes         | `ram.buffers_bytes`      | uint64    | bytes         | ≥ 0               | `/proc/meminfo`       | Siempre              |
| Espacio total en disco raíz    | `disk.total_bytes`       | uint64    | bytes         | ≥ 1 MiB           | `/proc/statfs`        | Según montaje        |
| Espacio libre en disco raíz    | `disk.free_bytes`        | uint64    | bytes         | ≥ 0               | `/proc/statfs`        | Según montaje        |
| Uso de disco raíz              | `disk.used_pct`          | float     | %             | 0.0 – 100.0       | `/proc/statfs`        | Según montaje        |
| Bytes recibidos por red        | `net.rx_bytes`           | uint64    | bytes         | ≥ 0               | `/proc/net/dev`       | Según interfaz       |
| Bytes enviados por red         | `net.tx_bytes`           | uint64    | bytes         | ≥ 0               | `/proc/net/dev`       | Según interfaz       |
| Carga del sistema (1 min)      | `load.avg_1min`          | float     | —             | ≥ 0               | `/proc/loadavg`       | Siempre              |
| Carga del sistema (5 min)      | `load.avg_5min`          | float     | —             | ≥ 0               | `/proc/loadavg`       | Siempre              |
| Tiempo de actividad del sistema| `system.uptime_s`        | float     | segundos      | ≥ 0               | `/proc/uptime`        | Siempre              |
