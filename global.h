#ifndef GLOBAL_H
#define GLOBAL_H

#include <SPI.h>
#include <Wire.h>
#include "ADXL.h"
#include "LoRa.h"
#include "display.h"
#include <Arduino.h>
#include <SX126XLT.h>  // SX12XX-LoRa Bibliothek
#include "SSD1306Wire.h"
#include "pins_arduino.h"

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

// Status-Variablen
extern String statusLine;
extern String secondsLine;
extern String tasterLine;
extern String sendLine;
extern unsigned long lastPacketTime;
extern const int8_t TXpower;
extern int messageCounter; 

// Taster-Entprellung
extern unsigned long lastDebounceTime;
extern int lastButtonState; 
extern int buttonState;
extern String lastSentMessage;

void debugPrint(uint8_t level, const String& message);

#endif