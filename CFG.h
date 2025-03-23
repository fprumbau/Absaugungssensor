#ifndef CFG_H
#define CFG_H

#include "global.h"

//if we cannot mount the LittleFS disk, shall we format one?
#define FORMAT_LITTLEFS_IF_FAILED true

class CFG {
  private:
        char* _ssid; // = "P...y";
        char* _pass; // = "5...7";
        void init();
  public:
        void load();
        const char* load(const String& key);        
        bool save();
        bool save(const String& key, const String& val);
        void print();
        void set(const String& keyVal);
        void set(const char* key, const char* val);
        String getValue(String data, char separator, int index);
        char* ssid();
        char* pass();
};

extern CFG config; // Globales Konfig-Objekt

#endif