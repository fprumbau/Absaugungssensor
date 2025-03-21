#include "heltec.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("Start vor Heltec.begin...");

  // Display aktivieren, LoRa deaktivieren, Serial aktivieren
  Heltec.begin(true /*DisplayEnable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Serial.println("Heltec.begin abgeschlossen");

  // Display sollte bereits initialisiert sein, aber wir testen init() separat
  Heltec.display->init();
  Serial.println("Display init abgeschlossen");

  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, "Hello World!");
  Heltec.display->display();
  Serial.println("Display-Zugriff abgeschlossen");
}

void loop() {
  // Leer lassen
}