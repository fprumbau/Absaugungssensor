#ifndef LORA_H
#define LORA_H

#include "global.h"

#define SS_LoRa 8
#define SCK_LoRa 9
#define MOSI_LoRa 10
#define MISO_LoRa 11
#define RST_LoRa 12
#define BUSY_LoRa 13
#define DIO1_LoRa 14
#define SW_LoRa -1

#define SENSOR_ID 1  // Eindeutige ID für diesen Sensor (1-255)

class LoRa {
public:
  LoRa();
  bool init();
  bool send(const String& message, uint16_t timeout = 1000, int8_t txPower = 10); // Bestehende Methode
  bool send(uint8_t action); // Neue Methode für Protokoll
  bool receive(uint8_t& sensorId, uint8_t& action); // Neue Empfangsmethode
  const char* actionToString(uint8_t action); // Für Debug

private:
  SX126XLT LT;
  static constexpr int SS_PIN = 8;
  static constexpr int RST_PIN = 12;
  static constexpr int BUSY_PIN = 13;
  static constexpr int DIO1_PIN = 14;
  static constexpr int SW_PIN = -1;
  static constexpr uint32_t FREQUENCY = 868000000;
  static constexpr int8_t SPREADING_FACTOR = LORA_SF7;
  static constexpr int8_t BANDWIDTH = LORA_BW_125;
  static constexpr int8_t CODE_RATE = LORA_CR_4_5;
};

extern LoRa lora;

#endif