#include "CFG.h"
#include <ArduinoJson.h>

CFG::CFG() : ssid("default_ssid"), pass("default_pass") {}

bool CFG::initializeFS() {
  if (!LittleFS.begin(true)) { // true = format on fail
    Serial.println("LittleFS Mount Failed even with format");
    return false;
  }
  debugPrint(DEBUG_CONFIG, "LittleFS mounted successfully");
  return true;
}

bool CFG::load() {
  if (!initializeFS()) {
    return false;
  }
  File file = LittleFS.open(CFG_FILE, "r");
  if (!file) {
    Serial.println("No config file found, creating new one with current values");
    if (!save()) {
      Serial.println("Failed to create new config file");
      return false;
    }
    return true;
  }

  String content;
  while (file.available()) {
    content += (char)file.read();
  }
  debugPrint(DEBUG_CONFIG, "Config file content: " + content);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, content);
  if (error) {
    Serial.println("Failed to parse config file: " + String(error.c_str()));
    file.close();
    return false;
  }

  if (doc[SSID_KEY].is<String>() && doc[PASS_KEY].is<String>()) {
    ssid = doc[SSID_KEY].as<String>();
    pass = doc[PASS_KEY].as<String>();
    debugPrint(DEBUG_CONFIG, "Config loaded: SSID=" + ssid + ", Pass=" + pass);
  } else {
    Serial.println("Config file invalid, keeping current values");
    file.close();
    return false;
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

  String jsonOutput;
  serializeJson(doc, jsonOutput);
  debugPrint(DEBUG_CONFIG, "Saving config: " + jsonOutput);

  if (file.print(jsonOutput) == 0) {
    Serial.println("Failed to write to config file");
    file.close();
    return false;
  }

  file.close();
  debugPrint(DEBUG_CONFIG, "Config saved: SSID=" + ssid + ", Pass=" + pass);
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

  String content;
  while (file.available()) {
    content += (char)file.read();
  }
  debugPrint(DEBUG_CONFIG, "Config file content: " + content);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, content);
  if (error) {
    Serial.println("Failed to parse config file: " + String(error.c_str()));
    file.close();
    return "";
  }

  if (doc[key].is<String>()) {
    static String value;
    value = doc[key].as<String>();
    file.close();
    debugPrint(DEBUG_CONFIG, "Loaded " + key + ": " + value);
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
    debugPrint(DEBUG_CONFIG, "Set and saved " + key + ": " + value);
    return true;
  }

  if (saveNow) {
    return save();
  }
  debugPrint(DEBUG_CONFIG, "Set " + key + ": " + value + " (not saved yet)");
  return true;
}

CFG config;