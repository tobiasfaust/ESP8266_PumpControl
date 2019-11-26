
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
#include "BaseConfig.h"
#include "valveRelation.h"
#include "valve.h"
#include "MQTT.h"

extern BaseConfig* Config;

class valveStructure {

  public:
    valveStructure(uint8_t sda, uint8_t scl);
    void      loop();
    void      OnForTimer(String SubTopic, int duration);
    void      SetOn(String SubTopic);
    void      SetOn(uint8_t Port);
    void      SetOff(String SubTopic);
    void      SetOff(uint8_t Port);
    bool      GetState(uint8_t Port);
    bool      GetEnabled(uint8_t Port);
    void      SetEnable(uint8_t Port, bool state);
    uint8_t   CountActiveThreads();
    
    void      StoreJsonConfig(String* json);
    void      LoadJsonConfig();
    void      GetWebContent(String* html);
    void      getWebJsParameter(String* html);
    void      ReceiveMQTT(const char* topic, const char* value);
  
  private:
    void      addValve(uint8_t Port, String SubTopic);
    valve*    GetValveItem(uint8_t Port);
      
    valveHardware* ValveHW = NULL;
    std::vector<valve> *Valves = NULL;
    
    uint8_t pin_sda = SDA;
    uint8_t pin_scl = SCL;
    uint8_t parallelThreads = 0;
};

#endif
