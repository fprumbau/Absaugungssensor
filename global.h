#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

#include "SX126XLT.h"
#include <SPI.h>
#include <Wire.h>
#include "ADXL.h"
#include "LoRa.h"
#include "MyWifi.h"
#include "OTA.h"
#include "display.h"
#include "SSD1306Wire.h"
#include "pins_arduino.h"
#include <CFG.h>

#define TASTER_PIN 0  // Taster
#define DEBOUNCE_DELAY 100  // Entprellungszeit in ms

// Debug levels
#define DEBUG_LOOP    (1 << 0) // 0
#define DEBUG_LORA    (1 << 1) // 1
#define LORA_MSGS     (1 << 2) // 2
#define DEBUG_ADXL    (1 << 3) // 4
#define DEBUG_INIT    (1 << 4) // 8
#define DEBUG_DISPLAY (1 << 5) // 16
#define DEBUG_CONFIG  (1 << 6) // 32
#define DEBUG_WIFI    (1 << 7) // 64

extern AsyncWebServer server; 

extern uint8_t debugLevel;

// Status-Variablen
extern const int8_t TXpower;
extern int messageCounter; 

// Taster-Entprellung
extern unsigned long lastDebounceTime;
extern int lastButtonState; 
extern int buttonState;
extern String lastSentMessage;

extern bool TasterGedrueckt;
extern unsigned long TasterPressTime;
extern const long TasterEntprellZeit;

void debugPrint(uint8_t level, const String& message);

#endif