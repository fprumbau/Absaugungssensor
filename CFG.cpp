#include "CFG.h"

#define WEBUSER "webUser"
#define WEBPASS "webPass"

CFG config; //Definition

void CFG::load() {

  if(!LittleFS.begin()) {
    return;
  }
  File configFile = LittleFS.open("/config.json", "r");
  if(!configFile){
    return;
  }
  size_t size = configFile.size();
  if(size>8192) {
     Serial.println(F("Config file is to large"));
     return;
  }
  //allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  //We don't use String here because ArduinoJson lib req the intput buffer
  // to be mutable. if you don't use ArduionJson, you may as well use
  // configFile.readString instead
  configFile.readBytes(buf.get(), size);
  debugPrint(DEBUG_CONFIG, buf.get());

  DynamicJsonDocument doc(1024);
  
  auto error = deserializeJson(doc, buf.get());

  if(error) {
    Serial.println(F("Failed to parse config file"));
  }

  if(doc.containsKey(WEBUSER) && doc.containsKey(WEBPASS)) {
    //Webzugang
    const char* webUser = doc[WEBUSER];
    _webUser = new char[strlen(webUser)+1];
    strcpy(_webUser, webUser); 
  
    const char* webPass = doc[WEBPASS];
    _webPass = new char[strlen(webPass)+1];
    strcpy(_webPass, webPass);

    if (debugLevel & 32) {
      Serial.print(F("\nInitialisiere User/Pw: user:|"));
      Serial.print(_webUser);
      Serial.print(F("|; password:|"));
      Serial.print(_webPass);
      Serial.println(F("|"));
    }
        
  }
}

bool CFG::save() {
  
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    return false;
  }

  DynamicJsonDocument doc(1024);
  
  doc[WEBUSER] = _webUser;
  doc[WEBPASS] = _webPass;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }
  if (debugLevel & 32) {
    serializeJson(doc, Serial);
  }
  serializeJson(doc, configFile);
  Serial.println(F("\nKonfiguration wurde erfolgreich gespeichert."));

  configFile.flush();
  configFile.close();
  
  return true;
}

void CFG::set(const String& keyVal) {

  String key = getValue(keyVal, ':', 0);
  String val = getValue(keyVal, ':', 1);

  set(key.c_str(), val.c_str());
}

void CFG::set(const char* key, const char* val) {

  if (debugLevel & 32) {
    Serial.print(F("Set config value '"));
    Serial.print(key);
    Serial.print(F("' to '"));
    Serial.print(val);
    Serial.println(F("'; Still has to be saved"));
  }

  String keyStr = String(key);

  if(keyStr == WEBUSER) {
      _webUser = new char[strlen(val)+1];
      strcpy(_webUser, val);    
  } else if(keyStr == WEBPASS) {
      _webPass = new char[strlen(val)+1];
      strcpy(_webPass, val);
  } else {
      Serial.print(F("Fuer diesen Konfigwert wurde keine Verarbeitung gefunden: "));
      Serial.println(key);
  }
}

String CFG::getValue(String data, char separator, int index) {
  
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

char* CFG::webUser() {
  return _webUser;
}
char* CFG::webPass() {
  return _webPass;
}
const char* CFG::load(const String& key) {
  if(!LittleFS.begin()) {
    return NULL;
  }
  File configFile = LittleFS.open("/config.json", "r");
  if(!configFile){
    return NULL;
  }
  size_t size = configFile.size();
  if(size>8192) {
     Serial.println(F("Config file is to large"));
     return NULL;
  }
  //allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  //We don't use String here because ArduinoJson lib req the intput buffer
  // to be mutable. if you don't use ArduionJson, you may as well use
  // configFile.readString instead
  configFile.readBytes(buf.get(), size);
  if (debugLevel & 32) {
    Serial.println(buf.get());
  }

  DynamicJsonDocument doc(1024);
  
  auto error = deserializeJson(doc, buf.get());

  if(error) {
    Serial.println(F("Failed to parse config file"));
  }
  
  const char* val = doc[key];    
  return val;        
}       
bool CFG::save(const String& key, const String& val) {

  if(!LittleFS.begin()) {
    return NULL;
  }
  File configFile = LittleFS.open("/config.json", "r");
  if(!configFile){
    return NULL;
  }
  size_t size = configFile.size();
  if(size>8192) {
     Serial.println(F("Config file is to large"));
     return NULL;
  }
  //allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  //We don't use String here because ArduinoJson lib req the intput buffer
  // to be mutable. if you don't use ArduionJson, you may as well use
  // configFile.readString instead
  configFile.readBytes(buf.get(), size);
  debugPrint(DEBUG_CONFIG, buf.get());

  DynamicJsonDocument doc(1024);
  
  auto error = deserializeJson(doc, buf.get());

  if(error) {
    Serial.println(F("Failed to parse config file"));
  }  

  doc[key]=val;  

  Serial.print("Key: ");
  Serial.print(key);
  Serial.print("; Value: ");
  Serial.println(val);
  
  serializeJson(doc, Serial);
  serializeJson(doc, configFile);
  Serial.println(F("\nKonfiguration wurde erfolgreich gespeichert."));

  configFile.flush();
  configFile.close();  
  return true;
}

void CFG::print() {
  Serial.println(F("--------------------------------"));
  Serial.print(F("_webUser: "));
  Serial.println(_webUser);   
}
