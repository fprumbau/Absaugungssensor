#ifndef CFG_H
#define CFG_H

#include "global.h"
#include <LittleFS.h>

class CFG {
private:
  const char* SSID_KEY = "ssid";
  const char* PASS_KEY = "pass";
  const char* CFG_FILE = "/config.json";
  String ssid; // String statt const char* für dynamische Werte
  String pass;

  bool initializeFS(); // Private Methode für FS-Initialisierung

public:
  CFG();
  bool load();              // Lädt die Konfiguration aus LittleFS
  bool save();              // Speichert die Konfiguration in LittleFS
  const char* getSSID() const; // Gibt SSID zurück
  const char* getPass() const; // Gibt Passwort zurück
  const char* load(const String& key); // Lädt einen spezifischen Wert
  bool setValue(const String& key, const String& value, bool saveNow = true); // Setzt einen Wert und speichert optional
};

extern CFG config;

#endif