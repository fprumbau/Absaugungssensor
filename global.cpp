#include "global.h"

//Debug-Level (Bitmasken)
//DEBUG_LOOP: Loop Start/Ende Meldungen
//DEBUG_LORA: LoRa-Statusmeldungen (Prüfe auf Pakete)
//LORA_MSGS: LoRa-Nachrichten
//DEBUG_ADXL: ADXL-Ausgaben
uint8_t debugLevel = LORA_MSGS || DEBUG_ADXL;  

void debugPrint(uint8_t level, const String& message) {
  if (debugLevel & level) {
    Serial.println(message);
  }
}