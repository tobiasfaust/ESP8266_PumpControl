
#ifndef VALVESTRUCTURE_H
#define VALVESTRUCTURE_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <FS.h> 
#include <ArduinoJson.h>
#include <i2cdetect.h>
#include "valveRelation.h"
#include "valve.h"
#include "JavaScript.h"
#include "CSS.h"
  
class valveStructure {

  public:
    valveStructure(uint8_t sda, uint8_t scl);
    void      loop();
    void      OnForTimer(String SubTopic, int duration);

    void      StoreJsonConfig(String json); 
    void      LoadJsonConfig();
    void      GetWebContent(String* html);
    void      getWebJsParameter(String* html);
  
  private:
    void      addValve(String SubTopic); // Virtual
    void      addValve(uint8_t Port, String SubTopic); // Normal

    valveHardware* ValveHW = NULL;
    std::vector<valve> *Valves = NULL;
    i2cdetect* I2Cdetect = NULL;
    
    uint8_t pin_sda = SDA;
    uint8_t pin_scl = SCL;
};

#endif
