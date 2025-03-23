#include "global.h"

// LoRa-Instanz
SX126XLT LT;

void setup() {
  debugLevel = DEBUG_DISPLAY | LORA_MSGS; // Debug-Level setzen

  Serial.begin(115200);
  while (!Serial) delay(10);

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

  // SPI initialisieren
  SPI.begin(SCK_LoRa, MISO_LoRa, MOSI_LoRa, SS_LoRa);
  debugPrint(DEBUG_INIT, "SPI initialisiert");

  // LoRa initialisieren
  if (!LT.begin(SS_LoRa, RST_LoRa, BUSY_LoRa, DIO1_LoRa, SW_LoRa, DEVICE_SX1262)) {
    Serial.println("LoRa-Initialisierung fehlgeschlagen!");
  } else {
    debugPrint(DEBUG_INIT, "LoRa initialisiert erfolgreich!");
    // LoRa-Parameter setzen (müssen mit Sender übereinstimmen)
    LT.setupLoRa(868000000, 0, LORA_SF7, LORA_BW_125, LORA_CR_4_5, LDRO_AUTO);  // 868 MHz, SF7, BW 125 kHz
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
        if (LT.transmit(buffer, len, 1000, TXpower, WAIT_TX)) {
          debugPrint(LORA_MSGS, "Nachricht gesendet: " + message);
        } else {
          Serial.println("Fehler beim Senden!");
        }
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