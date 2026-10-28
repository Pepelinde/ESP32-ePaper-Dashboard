# Waveshare e-Paper ESP32 Driver Board Rev. 3

El dashboard y Prueba_Pantalla usan el conector FPC interno de esta placa.
No requieren cables GPIO externos ni el antiguo e-Paper Driver HAT.
Conectar el panel solo con toda alimentacion desconectada.

Asignacion interna: DIN=14, CLK=13, CS=15, DC=27, RST=26, BUSY=25.
No se utiliza MISO ni el antiguo control PWR por GPIO33.
GPIO15 se reserva para CS: se ha retirado el despertar tactil del dashboard.
Se mantiene el despertar automatico por temporizador.

Arduino IDE: ESP32 Dev Module (ESP32 clasico, no C3 ni S3), flash 4 MB,
puerto de la nueva placa, monitor serie 115200. Usar inicialmente USB.
Si aparece Wrong boot mode, mantener BOOT al reiniciar con EN para entrar
en modo de descarga. No conectar ni retirar el FPC con la placa encendida.

El panel sigue configurado como GxEPD2_750_T7, 800x480 monocromo.
Para una primera comprobacion, cargar examples/pantalla/main.cpp:
realiza un solo refresco. Luego cargar src/main.cpp.
El contenido del dashboard y sus credenciales no requieren cambios.

Para esta placa y el panel 7.5 monocromo, la tabla oficial indica selector
Display Config en A (0.47R). No copiar A/B del antiguo Driver HAT: las
etiquetas de resistencia pueden cambiar entre placas.
El segundo selector debe estar en ON para alimentar el USB-UART y poder
cargar el programa y leer el monitor serie. No es un selector SPI 3/4 hilos.

Documentacion oficial:
https://docs.waveshare.net/E-Paper_ESP32_Driver_Board/
https://docs.waveshare.net/E-Paper_ESP32_Driver_Board/Instructions-For-Use/

La carpeta examples/legacy_esp32_hat conserva pruebas de la placa anterior;
sus asignaciones GPIO no corresponden a esta Waveshare.

