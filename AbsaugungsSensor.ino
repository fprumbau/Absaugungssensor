#include <Wire.h>
#include <SPI.h>
#include <SX126XLT.h>  // SX12XX-LoRa Bibliothek
#include "SSD1306Wire.h"
#include "pins_arduino.h"

#define TASTER 19  // Taster an GPIO 19
#define DEBOUNCE_DELAY 100  // Entprellungszeit in ms

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

// OLED-Display initialisieren
SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED);

// LoRa-Instanz
SX126XLT LT;

// Status-Variablen
String statusLine = "Board laeuft...";
String secondsLine = "Sekunden: 0";
String tasterLine = "Taster: OFF";
String sendLine = "";
unsigned long lastPacketTime = 0;  // Zeit des letzten empfangenen Pakets
const int8_t TXpower = 10;  // 10 dBm – Sendeleistung
int messageCounter = 1;  // Laufender Zähler für Nachrichten

// Taster-Entprellung
unsigned long lastDebounceTime = 0;  // Zeit des letzten Tasterwechsels
int lastButtonState = HIGH;  // Letzter Tasterstatus
int buttonState = HIGH;      // Aktueller Tasterstatus
String lastSentMessage = "";  // Zuletzt gesendete Nachricht

// Debug-Level (Bitmasken)
// 1 (2^0): Loop Start/Ende Meldungen
// 2 (2^1): LoRa-Statusmeldungen (Prüfe auf Pakete)
// 4 (2^2): LoRa-Nachrichten
const uint8_t debug = 4;  // Nur LoRa-Nachrichten (4)

void VextON() {
  pinMode(36, OUTPUT);  // Vext auf GPIO 36
  digitalWrite(36, LOW);
}

void VextOFF() {
  pinMode(36, OUTPUT);
  digitalWrite(36, HIGH);
}

void displayReset() {
  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, LOW);  // Reset aktivieren
  delay(10);                    // Längeres Timing
  digitalWrite(RST_OLED, HIGH); // Reset deaktivieren
  delay(10);                    // Wartezeit nach Reset
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("AbsaugungsSensor startet...");

  // Taster initialisieren
  pinMode(TASTER, INPUT_PULLUP);  // Pullup, LOW = gedrückt

  // Heltec-spezifische Display-Steuerung
  VextON();
  displayReset();

  // I2C initialisieren
  Wire.begin(SDA_OLED, SCL_OLED);
  if (debug & 2) Serial.println("I2C initialisiert");

  // Display initialisieren
  if (!display.init()) {
    Serial.println("OLED-Initialisierung fehlgeschlagen!");
    statusLine = "ERR";
    secondsLine = "";
    tasterLine = "";
    sendLine = "OLED-Fehler";
  } else {
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    if (debug & 2) Serial.println("OLED initialisiert");
    // Initiale Meldung "Starte..."
    display.clear();
    display.drawString(0, 0, "Starte...");
    display.display();
    delay(1000);  // 1 Sekunde warten
  }

  // SPI initialisieren
  SPI.begin(SCK_LoRa, MISO_LoRa, MOSI_LoRa, SS_LoRa);
  if (debug & 2) Serial.println("SPI initialisiert");

  // LoRa initialisieren
  if (!LT.begin(SS_LoRa, RST_LoRa, BUSY_LoRa, DIO1_LoRa, SW_LoRa, DEVICE_SX1262)) {
    Serial.println("LoRa-Initialisierung fehlgeschlagen!");
    statusLine = "ERR";
    secondsLine = "";
    tasterLine = "";
    sendLine = "LoRa-Fehler";
  } else {
    if (debug & 2) Serial.println("LoRa initialisiert erfolgreich!");
    // LoRa-Parameter setzen (müssen mit Sender übereinstimmen)
    LT.setupLoRa(868000000, 0, LORA_SF7, LORA_BW_125, LORA_CR_4_5, LDRO_AUTO);  // 868 MHz, SF7, BW 125 kHz
  }
}

void loop() {
  // Taster prüfen mit Entprellung
  int reading = digitalRead(TASTER);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {  // Taster von OFF (HIGH) zu ON (LOW)
        String message = "BlaBla " + String(messageCounter++);
        if (debug & 4) Serial.println("Sende LoRa-Nachricht: " + message);
        uint8_t buffer[255];
        uint8_t len = message.length();
        for (uint8_t i = 0; i < len; i++) {
          buffer[i] = message[i];
        }
        if (LT.transmit(buffer, len, 1000, TXpower, WAIT_TX)) {
          if (debug & 4) Serial.println("Nachricht gesendet: " + message);
          lastSentMessage = message;
        } else {
          Serial.println("Fehler beim Senden!");
          lastSentMessage = "Fehler";
        }
      }
      // Tasterstatus aktualisieren
      tasterLine = (buttonState == HIGH) ? "Taster: OFF" : "Taster: ON";
      // Sendestatus aktualisieren
      if (buttonState == LOW && lastSentMessage != "") {
        sendLine = "Sende: " + lastSentMessage;
      } else if (buttonState == HIGH) {
        sendLine = "";  // Zurücksetzen, wenn Taster losgelassen
      }
    }
  }
  lastButtonState = reading;

  // LoRa-Nachricht empfangen
  uint8_t buffer[255];  // Puffer für empfangene Daten
  uint8_t maxLen = sizeof(buffer);
  int16_t len = LT.receive(buffer, maxLen, 2000, NO_WAIT);  // Nicht-blockierend
  if (len > 0) {  // Prüfe, ob Daten empfangen wurden
    String message = "";
    for (uint8_t i = 0; i < len; i++) {
      message += (char)buffer[i];
    }
    statusLine = "Board laeuft...";
    secondsLine = "Sekunden: " + String(millis() / 1000);
    tasterLine = (buttonState == HIGH) ? "Taster: OFF" : "Taster: ON";
    sendLine = "Empf.: " + message + " RSSI: " + String(LT.readPacketRSSI());
    lastPacketTime = millis();  // Zeit des letzten Pakets speichern
    if (debug & 4) Serial.println("LoRa-Nachricht: " + message + " RSSI: " + String(LT.readPacketRSSI()));
  } else {
    // Wenn kein Paket empfangen, aber letzte Nachricht < 10s her, behalte Empfang
    if (millis() - lastPacketTime < 10000) {
      // Behalte sendLine vom letzten Paket
    } else if (buttonState == HIGH) {
      statusLine = "Board laeuft...";
      secondsLine = "Sekunden: " + String(millis() / 1000);
      tasterLine = "Taster: OFF";
      sendLine = "";
    } else {
      statusLine = "Board laeuft...";
      secondsLine = "Sekunden: " + String(millis() / 1000);
      tasterLine = "Taster: ON";
      if (lastSentMessage != "") sendLine = "Sende: " + lastSentMessage;
    }
  }

  // Display aktualisieren
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, statusLine);
  display.drawString(0, 16, secondsLine);
  display.drawString(0, 32, tasterLine);
  display.drawString(0, 48, sendLine);
  display.display();

  delay(100);  // Kürzerer Delay für flüssigere Updates
}