#include "CFG.h"
#include <ArduinoJson.h>

CFG::CFG() : ssid("default_ssid"), pass("default_pass") {}

bool CFG::initializeFS() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed, attempting format...");
    if (!LittleFS.format()) {
      Serial.println("LittleFS Format Failed");
      return false;
    }
    if (!LittleFS.begin()) {
      Serial.println("LittleFS Mount Failed after format");
      return false;
    }
    debugPrint(DEBUG_DISPLAY, "LittleFS formatted and mounted");
  }
  return true;
}

bool CFG::load() {
  if (!initializeFS()) {
    return false;
  }
  File file = LittleFS.open(CFG_FILE, "r");
  if (!file) {
    Serial.println("No config file found, using defaults");
    save(); // Erstelle eine neue Datei mit Standardwerten
    return true;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to parse config file: " + String(error.c_str()));
    file.close();
    Serial.println("Reinitializing LittleFS...");
    LittleFS.end();
    if (!initializeFS()) {
      return false;
    }
    save(); // Erstelle eine neue Datei mit Standardwerten
    return true;
  }

  if (doc[SSID_KEY].is<String>() && doc[PASS_KEY].is<String>()) {
    ssid = doc[SSID_KEY].as<String>();
    pass = doc[PASS_KEY].as<String>();
    debugPrint(DEBUG_DISPLAY, "Config loaded: SSID=" + ssid);
  } else {
    Serial.println("Config file invalid, using defaults");
    file.close();
    save(); // Überschreibe mit Standardwerten
    return true;
  }

  file.close();
  return true;
}

bool CFG::save() {
  if (!initializeFS()) {
    return false;
  }
  File file = LittleFS.open(CFG_FILE, "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  JsonDocument doc;
  doc[SSID_KEY] = ssid;
  doc[PASS_KEY] = pass;

  if (serializeJson(doc, file) == 0) {
    Serial.println("Failed to write to config file");
    file.close();
    return false;
  }

  file.close();
  debugPrint(DEBUG_DISPLAY, "Config saved: SSID=" + ssid);
  return true;
}

const char* CFG::getSSID() const {
  return ssid.c_str();
}

const char* CFG::getPass() const {
  return pass.c_str();
}

const char* CFG::load(const String& key) {
  if (!initializeFS()) {
    return "";
  }
  File file = LittleFS.open(CFG_FILE, "r");
  if (!file) {
    Serial.println("No config file found");
    return "";
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to parse config file: " + String(error.c_str()));
    file.close();
    return "";
  }

  if (doc[key].is<String>()) {
    String value = doc[key].as<String>();
    file.close();
    return value.c_str();
  } else {
    file.close();
    return "";
  }
}

bool CFG::setValue(const String& key, const String& value, bool saveNow) {
  if (key == SSID_KEY) {
    ssid = value;
  } else if (key == PASS_KEY) {
    pass = value;
  } else {
    // Für zukünftige Schlüssel: nur in Datei speichern, ohne interne Variable
    JsonDocument doc;
    File file = LittleFS.open(CFG_FILE, "r");
    if (file) {
      DeserializationError error = deserializeJson(doc, file);
      if (error) {
        Serial.println("Failed to parse config file: " + String(error.c_str()));
      }
      file.close();
    }
    doc[key] = value;
    file = LittleFS.open(CFG_FILE, "w");
    if (!file) {
      Serial.println("Failed to open config file for writing");
      return false;
    }
    if (serializeJson(doc, file) == 0) {
      Serial.println("Failed to write to config file");
      file.close();
      return false;
    }
    file.close();
    debugPrint(DEBUG_DISPLAY, "Set and saved " + key + ": " + value);
    return true;
  }

  if (saveNow) {
    return save();
  }
  debugPrint(DEBUG_DISPLAY, "Set " + key + ": " + value + " (not saved yet)");
  return true;
}

CFG config;