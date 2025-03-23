#include "MyWifi.h"
#include "global.h"

void MyWifi::connect() {

  //This is here to force the ESP32 to reset the WiFi and initialize correctly.
  WiFi.disconnect(true);
  WiFi.setSleep(false);
  WiFi.enableSTA(true);
  Serial.print(F("WIFI status = "));
  Serial.println(WiFi.getMode());
  WiFi.mode(WIFI_STA);
  //Ende silly mode 

  const char* ssid = config.getSSID();
  const char* pass = config.getPass();
 
  WiFi.begin(ssid, pass); 

  while(WiFi.status() != WL_CONNECTED) {
        wifiReconnects++;
        lastReconnectMillis = millis();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, pass);
        Serial.print(F("."));
        delay(3000); 
  }

  if(connected()) {
        Serial.printf("\nNew Client. RSSi: %ld dBm\n",WiFi.RSSI()); 
        Serial.print(F("Ip Address: "));
      
        IPAddress ip = WiFi.localIP();
        Serial.println(ip);
        _localIP = ip;
        _ip=ip.toString();
  }
}

String MyWifi::getIpAddress() {
  return _ip;
}

IPAddress MyWifi::localIP() {
  return _localIP;
}

void MyWifi::reconnect() {
  long now = millis();
  if((now - lastReconnectMillis) > 60000) { //Ein Reconnect max. jede Minute
    if(wifiReconnects >= myWifiRestartLimit) {
      String msg = F("Nach {myWifiRestartLimit} Wifi Reconnects: Esp.restart()");
      Serial.println(msg);
      wifiReconnects=0;
      ESP.restart();
    } else {    
      debugPrint(DEBUG_WIFI, F("Restarting WiFi..."));
      connect();
    }
  }
}

bool MyWifi::connected() {
  return WiFi.status() == WL_CONNECTED;
}

void MyWifi::print() {
  Serial.println(F("--------------------------------"));
  Serial.print(F("Wifi reconnects: "));
  Serial.println(wifiReconnects);
  Serial.print(F("Wifi IP: "));
  Serial.println(localIP());  
  Serial.print(F("MyWifi.connected() :: "));
  Serial.println(connected());   
}
