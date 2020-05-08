#ifndef UPDATER_H
#define UPDATER

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <vector>
#include "ArduinoJson.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
//#include <WiFiClientSecureBearSSL.h> //HTTPS
#include <WiFiClient.h> // HTTP

#include "_Release.h";

  enum stage_t {UNDEF, PROD, PRE, DEV};
  
  typedef struct {
    String name;
    String version;
    int number;
    stage_t stage;
    String downloadURL;
  } release_t;

class updater {
  
  public:
    updater();
    void        setIndexJson(String url);
    void        setStage(stage_t s);
    void        setAutoMode(bool a);
    void        setInterval(int seconds);
    void        loop();
    release_t*  GetCurrentRelease();
    String      GetReleaseName();
    const int&  GetInterval()     const {return interval;}
    std::vector<release_t>* GetReleases();
    void        InstallRelease(String ReleaseName);
    void        RefreshReleases();
    String      GetUpdateErrorString();

  private:
    //BearSSL::WiFiClientSecure* client;
    WiFiClient* client;
    
    void        Update();
    void        downloadJson();
    void        parseJson(String* json);
    //String*     getURLHostName(String* url);
    //String*     getURLPath(String* url);
    void        LoadJsonConfig();
    void        StoreJsonConfig(release_t* r);
    release_t   getLatestRelease();
    stage_t     String2Stage(String s);
    String      Stage2String(stage_t s);
    
    String      json_url;
    stage_t     stage;
    release_t   currentRelease;
    bool        automode;
    bool        updateError;
    int         interval;
    bool        DoUpdate = false;
    long        lastupdate;

    std::vector<release_t>* releases = NULL;
};

#endif
