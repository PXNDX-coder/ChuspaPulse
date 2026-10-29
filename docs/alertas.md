# Sistema de Alertas

El módulo `GestorAlertas` evalúa las métricas del sistema en cada ciclo de muestreo. Si una métrica supera su umbral, se registra una advertencia (`WARN`) en los logs del agente.

## Componentes

* **AlertaUmbral**: regla que compara una métrica contra un valor límite con el operador `>` o `<`.
* **GestorAlertas**: guarda varias `AlertaUmbral`, las evalúa todas contra un snapshot y ejecuta un callback por cada alerta que se dispara.
* **INotificador**: interfaz para enviar las alertas disparadas a un canal externo. La implementación incluida es un webhook HTTP genérico.

## Cómo funciona

1. El `Sampler` recolecta las métricas de todos los collectors.
2. Guarda el snapshot en la base de datos.
3. Convierte el snapshot a porcentajes de CPU, RAM y disco.
4. `GestorAlertas` evalúa cada alerta contra esos valores.
5. Por cada alerta disparada, escribe un mensaje de nivel `WARN` en el log.
6. Si hay un webhook configurado, envía la alerta a esa URL sin esperar la respuesta.

El ciclo se repite cada `intervalo_segundos` (sección `[sampler]`).

## Métricas evaluadas

| Métrica | Origen del valor | Operador |
| ------- | ---------------- | -------- |
| CPU     | `cpu.usage` (porcentaje) | `>` |
| RAM     | `memoria usada / memoria total * 100` | `>` |
| Disco   | `disk.used / disk.total * 100` | `>` |

## Configuración de umbrales (`pulso.toml`)

Los umbrales se leen desde la sección `[umbrales]`. Todos son porcentajes (0 a 100):

```toml
[umbrales]
cpu_max = 80.0
ram_max = 85.0
disco_max = 90.0
```

Si el archivo o alguna clave no existe, se usan esos mismos valores por defecto.

## Ejemplo de salida en el log

Cuando el uso de CPU supera `cpu_max`, aparece una línea de nivel `WARN` con el mensaje de la alerta, por ejemplo:

```
Alerta: cpu > 80.000000
```

## Notificaciones por webhook

Además del log, cada alerta disparada puede enviarse a un servicio externo mediante un `POST` HTTP.

### Configuración (`pulso.toml`)

```toml
[webhook]
url = "http://mi-servidor:9000/alertas"
timeout_segundos = 3
```

* `url`: dirección del webhook. Si está vacía o no existe, no se envía ninguna notificación. Solo se admite `http://`.
* `timeout_segundos`: tiempo máximo de conexión, envío y respuesta. Debe ser mayor que 0. El valor por defecto es `3`.

### Formato del mensaje

Se envía un `POST` con `Content-Type: application/json`:

```json
{
  "timestamp": 1700000000,
  "metrica": "cpu",
  "umbral": 80.0,
  "valor": 95.5,
  "mensaje": "Alerta: cpu > 80.000000"
}
```

| Campo | Descripción |
| ----- | ----------- |
| `timestamp` | Momento del disparo, en segundos desde epoch |
| `metrica` | Métrica evaluada (`cpu`, `ram` o `disk`) |
| `umbral` | Umbral configurado |
| `valor` | Valor de la métrica al dispararse la alerta |
| `mensaje` | Mismo mensaje que se escribe en el log |

### Comportamiento ante fallos

* El envío se hace en un hilo aparte, así que el ciclo del sampler no espera la respuesta del webhook.
* Si el webhook no existe, no responde dentro de `timeout_segundos` o devuelve un error, la notificación se descarta sin reintentos y el agente sigue funcionando.

## Comportamiento y limitaciones

* Las alertas solo se evalúan en modo daemon, no con `--once`.
* Mientras la métrica siga por encima del umbral, la advertencia se repite en cada ciclo. No existe un estado de "alerta resuelta". Lo mismo ocurre con el webhook: recibe un `POST` por cada ciclo en que la alerta siga activa.
* Los mensajes son de nivel `WARN`: no se ven si `nivel_log` es `"error"`.
* Las alertas no se consultan por la API. El endpoint `/alerts` está fuera del alcance de esta integración.
