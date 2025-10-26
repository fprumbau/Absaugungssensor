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
#include "Web.h"
#include "display.h"
#include "Absaugung.h"
#include "SSD1306Wire.h"
#include "pins_arduino.h"
#include <CFG.h>

#define STARTE 1
#define STOPPE 2
#define STARTED 3
#define STOPPED 4
#define ACK 5
#define QUERY 6

#define TASTER_PIN 0  // Taster
#define DEBOUNCE_DELAY 100  // Entprellungszeit in ms

// Debug levels
const uint16_t DEBUG_LOOP  = (1 << 0); // 0
const uint16_t DEBUG_LORA  = (1 << 1); // 1
const uint16_t LORA_MSGS   = (1 << 2); // 2
const uint16_t DEBUG_ADXL  = (1 << 3); // 4
const uint16_t DEBUG_INIT  = (1 << 4); // 8
const uint16_t DEBUG_DISPLAY = (1 << 5); // 16
const uint16_t DEBUG_CONFIG  = (1 << 6); // 32
const uint16_t DEBUG_WIFI    = (1 << 7); // 64
const uint16_t DEBUG_ABSG    = (1 << 8); // 128
const uint16_t DEBUG_SWITCH  = (1 << 9); // 256

extern AsyncWebServer server; 

extern uint16_t debugLevel;

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

extern unsigned long lastActivityTime;
extern bool sleepAnnounced;

extern const long maxShortPressTime;
extern bool TasterState;

void debugPrint(uint16_t level, const String& message);

#endif