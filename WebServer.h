// https://github.com/esp8266/Arduino/issues/3205
// https://github.com/Hieromon/PageBuilder
// https://www.mediaevent.de/tutorial/sonderzeichen.html

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
//#include "uptime_formatter.h"
#include "uptime.h"
#include "_Release.h"

#include "BaseConfig.h"
#include "JavaScript.h"
#include "JsAjax.h"
#include "CSS.h"
#include "sensor.h"
#include "valveStructure.h"
#include "valveRelation.h"

extern BaseConfig* Config;
extern sensor* LevelSensor;
extern valveStructure* VStruct;
extern valveRelation* ValveRel;
extern i2cdetect* I2Cdetect;

class WebServer {

  enum page_t {ROOT, BASECONFIG, SENSOR, VENTILE, RELATIONS};
  
  public:
    WebServer();

    void      loop();

  private:
    
    bool      DoReboot;
    MDNSResponder mdns;
    ESP8266WebServer* server;
    ESP8266HTTPUpdateServer httpUpdater;
    
    void      handleNotFound();
    void      handleReboot();
    void      handleCSS();
    void      handleJS();
    void      handleJsAjax();
    void      handleJSParam();
    void      handleRoot();
    void      handleBaseConfig();
    void      handleVentilConfig();
    void      handleSensorConfig();
    void      handleRelations();
    void      handleAjax();
    void      ReceiveJSONConfiguration(page_t page);
    void      getPageHeader(String* html, page_t pageactive);
    void      getPageFooter(String* html);
    
    void      getPage_Status(String* html);
  
};

#endif

