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

enum sensorType_t {NONE, HCSR04, ANALOG};

class sensor {

  public:
    sensor();
    void      init(uint8_t pinTrigger, uint8_t pinEcho);
    void      init();
    void      loop();
    void      StoreJsonConfig(String* json); 
    void      LoadJsonConfig();
    void      GetWebContent(String* html);
    const int& GetRaw() const {return raw;}
    const int& GetLvl() const {return level; }
    const sensorType_t& GetType() const {return Type; }
    
  private:
    void      loop_analog();
    void      loop_hcsr04();
    
    uint8_t   pinTrigger;
    uint8_t   pinEcho;
    int       raw;
    int       level;
    uint16_t  MAX_DIST;
    sensorType_t   Type;
    uint8_t   measureDistMin;
    uint8_t   measureDistMax;
    uint16_t  measurecycle;
    unsigned long previousMillis = 0;
    
};

#endif

