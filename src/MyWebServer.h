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
#include "uptime.h" // https://github.com/YiannisBourkelis/Uptime-Library/
#include "uptime_formatter.h"
#include "html_update.h"
#include "_Release.h"
#include "handleFiles.h"

#include "baseconfig.h"
#include "sensor.h"
#include "valveStructure.h"
#include "valveRelation.h"

extern sensor* LevelSensor;
extern valveStructure* VStruct;
extern valveRelation* ValveRel;

#ifdef USE_I2C
  extern i2cdetect* I2Cdetect;
#endif

#ifdef ESP8266
  #define ESPGPIO "gpio_esp8266.js"
#elif ESP32
  #define ESPGPIO "gpio_esp32.js"
#endif

class MyWebServer {

  public:
    MyWebServer(AsyncWebServer *server, DNSServer* dns);

    void      loop();

  private:

    AsyncWebServer* server;
    DNSServer* dns;

    bool      DoReboot;
    unsigned long RequestRebootTime;

    handleFiles* fsfiles;
    
    void      handle_update_page(AsyncWebServerRequest *request);
    void      handle_update_progress(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);    
    void      handle_update_response(AsyncWebServerRequest *request);
    void      handleNotFound(AsyncWebServerRequest *request);
    void      handleReboot(AsyncWebServerRequest *request);
    void      handleReset(AsyncWebServerRequest *request);
    void      handleWiFiReset(AsyncWebServerRequest *request);
    void      handleRequestFiles(AsyncWebServerRequest *request);
    void      handleRoot(AsyncWebServerRequest *request);
    void      handleJSParam(AsyncWebServerRequest *request);
    
    void      handleAjax(AsyncWebServerRequest *request);
    void      GetInitDataStatus(AsyncResponseStream *response);
    void      GetInitDataNavi(AsyncResponseStream *response);
  
};

#endif
