// Independent test for the existing ESP32 and 800x480 monochrome panel.
// Uses the same GxEPD2_750_T7 driver as the dashboard.
#include <SPI.h>
#include <GxEPD2_BW.h>

constexpr int EPD_CS = 21;
constexpr int EPD_DC = 17;
constexpr int EPD_RST = 16;
constexpr int EPD_BUSY = 4;
constexpr int EPD_PWR = 33;
constexpr int EPD_CLK = 18;
constexpr int EPD_DIN = 23;
// Solo activar si GPIO33 esta conectado a una ENTRADA PWR de la controladora.
// Nunca alimentar VCC de la pantalla directamente desde un GPIO.
constexpr bool CONTROL_PWR = false;

#ifndef SOLO_ESP32
#define SOLO_ESP32 0
#endif

GxEPD2_BW<GxEPD2_750_T7, 120> display(
  GxEPD2_750_T7(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.printf("\nESP32 iniciado. Heap libre: %u bytes\n", ESP.getFreeHeap());
#if SOLO_ESP32
  Serial.println("Modo SOLO_ESP32: pantalla sin inicializar. Desconectala para esta prueba.");
  return;
#endif
  Serial.println("\nPRUEBA PANTALLA: un unico refresco, sin WiFi.");
  Serial.println("DIN=23 CLK=18 CS=21 DC=17 RST=16 BUSY=4");
  if (CONTROL_PWR) {
    pinMode(EPD_PWR, OUTPUT);
    digitalWrite(EPD_PWR, HIGH);
  }
  delay(500);
  SPI.begin(EPD_CLK, -1, EPD_DIN, EPD_CS);
  display.init(115200, true, 2, false);
  display.setRotation(0);
  display.setFullWindow();

  Serial.println("Enviando patron...");
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawRect(10, 10, 780, 460, GxEPD_BLACK);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(4);
    display.setCursor(40, 40);
    display.print("PRUEBA E-PAPER");
    display.setTextSize(2);
    display.setCursor(40, 100);
    display.print("Texto y figuras deben verse nitidos");
    display.fillRect(40, 160, 200, 180, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(70, 235);
    display.print("NEGRO");
    display.setTextColor(GxEPD_BLACK);
    display.drawRect(290, 160, 200, 180, GxEPD_BLACK);
    display.setCursor(350, 235);
    display.print("BLANCO");
    display.drawCircle(640, 250, 85, GxEPD_BLACK);
    display.drawLine(550, 160, 730, 340, GxEPD_BLACK);
    display.drawLine(550, 340, 730, 160, GxEPD_BLACK);
    display.setCursor(40, 405);
    display.print("800 x 480 - ESP32 - SPI");
  } while (display.nextPage());

  display.hibernate();
  if (CONTROL_PWR) digitalWrite(EPD_PWR, LOW);
  Serial.println("Prueba terminada. Comprueba visualmente la imagen.");
  Serial.println("Si aparece Busy Timeout, el refresco no esta confirmado.");
}

void loop() {
  delay(5000); // No automatic retries or repeated refreshes.
  Serial.printf("ESP32 activo; uptime=%lu s; heap=%u; BUSY=%d\n",
    millis() / 1000UL, ESP.getFreeHeap(),
    SOLO_ESP32 ? -1 : digitalRead(EPD_BUSY));
}
