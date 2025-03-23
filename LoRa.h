#ifndef LORA_H
#define LORA_H

#include "global.h"

// Pin-Definitionen für Heltec WiFi LoRa 32 V3 (aus Forum-Posting)
#define SS_LoRa 8
#define SCK_LoRa 9
#define MOSI_LoRa 10
#define MISO_LoRa 11
#define RST_LoRa 12
#define BUSY_LoRa 13
#define DIO1_LoRa 14
#define SW_LoRa -1

class LoRa {
public:
  LoRa();
  bool init();              // Initialisiert LoRa
  bool send(const String& message, uint16_t timeout = 1000, int8_t txPower = 10); // Sendet eine Nachricht

private:
  SX126XLT LT;              // Internes SX126XLT-Objekt
  static constexpr int SS_PIN = 8;    // CS-Pin für LoRa
  static constexpr int RST_PIN = 12;  // Reset-Pin für LoRa
  static constexpr int BUSY_PIN = 13; // Busy-Pin für LoRa
  static constexpr int DIO1_PIN = 14; // DIO1-Pin für LoRa
  static constexpr int SW_PIN = -1;   // SW-Pin (nicht verwendet)
  static constexpr uint32_t FREQUENCY = 868000000; // 868 MHz
  static constexpr int8_t SPREADING_FACTOR = LORA_SF7;
  static constexpr int8_t BANDWIDTH = LORA_BW_125;
  static constexpr int8_t CODE_RATE = LORA_CR_4_5;
};

extern LoRa lora; // Globales LoRa-Objekt

#endif