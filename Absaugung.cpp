#include "Absaugung.h"

void Absaugung::init() {
    debugPrint(DEBUG_ABSG, "Initialisiere Absaugung");
    if (!lora.init()) {
      Serial.println("LoRa initialization failed!");
      while (1);
    }
    initialized = true;
}

void Absaugung::toggle() {
  if(started()) {
    stop();
  } else {
    start();
  }
}

bool Absaugung::start() {
    if(!initialized) {
       return false;
    }
    debugPrint(DEBUG_ABSG, "   Starte Absaugung");
    lastSendTime = millis();
    awaitingConfirmation = true;
    return lora.send(STARTE);
}

bool Absaugung::stop() {
    if(!initialized) {
       return false;
    }
    debugPrint(DEBUG_ABSG, "   Stoppe Absaugung");
    lastSendTime = millis();
    awaitingConfirmation = true;
    return lora.send(STOPPE);
}

bool Absaugung::awaitsConfirmation() {
  return awaitingConfirmation;
}

bool Absaugung::started() {
  if(isStarted && (millis()-startTimeout) > 300000) {
      debugPrint(DEBUG_ABSG, "Nach 5 Minuten wird Startet wegen Sleep zurueckgenommen");
      lastActivityTime = millis(); // Aktivität zurücksetzen
      isStarted = false;
  }
  return isStarted;
}

bool Absaugung::stopped() {
  if(isStarted && (millis()-startTimeout) > 300000) {
      debugPrint(DEBUG_ABSG, "Nach 5 Minuten wird Startet wegen Sleep zurueckgenommen");
      lastActivityTime = millis(); // Aktivität zurücksetzen
      isStarted = false;
  }
  return !isStarted;
}

void Absaugung::loop() {
    if(!initialized) {
       return;
    }
    // Neue Empfangslogik
    uint8_t sensorId, action;
    if (lora.receive(sensorId, action)) {
        if (sensorId == SENSOR_ID) {
            if (action == STARTED) { // "started" empfangen
                isStarted = true;
                awaitingConfirmation = false;
                debugPrint(LORA_MSGS, "     Received confirmation: sensor" + String(SENSOR_ID) + ": started");
                lastActivityTime = millis(); // Aktivität zurücksetzen
                lora.send(ACK);
                startTimeout = millis();
            } else if (action == STOPPED) { // "gestoppt" empfangen
                isStarted = false;
                awaitingConfirmation = false;
                debugPrint(LORA_MSGS, "     Received confirmation: sensor" + String(SENSOR_ID) + ": gestoppt");
                lastActivityTime = millis(); // Aktivität zurücksetzen
                lora.send(ACK);
            }
        } else {
             //debugPrint(LORA_MSGS, "          Received confirmation for sensor" + String(sensorId) + ": " + lora.actionToString(action));
        }
    }
}

const char* Absaugung::status() {
  if(isStarted) {
    return "ON";
  }
  return "OFF";
}

Absaugung absaugung;

