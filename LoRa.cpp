#include "LoRa.h"

LoRa::LoRa() {}

bool LoRa::init() {
  SPI.begin(SCK, MISO, MOSI, SS_PIN);
  if (!LT.begin(SS_PIN, RST_PIN, BUSY_PIN, DIO1_PIN, SW_PIN, DEVICE_SX1262)) {
    Serial.println("LoRa-Initialisierung fehlgeschlagen!");
    return false;
  }
  LT.setupLoRa(FREQUENCY, 0, SPREADING_FACTOR, BANDWIDTH, CODE_RATE, LDRO_AUTO);
  debugPrint(DEBUG_LORA, "LoRa initialisiert erfolgreich!");
  return true;
}

bool LoRa::send(const String& message, uint16_t timeout, int8_t txPower) {
  uint8_t buffer[256]; // Annahme: max. 256 Bytes
  int len = message.length();
  if (len > 255) len = 255; // Begrenzung auf 255 Bytes
  message.getBytes(buffer, len + 1); // String in Buffer kopieren
  if (LT.transmit(buffer, len, timeout, txPower, WAIT_TX)) {
    debugPrint(DEBUG_LORA, "Nachricht gesendet: " + message);
    return true;
  } else {
    Serial.println("Fehler beim Senden!");
    return false;
  }
}

LoRa lora; // Definition des globalen Objekts