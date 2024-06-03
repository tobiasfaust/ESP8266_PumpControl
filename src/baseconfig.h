#ifndef BASECONFIG_H
#define BASECONFIG_H

#include "CommonLibs.h"
#include "ArduinoJson.h"
#include "updater.h"
#include <iomanip>  // needed by setw / setfill

class BaseConfig {

  public:
    BaseConfig();
    void      LoadJsonConfig();
    void      loop();

    const uint8_t&  GetPinSDA()      const {return pin_sda;}
    const uint8_t&  GetPinSCL()      const {return pin_scl;}
    const uint8_t&  GetPin1Wire()      const {return pin_1wire;}
    const uint8_t&  GetI2cOLED()     const {return i2caddress_oled;}
    const bool&     EnabledOled()    const {return enable_oled;}
    const uint8_t&  GetOledType()   const {return oled_type;}
    const bool&     Enabled1Wire()    const {return enable_1wire;}
    const String&   GetMqttServer()  const {return mqtt_server;}
    const uint16_t& GetMqttPort()   const {return mqtt_port;}
    const String&   GetMqttUsername()const {return mqtt_username;}
    const String&   GetMqttPassword()const {return mqtt_password;}
    const String&   GetMqttBasePath()  const {return mqtt_basepath;}
    const String&   GetMqttRoot()    const {return mqtt_root;}
    const bool&     UseRandomMQTTClientID() const { return mqtt_UseRandomClientID; }
    const uint8_t&  Get3WegePort()   const {return ventil3wege_port;}
    const bool&     Enabled3Wege()   const {return enable_3wege;}
    const uint8_t&  GetMaxParallel() const {return max_parallel;}
    const uint16_t& GetKeepAlive()   const {return keepalive;}
    const uint8_t&  GetDebugLevel()   const {return debuglevel;}
    String          GetReleaseName();
    const bool&     GetUseETH()        const { return useETH; }
    void            GetInitData(AsyncResponseStream* response);
    const String&   GetLANBoard()      const {return LANBoard;}

    size_t          getFragmentation();
     
  private:
    String    mqtt_server;
    String    mqtt_username;
    String    mqtt_password;
    uint16_t  mqtt_port;
    String    mqtt_root;
    String    mqtt_basepath;
    bool      mqtt_UseRandomClientID;
    uint16_t  keepalive;
    uint8_t   debuglevel;
    uint8_t   pin_sda;
    uint8_t   pin_scl;
    uint8_t   pin_1wire;
    bool      enable_oled;
    uint8_t   oled_type;
    bool      enable_1wire;
    uint8_t   i2caddress_oled;
    bool      enable_3wege; // wechsel Regen- /Trinkwasser
    uint8_t   ventil3wege_port; // Portnummer des Ventils
    uint8_t   max_parallel;
    bool      enable_autoupdate;
    stage_t   autoupdate_stage;
    String    autoupdate_url;
    bool      useETH;  // otherwise use WIFI
    String    LANBoard;
    
    updater*  ESPUpdate;
};

extern BaseConfig* Config;

#endif
