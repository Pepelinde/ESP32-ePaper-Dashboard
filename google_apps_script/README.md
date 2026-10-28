# Conectar los datos reales

## 1. OpenWeather: listo para probar

El ESP32 utiliza la clave y las coordenadas existentes en `src/owm_credentials.local.h`.
Además del tiempo, consulta por HTTPS `/data/2.5/air_pollution` y muestra AQI,
PM2.5 y PM10. No tienes que añadir otra clave.

La escala de OpenWeather es: 1 buena, 2 aceptable, 3 moderada, 4 mala y 5 muy
mala. No es una escala de 0 a 500. Las partículas se expresan en µg/m³.

Puedes cargar el programa antes de configurar Google: mostrará los datos de
aire disponibles y «Configurar Google» en los demás bloques, sin datos ficticios.

## 2. Crear el servicio de Google

1. Abre https://script.google.com con la cuenta que contiene tu calendario,
   tareas y Gmail. Crea un **Nuevo proyecto**, por ejemplo «Dashboard ESP32».
2. Copia el contenido de `Code.gs` de esta carpeta en el archivo del editor de
   Google (normalmente `Código.gs`).
3. En **Configuración del proyecto**, activa **Mostrar el archivo de manifiesto
   appsscript.json en el editor**. Sustituye su contenido por el de
   `appsscript.json` de esta carpeta y guarda.
4. El manifiesto habilita Calendar API v3, Tasks API v1 y Gmail API v1. Comprueba
   que aparecen en **Servicios**. Si usas un proyecto de Google Cloud propio,
   habilita también esas tres APIs en dicho proyecto.
5. En **Configuración del proyecto → Propiedades de la secuencia de comandos**,
   añade:

   | Propiedad | Valor |
   | --- | --- |
   | `DASHBOARD_TOKEN` | Una cadena aleatoria privada de al menos 32 caracteres |
   | `DASHBOARD_NOTE` | Texto manual provisional de NOTA; no conectado a Google Keep |
   | `CALENDAR_ID` | Opcional: ID de otro calendario; vacío usa el principal |
   | `TASK_LIST_ID` | Opcional: ID de otra lista; vacío usa la lista predeterminada |

   Puedes generar el token en PowerShell con:

   ```powershell
   [guid]::NewGuid().ToString('N') + [guid]::NewGuid().ToString('N')
   ```

   Usa el mismo token después en el ESP32. No lo publiques ni lo pegues en chats.
6. Selecciona `testDashboard` y pulsa **Ejecutar**. Autoriza la lectura del
   calendario, tareas y metadatos de Gmail. En el registro deben aparecer los
   cuatro bloques con `true`. Si uno aparece con `false`, revisa su aviso en el
   registro y la activación del servicio correspondiente.
7. Pulsa **Implementar → Nueva implementación → Aplicación web**:
   - **Ejecutar como:** tú.
   - **Quién tiene acceso:** cualquier usuario (sin inicio de sesión).
   - Implementa y copia la URL que termina en `/exec`.

   El ESP32 no inicia sesión con Google. El script verifica el token antes de
   leer cualquier dato. Aunque la ruta es accesible sin inicio de sesión, la
   respuesta con tus datos requiere ese token. Si tu organización impide acceso
   anónimo, este método necesita otra cuenta o una alternativa de autenticación.

## 3. Configurar el ESP32

En `src/dashboard_config.local.h`, completa únicamente estas dos líneas:

```cpp
static const char GOOGLE_DASHBOARD_URL[] = "https://script.google.com/macros/s/TU_IMPLEMENTACION/exec";
static const char GOOGLE_DASHBOARD_TOKEN[] = "EL_MISMO_TOKEN_DE_LAS_PROPIEDADES";
```

Usa la URL sin `?token=...`, no una URL `/dev`. Guarda, compila y carga
`Waveshare_7_5_Mejora.ino` desde Arduino IDE, conservando los archivos `.h` junto
al sketch. Si cambias el código de Google, actualiza la implementación a una
**Nueva versión**. Cambiar la propiedad DASHBOARD_NOTE no requiere otra
implementacion ni volver a cargar el ESP32. La integracion con Google Keep
esta pendiente de confirmar el tipo de cuenta y acceso a su API empresarial.

## Qué muestra la pantalla

- Calendario calculado en el ESP32 con la fecha local. Google marca los días
  con eventos y proporciona los tres próximos eventos del mes, incluidos
  recurrentes y de todo el día. Los eventos de varios días marcan todos los
  días ocupados. Los títulos largos se recortan al ancho disponible.
- Las tres tareas pendientes modificadas más recientemente de la lista elegida.
  Si existen más de tres, aparece «Hay más tareas pendientes». No se muestran
  las completadas. El script devuelve cuatro títulos como máximo; el cuarto
  sirve para detectar que hay más tareas.
- La nota manual de DASHBOARD_NOTE (hasta 180 caracteres y cuatro lineas en
  el firmware actual). No procede de Google Tasks ni de Google Keep.
- El número de **mensajes** sin leer de la etiqueta UNREAD de Gmail, sin asuntos,
  remitentes ni contenido. No es el número de conversaciones sin leer.
- Datos de aire de OpenWeather y el tiempo/previsión que ya funcionaban.

El refresco sigue siendo cada 30 minutos, entre las 07:00 y las 23:00. La hora
de la cabecera y el reloj corresponden al refresco de la pantalla.

## Fallos y datos guardados

Cada bloque se actualiza de forma independiente. El ESP32 guarda sus últimos
datos válidos en memoria persistente; si falla una consulta muestra
**GUARDADO** en ese bloque. Descarta cachés de más de 24 horas. Para el calendario
también exige el mismo mes/año y oculta eventos ya finalizados. Una lista vacía
correcta muestra «Sin tareas pendientes» o «Sin próximos eventos este mes»;
un fallo sin caché muestra «No disponible», y nunca inventa un cero en Gmail.

La caché no guarda tu token. Desactivar Google vaciando la configuración impide
mostrar sus datos guardados. Cambiar la URL impide reutilizar la caché anterior.

Se mantiene el comportamiento existente del tiempo: si falla el Wi-Fi, la hora
NTP o la consulta meteorológica principal, la pantalla conserva la imagen previa.
La cabecera es el estado Wi-Fi observado durante ese refresco, no una supervisión
en tiempo real mientras el ESP32 duerme.

El monitor serie a **115200 baudios** informa del código HTTP y de la disponibilidad
de los bloques sin imprimir el token ni los datos de Google. Un HTTP 200 con JSON
no disponible suele indicar una página de inicio de sesión o una implementación
incorrecta; una respuesta rechazada puede indicar un token que no coincide.

HTTPS valida certificados mediante `dashboard_ca.h`, con certificados públicos
de Google, ISRG y Sectigo. Se ha comprobado que la cadena actual de OpenWeather
termina en Sectigo Root R46. No se desactiva la verificación TLS. Si los proveedores
cambian su cadena de confianza, habrá que actualizar esos certificados.

## Referencias

- https://docs.openweather.co.uk/api/air-pollution
- https://developers.google.com/apps-script/guides/web
- https://developers.google.com/apps-script/guides/content
- https://developers.google.com/apps-script/advanced/calendar
- https://developers.google.com/apps-script/advanced/tasks
- https://developers.google.com/apps-script/advanced/gmail
- https://developers.google.com/workspace/gmail/api/auth/scopes
