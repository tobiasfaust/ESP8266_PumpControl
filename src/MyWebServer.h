// https://github.com/esp8266/Arduino/issues/3205
// https://github.com/Hieromon/PageBuilder
// https://www.mediaevent.de/tutorial/sonderzeichen.html
//
// https://byte-style.de/2018/01/automatische-updates-fuer-microcontroller-mit-gitlab-und-platformio/
// https://community.blynk.cc/t/self-updating-from-web-server-http-ota-firmware-for-esp8266-and-esp32/18544
// https://forum.fhem.de/index.php?topic=50628.0

#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include "CommonLibs.h" 
#include <ArduinoJson.h>
#include "uptime.h"
#include "html_update.h"
#include "_Release.h"

#include "BaseConfig.h"
#include "JavaScript.h"
#include "JsAjax.h"
#include "CSS.h"
#include "sensor.h"
#include "valveStructure.h"
#include "valveRelation.h"

extern BaseConfig* Config;
extern MQTT* mqtt;
extern sensor* LevelSensor;
extern valveStructure* VStruct;
extern valveRelation* ValveRel;
extern i2cdetect* I2Cdetect;

class MyWebServer {

  enum page_t {ROOT, BASECONFIG, SENSOR, VENTILE, ONEWIRE, RELATIONS};
  
  public:
    MyWebServer(AsyncWebServer *server, DNSServer* dns);

    void      loop();

  private:
    
    bool      DoReboot;

    AsyncWebServer* server;
    DNSServer* dns;
        
    void      handle_update_page(AsyncWebServerRequest *request);
    void      handle_update_progress(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);    
    void      handle_update_response(AsyncWebServerRequest *request);
    void      handleNotFound(AsyncWebServerRequest *request);
    void      handleReboot(AsyncWebServerRequest *request);
    void      handleReset(AsyncWebServerRequest *request);
    void      handleWiFiReset(AsyncWebServerRequest *request);
    void      handleCSS(AsyncWebServerRequest *request);
    void      handleJS(AsyncWebServerRequest *request);
    void      handleJsAjax(AsyncWebServerRequest *request);
    void      handleJSParam(AsyncWebServerRequest *request);
    void      handleRoot(AsyncWebServerRequest *request);
    
    void      handleBaseConfig(AsyncWebServerRequest *request);
    void      handleVentilConfig(AsyncWebServerRequest *request);
    void      handle1WireConfig(AsyncWebServerRequest *request);
    void      handleSensorConfig(AsyncWebServerRequest *request);
    void      handleRelations(AsyncWebServerRequest *request);
    void      handleAjax(AsyncWebServerRequest *request);
    void      ReceiveJSONConfiguration(AsyncWebServerRequest *request, page_t page);
    void      getPageHeader(AsyncResponseStream *response, page_t pageactive);
    void      getPageFooter(AsyncResponseStream *response);
    
    void      getPage_Status(AsyncResponseStream *response);
  
};

#endif
