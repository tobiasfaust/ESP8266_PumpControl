#ifndef SENSOR_H
#define SENSOR_H

#include "CommonLibs.h"
#include "CommonLibs.h"
#include <ArduinoJson.h>
#include <vector>
#include "mqtt.h"
#include "baseconfig.h"
#include "valveStructure.h"

extern valveStructure* VStruct;
extern BaseConfig* Config;

enum sensorType_t {NONE, EXTERN, HCSR04, ONBOARD_ANALOG};

class sensor {

  public:
    sensor();
    void      init_hcsr04(uint8_t pinTrigger, uint8_t pinEcho);
    void      init_extern(String externalSensor);
    void      init_analog(uint8_t pinAnalog) ;
    
    void      setSensorType(sensorType_t t);
    void      loop();
    void      SetLvl(uint8_t lvl);
    void      LoadJsonConfig();
    void      GetInitData(AsyncResponseStream* response);    
    
    const uint16_t& GetRaw() const {return raw;}
    const uint8_t&  GetLvl() const {return level; }
    const sensorType_t& GetType() const {return Type; }
    const uint8_t& GetThresholdMin()const {return threshold_min;}
    const uint8_t& GetThresholdMax()const {return threshold_max;}
    const String&  GetExternalSensor() const {return externalSensor;}

  private:
    void      loop_analog();
    void      loop_hcsr04();
    
    sensorType_t   Type;
    
    uint16_t  measureDistMin;
    uint16_t  measureDistMax;
    uint16_t  measurecycle;
    uint8_t   level;
    uint16_t  raw;
    uint8_t   pinTrigger;
    uint8_t   pinEcho;
    uint8_t   pinAnalog;
    uint16_t  MAX_DIST;
    uint8_t   threshold_min;
    uint8_t   threshold_max;
    String    externalSensor;
    bool      moistureEnabled;
    
    unsigned long previousMillis_sensor = 0;
    unsigned long previousMillis_moisture = 0;

};

#endif
