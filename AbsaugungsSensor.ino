#include "global.h"



void setup() {

    debugLevel = DEBUG_DISPLAY | LORA_MSGS | DEBUG_CONFIG; // Debug-Level setzen

    Serial.begin(115200);
    while (!Serial) delay(10);

    //Konfig lesen
    config.load(); // Lade Konfiguration (SSID, Passwort)
    debugPrint(DEBUG_DISPLAY, "SSID: " + String(config.getSSID()));
    debugPrint(DEBUG_DISPLAY, "Pass: " + String(config.getPass()));

    //einmal schreiben
    config.save();

    if (!oled.init()) {
      Serial.println("Display initialization failed!");
      while (1);
    }
    oled.clear();
    oled.drawString(0, 0, "AbsaugungsSensor startet...");
    oled.display();

    debugPrint(DEBUG_INIT, "AbsaugungsSensor startet...");

    // Taster initialisieren
    pinMode(TASTER, INPUT_PULLUP);  // Pullup, LOW = gedrückt

    // LoRa initialisieren
    if (!lora.init()) {
      Serial.println("LoRa initialization failed!");
      while (1);
    }

    //ADXL initialisieren
    if (!adxl.init()) {
      debugPrint(DEBUG_ADXL, "ADXL initialization failed!");
      while (1);
    }
}

void loop() {

    // Taster prüfen mit Entprellung
    int reading = digitalRead(TASTER);
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {  // Taster von OFF (HIGH) zu ON (LOW)
          String message = "BlaBla " + String(messageCounter++);
          debugPrint(LORA_MSGS, "Sende LoRa-Nachricht: " + message);
          uint8_t buffer[255];
          uint8_t len = message.length();
          for (uint8_t i = 0; i < len; i++) {
            buffer[i] = message[i];
          }
          lora.send(message, 1000, 10); // Timeout 1000ms, TX-Power 10dBm
        }
      }
    }
    lastButtonState = reading;

    adxl.update();
    if (debugLevel & 8) adxl.print();

    if (adxl.detectMovement(0.2)) {
      debugPrint(DEBUG_ADXL, "Bewegung erkannt!");
      // Optional: Bei Bewegung LoRa-Nachricht schicken
      //LoRa.beginPacket();
      // ... LoRa.send ...
      //LoRa.endPacket()
    }

    // Display aktualisieren
    oled.clear();
    oled.drawString(0, 0, "AbsaugungsSensor");
    oled.drawString(0, 16, "Taster: " + String(buttonState == LOW ? "gedrückt" : "los"));
    oled.drawString(0, 32, "X: " + String(adxl.getGX(), 2) + " g");
    oled.drawString(0, 40, "Y: " + String(adxl.getGY(), 2) + " g");
    oled.drawString(0, 48, "Z: " + String(adxl.getGZ(), 2) + " g");
    oled.display();;

    delay(100);  // Kürzerer Delay für flüssigere Updates

}