#ifndef GLOBAL_H
#define GLOBAL_H

#include <SPI.h>
#include <Wire.h>
#include "ADXL.h"
#include <Arduino.h>
#include <SX126XLT.h>  // SX12XX-LoRa Bibliothek
#include "SSD1306Wire.h"
#include "pins_arduino.h"

#define OLED_ADDRESS 0x3C

// Debug levels
#define DEBUG_LOOP  (1 << 0) // Bit 0
#define DEBUG_LORA  (1 << 1) // Bit 1
#define LORA_MSGS (1 << 2)  // Bit 2
#define DEBUG_ADXL (1 << 3) //Bit 3
#define DEBUG_INIT (1 << 4) //Bit 4

extern uint8_t debugLevel;
extern SSD1306Wire display;

void debugPrint(uint8_t level, const String& message);

#endif