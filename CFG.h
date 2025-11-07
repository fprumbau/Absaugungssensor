#ifndef CFG_H
#define CFG_H

#include "global.h"
#include <LittleFS.h>

class CFG {
private:
  const char* CFG_FILE = "/config.json";

  const char* SSID_KEY = "ssid";
  const char* PASS_KEY = "pass";
  const char* IDLE_KEY = "idleTime";
  const char* SENSOR_KEY = "sensorId";
  const char* PROFILE_KEY = "profile";

  String ssid;
  String pass;
  String profile;
  unsigned long idleTime; //in Sekunden

  bool initializeFS();

public:
  CFG() : ssid("default_ssid"), pass("default_pass"), idleTime(60) {} // Default: 60s
  unsigned long getIdleTime() const { return idleTime; } 
  bool load();
  bool save();
  const char* getSSID() const;
  const char* getPass() const;
  const char* getProfile() const;
  const char* load(const String& key);
  bool setValue(const String& key, const String& value, bool saveNow = true);
};

extern CFG config;
extern uint8_t SENSOR_ID;

#endif