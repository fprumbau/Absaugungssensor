#ifndef CFG_H
#define CFG_H

#include "global.h"
#include <LittleFS.h>

class CFG {
private:
  const char* SSID_KEY = "ssid";
  const char* PASS_KEY = "pass";
  const char* CFG_FILE = "/config.json";
  String ssid;
  String pass;

  bool initializeFS();

public:
  CFG();
  bool load();
  bool save();
  const char* getSSID() const;
  const char* getPass() const;
  const char* load(const String& key);
  bool setValue(const String& key, const String& value, bool saveNow = true);
};

extern CFG config;

#endif