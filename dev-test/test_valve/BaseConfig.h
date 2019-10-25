#ifndef BASECONFIG_H
#define BASECONFIG_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <FS.h> 
#include <ArduinoJson.h>

class BaseConfig {

  public:
    BaseConfig();
    void      StoreJsonConfig(String* json); 
    void      LoadJsonConfig();
    void      GetWebContent(String* html);
    const uint8_t& GetPinSDA()      const {return pin_sda;}
    const uint8_t& GetPinSCL()      const {return pin_scl;}
    const uint8_t& GetI2cOLED()     const {return i2caddress_oled;}
    const String&  GetMqttServer()  const {return mqtt_server;}
    const uint8_t& GetMqttPort()    const {return mqtt_port;}
    const String&  GetMqttRoot()    const {return mqtt_root;}
    const uint8_t& Get3WegePort()   const {return ventil3wege_port;}
    const uint8_t& GetMaxParallel() const {return max_parallel;}
    const uint8_t& GetThresholdMin()const {return threshold_min;}
    const uint8_t& GetThresholdMax()const {return threshold_max;}
    
  private:
    String    mqtt_server;
    uint16_t  mqtt_port;
    String    mqtt_root;
    uint8_t   pin_sda;
    uint8_t   pin_scl;
    bool      enable_oled;
    uint8_t   i2caddress_oled;
    uint8_t   threshold_min;
    uint8_t   threshold_max;
    bool      enable_3wege; // wechsel Regen- /Trinkwasser
    uint8_t   ventil3wege_port; // Portnummer des Ventils
    uint8_t   max_parallel;
};

#endif

