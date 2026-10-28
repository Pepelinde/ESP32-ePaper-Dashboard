#include <Arduino.h>
void setup() { Serial.begin(115200); delay(1500); Serial.println("ESP32: prueba sin pantalla ni WiFi"); }
void loop() { Serial.printf("ESP32 activo; uptime=%lu s; heap=%u\n", millis()/1000UL, ESP.getFreeHeap()); delay(5000); }
