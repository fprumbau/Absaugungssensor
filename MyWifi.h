#ifndef MYWIFI_H
#define MYWIFI_H

#include "global.h"
#include <WiFi.h>

class MyWifi {
public:
  MyWifi();
  bool begin(const char* ssid, const char* password); // Startet WiFi-Modus
  void loop();                                       // Prüft Timeout und Verbindungsstatus
  bool isConnected();                                // Gibt Verbindungsstatus zurück
  void disconnect();                                 // Beendet WiFi-Verbindung
  bool isActive() const;                             // Gibt zurück, ob WiFi-Modus aktiv ist
  void resetTimeout();                               // Setzt Timeout zurück

private:
  String ssid;
  String password;
  bool active;
  unsigned long startTime;
  static constexpr unsigned long TIMEOUT_MS = 120000; // 120 Sekunden
};

extern MyWifi wifi;
extern const long WifiActivationTime; 

#endif