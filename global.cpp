#include "global.h"

//Debug-Level (Bitmasken)
//DEBUG_LOOP: Loop Start/Ende Meldungen
//DEBUG_LORA: LoRa-Statusmeldungen (Prüfe auf Pakete)
//LORA_MSGS: LoRa-Nachrichten
//DEBUG_ADXL: ADXL-Ausgaben
uint8_t debugLevel = LORA_MSGS || DEBUG_ADXL;  

// Status-Variablen
const int8_t TXpower = 10;  // 10 dBm – Sendeleistung
int messageCounter = 1;  // Laufender Zähler für Nachrichten

// Taster-Entprellung
unsigned long lastDebounceTime = 0;  // Zeit des letzten Tasterwechsels
int lastButtonState = HIGH;  // Letzter Tasterstatus
int buttonState = HIGH;      // Aktueller Tasterstatus
String lastSentMessage = "";  // Zuletzt gesendete Nachricht

void debugPrint(uint8_t level, const String& message) {
  if (debugLevel & level) {
    Serial.println(message);
  }
}