#ifndef GLOBAL_H
#define GLOBAL_H

#include <SPI.h>
#include <Wire.h>
#include "ADXL.h"
#include "display.h"
#include <Arduino.h>
#include <SX126XLT.h>  // SX12XX-LoRa Bibliothek
#include "SSD1306Wire.h"
#include "pins_arduino.h"

// Pin-Definitionen für Heltec WiFi LoRa 32 V3 (aus Forum-Posting)
#define SDA_OLED 17
#define SCL_OLED 18
#define RST_OLED 21
#define SS_LoRa 8
#define SCK_LoRa 9
#define MOSI_LoRa 10
#define MISO_LoRa 11
#define RST_LoRa 12
#define BUSY_LoRa 13
#define DIO1_LoRa 14
#define SW_LoRa -1

#define TASTER 19  // Taster an GPIO 19
#define DEBOUNCE_DELAY 100  // Entprellungszeit in ms

// Debug levels
#define DEBUG_LOOP    (1 << 0) // Bit 0
#define DEBUG_LORA    (1 << 1) // Bit 1
#define LORA_MSGS     (1 << 2) // Bit 2
#define DEBUG_ADXL    (1 << 3) // Bit 3
#define DEBUG_INIT    (1 << 4) // ...
#define DEBUG_DISPLAY (1 << 5)

extern uint8_t debugLevel;

void debugPrint(uint8_t level, const String& message);

#endif