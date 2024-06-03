#ifndef UPDATER_H
#define UPDATER_H

#include "CommonLibs.h"
#include <vector>
#include "ArduinoJson.h"
#include <StreamUtils.h>
#include "_Release.h"

#ifdef ESP8266
   #include <ESP8266httpUpdate.h>
   #include <ESP8266HTTPClient.h>
   #include <WiFiClient.h> // HTTP

   using WM_httpUpdate = ESP8266HTTPUpdate;
   
#elif ESP32
  #include <HTTPClient.h>
  #include <HTTPUpdate.h>
  
  using WM_httpUpdate  = HTTPUpdate;
#endif 

enum stage_t {UNDEF, PROD, PRE, DEV};
  
typedef struct {
    String name;
    String version;
    uint32_t subversion = 0;
    uint32_t number = 0;
    stage_t stage;
    String downloadURL;
  } release_t;

class updater {
  
  private:
    WiFiClient*     WifiClient;
    WM_httpUpdate*  httpUpdate;
    
    void        Update();
    void        downloadJson();
    void        parseJson(WiFiClient stream);
    void        LoadJsonConfig();
    void        StoreJsonConfig(release_t* r);
    void        printRelease(release_t* r);
    void        InstallLatestRelease();
    release_t   GetLatestRelease()   const {return latestRelease;}
    const uint8_t& GetDebugLevel()   const {return debuglevel;}

    bool        DoUpdate = false;
    bool        automode;
    bool        updateError;
    uint32_t    interval;
    uint8_t     debuglevel;
    String      json_url;
    stage_t     stage;
    release_t   currentRelease;
    uint32_t    lastupdate;

    release_t   latestRelease;
    //std::vector<release_t>* releases = NULL;
    
  public:
    updater();
    void        setIndexJson(String url);
    void        setStage(stage_t s);
    void        setAutoMode(bool a);
    void        setInterval(uint32_t seconds);
    void        loop();
    release_t*  GetCurrentRelease();
    String      GetReleaseName();
    const uint32_t&  GetInterval()     const {return interval;}
    String      GetUpdateErrorString();
    stage_t     String2Stage(String s);
    String      Stage2String(stage_t s);
    void        SetDebugLevel(uint8_t value);
};

#endif
