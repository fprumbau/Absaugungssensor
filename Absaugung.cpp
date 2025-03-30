#include "Absaugung.h"

void Absaugung::init() {
    debugPrint(DEBUG_ABSG, "Initialisiere Absaugung");
    if (!lora.init()) {
      Serial.println("LoRa initialization failed!");
      while (1);
    }
}

void Absaugung::toggle() {
  if(started()) {
    stop();
  } else {
    start();
  }
}

bool Absaugung::start() {
    debugPrint(DEBUG_ABSG, "Starte Absaugung");
    lastSendTime = millis();
    awaitingConfirmation = true;
    return lora.send(START);
}

bool Absaugung::stop() {
    debugPrint(DEBUG_ABSG, "Stoppe Absaugung");
    lastSendTime = millis();
    awaitingConfirmation = true;
    return lora.send(STOP);
}

bool Absaugung::awaitsConfirmation() {
  return awaitingConfirmation;
}

bool Absaugung::started() {
  return isStarted;
}

bool Absaugung::stopped() {
  return !isStarted;
}

void Absaugung::loop() {
    // Neue Empfangslogik
    uint8_t sensorId, action;
    if (lora.receive(sensorId, action)) {
        if (sensorId == SENSOR_ID) {
            if (action == STARTED) { // "started" empfangen
                isStarted = true;
                awaitingConfirmation = false;
                debugPrint(LORA_MSGS, "Received confirmation: sensor" + String(SENSOR_ID) + ": started");
                lastActivityTime = millis(); // Aktivität zurücksetzen
            } else if (action == STOPPED) { // "gestoppt" empfangen
                isStarted = false;
                awaitingConfirmation = false;
                debugPrint(LORA_MSGS, "Received confirmation: sensor" + String(SENSOR_ID) + ": gestoppt");
                lastActivityTime = millis(); // Aktivität zurücksetzen
            }
        }
    }
}

Absaugung absaugung;

