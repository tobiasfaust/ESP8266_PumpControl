#ifndef WEBSERVER_H
#define WEBSERVER_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <FS.h> 
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

#include "BaseConfig.h"
#include "JavaScript.h"
#include "CSS.h"
#include "uptime.h"
#include "sensor.h"
#include "valveStructure.h"
#include "valveRelation.h"

extern BaseConfig* Config;
extern sensor* LevelSensor;
extern valveStructure* VStruct;
extern valveRelation* ValveRel;
extern i2cdetect* I2Cdetect;

/*
#ifdef __AVR__
  #define CB_HTML_VALVE void (*htmlValve)(String*)
#else
  #include <functional>
  #define CB_HTML_VALVE std::function<void(String*)> htmlValve
#endif
*/

class WebServer {

  enum page_t {ROOT, BASECONFIG, SENSOR, VENTILE, RELATIONS, AUTOCONFIG};

  public:
    WebServer();

    void      loop();
    //void SetHTMLFunction_Valve (CB_HTML_VALVE);

  private:
    
    bool      DoReboot;
    MDNSResponder mdns;
    ESP8266WebServer* server;
    ESP8266HTTPUpdateServer httpUpdater;
    uptime* UpTime = NULL;
    
    //CB_HTML_VALVE;

    void      handleNotFound();
    void      handleReboot();
    void      handleCSS();
    void      handleJS();
    void      handleJSParam();
    void      handleRoot();
    void      handleBaseConfig();
    void      handleVentilConfig();
    void      handleSensorConfig();
    void      handleRelations();
    void      ReceiveJSONConfiguration(page_t page);
    void      getPageHeader(String* html, page_t pageactive);
    void      getPageFooter(String* html);
    
    void      getPage_Status(String* html);
  
};

#endif

