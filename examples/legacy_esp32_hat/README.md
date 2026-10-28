# Diagnostico ESP32 y pantalla

Esta carpeta conserva la prueba del montaje anterior. En este repositorio se
compila desde la raiz con `pio run -e legacy_esp32_hat -t upload`.
Las instrucciones siguientes describen el proyecto independiente original;
no usar estos GPIO en la Waveshare con ESP32 integrado.

Proyecto PlatformIO independiente del dashboard. No utiliza Wi-Fi, bateria,
elevador ni credenciales. Usa GxEPD2_750_T7, igual que el dashboard: NO es
una reproduccion del driver Waveshare del ejemplo pegado.

## Primera prueba: ESP32 solo

1. Desconecta toda alimentacion antes de retirar los cables de la pantalla.
2. Conecta unicamente el ESP32 por USB.
3. Abre esta carpeta (Diagnostico_ESP32_Pantalla) como proyecto PlatformIO.
4. Compila y carga el entorno `solo_esp32`.
5. Abre el monitor serie a 115200. Debe indicar que esta activo cada 5 segundos,
   con un tiempo creciente. Un reinicio continuo o calentamiento anormal requiere
   revisar la placa. Esta prueba no certifica todos sus GPIO.

## Segunda prueba: pantalla

Con todo apagado, comprueba que los cables coinciden con:

| Senal pantalla | GPIO ESP32 |
| --- | --- |
| DIN / MOSI | 23 |
| CLK | 18 |
| CS | 21 |
| DC | 17 |
| RST | 16 |
| BUSY | 4 |
| GND | GND con continuidad confirmada |

La alimentacion VCC debe corresponder a la controladora concreta. No se
prescribe su tension aqui sin confirmar el modelo. No alimentes VCC desde
GPIO33. CONTROL_PWR permanece desactivado: activalo solo si has confirmado
que GPIO33 va a una entrada de habilitacion PWR independiente de VCC.

1. Usa solo USB como fuente del ESP32; deja bateria y elevador desconectados.
2. Compila y carga el entorno `pantalla`.
3. Abre el monitor serie a 115200 y pulsa EN para capturar el arranque completo.
4. Debe aparecer un marco, texto, un bloque negro, otro blanco y un circulo.
   Solo se realiza un refresco completo por arranque; no hay refrescos parciales.
5. Guarda el registro completo y una foto. `Busy Timeout` indica que la espera
   del controlador ha agotado su tiempo: el mensaje final no confirma el exito.

Si el patron sale bien, el conjunto funciona con este controlador en esta prueba.
Si sale mal, aun quedan por distinguir controlador, cableado, alimentacion y GPIO.
Si se calienta anormalmente, desconecta. No puentear el GND sospechoso.

## Comandos opcionales desde esta carpeta

    pio run -e solo_esp32 -t upload
    pio device monitor -b 115200
    pio run -e pantalla -t upload

PlatformIO descargara plataforma y bibliotecas cuando sea necesario. No se ha
fijado puerto: selecciona el del ESP32 si hay varios dispositivos conectados.
