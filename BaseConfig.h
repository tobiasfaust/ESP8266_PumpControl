#ifndef BASECONFIG_H
#define BASECONFIG_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#if defined(ESP8266) || defined(ESP32)
  #define min(x,y) _min(x,y);
#endif

#include <FS.h> 
#include "ArduinoJson.h"
#include "oled.h"

extern OLED* oled;

class BaseConfig {

  public:
    BaseConfig();
    void      StoreJsonConfig(String* json); 
    void      LoadJsonConfig();
    void      GetWebContent(String* html);
    const uint8_t& GetPinSDA()      const {return pin_sda;}
    const uint8_t& GetPinSCL()      const {return pin_scl;}
    const uint8_t& GetI2cOLED()     const {return i2caddress_oled;}
    const bool&    EnabledOled()    const {return enable_oled;}
    const String&  GetMqttServer()  const {return mqtt_server;}
    const uint16_t& GetMqttPort()   const {return mqtt_port;}
    const String&  GetMqttUsername()const {return mqtt_username;}
    const String&  GetMqttPassword()const {return mqtt_password;}
    const String&  GetMqttRoot()    const {return mqtt_root;}
    const uint8_t& Get3WegePort()   const {return ventil3wege_port;}
    const bool&    Enabled3Wege()   const {return enable_3wege;}
    const uint8_t& GetMaxParallel() const {return max_parallel;}
    
  private:
    String    mqtt_server;
    String    mqtt_username;
    String    mqtt_password;
    uint16_t  mqtt_port;
    String    mqtt_root;
    uint8_t   pin_sda;
    uint8_t   pin_scl;
    bool      enable_oled;
    uint8_t   i2caddress_oled;
    bool      enable_3wege; // wechsel Regen- /Trinkwasser
    uint8_t   ventil3wege_port; // Portnummer des Ventils
    uint8_t   max_parallel;
};

#endif

