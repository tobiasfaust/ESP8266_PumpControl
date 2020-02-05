#ifndef SENSOR_H
#define SENSOR_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <FS.h> 
#include <ArduinoJson.h>
#include "MQTT.h"
#include "valveStructure.h"
#include "BaseConfig.h"
#include "oled.h"

extern MQTT* mqtt;
extern valveStructure* VStruct;
extern BaseConfig* Config;
extern OLED* oled;

enum sensorType_t {NONE, EXTERN, HCSR04, ANALOG};

class sensor {

  public:
    sensor();
    void      init(uint8_t pinTrigger, uint8_t pinEcho);
    void      init(String externalSensor);
    void      init();
    void      setSensorType(sensorType_t t);
    void      loop();
    void      SetLvl(uint8_t lvl);
    void      StoreJsonConfig(String* json); 
    void      LoadJsonConfig();
    void      GetWebContent(String* html);
    const uint16_t& GetRaw() const {return raw;}
    const uint8_t&  GetLvl() const {return level; }
    const sensorType_t& GetType() const {return Type; }
    const uint8_t& GetThresholdMin()const {return threshold_min;}
    const uint8_t& GetThresholdMax()const {return threshold_max;}
    const String&  GetExternalSensor() const {return externalSensor;}
    
  private:
    void      loop_analog();
    void      loop_hcsr04();
    
    uint8_t   pinTrigger;
    uint8_t   pinEcho;
    uint16_t  raw;
    uint8_t   level;
    uint16_t  MAX_DIST;
    sensorType_t   Type;
    uint16_t  measureDistMin;
    uint16_t  measureDistMax;
    uint16_t  measurecycle;
    uint8_t   threshold_min;
    uint8_t   threshold_max;
    String    externalSensor;
    
    unsigned long previousMillis = 0;
    
};

#endif
