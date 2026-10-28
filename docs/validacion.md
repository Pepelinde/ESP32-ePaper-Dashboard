# Validacion de la copia para GitHub

Fecha: 28 de octubre de 2026.

| Comprobacion | Resultado |
| --- | --- |
| PlatformIO: dashboard | Compila correctamente |
| PlatformIO: pantalla | Compila correctamente |
| PlatformIO: solo_esp32 | Compila correctamente |
| PlatformIO: legacy_esp32_hat | Compila correctamente |
| Pruebas locales de Google Apps Script | Correctas, con servicios simulados |
| Revision de valores privados originales y patrones de secretos | Sin coincidencias en los archivos de texto exportados |
| Exclusiones Git para configuracion y foto locales | Comprobadas |

Versiones utilizadas: espressif32 7.1.3, GxEPD2 1.6.9, ArduinoJson 7.4.3,
U8g2_for_Adafruit_GFX 1.8.0, Adafruit GFX 1.13.0 y Adafruit BusIO 1.17.4.

La compilacion no verifica el funcionamiento fisico del montaje. No se ha
cargado esta copia en la placa ni se han consultado servicios reales con las
credenciales de ejemplo. El circuito de bateria sigue pendiente de validacion.

La exportacion convierte el sketch a C++ con declaraciones explicitas y
adapta las comparaciones de respuesta Google a ArduinoJson 7.4. Las
configuraciones personales y la foto se seleccionan mediante archivos
*.local.h ignorados por Git, con plantillas o imagen neutra como alternativa.
