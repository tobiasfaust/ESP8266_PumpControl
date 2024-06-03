
#ifndef VALVESTRUCTURE_H
#define VALVESTRUCTURE_H

#include "CommonLibs.h"
#include <ArduinoJson.h>
#include "BaseConfig.h"
#include "valveRelation.h"
#include "valve.h"
#include "MyMqtt.h"

extern BaseConfig* Config;
extern valveRelation* ValveRel;

#ifdef USE_I2C
  #include <i2cdetect.h>
  extern i2cdetect* I2Cdetect;
#endif

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
    
    void      GetInitData(AsyncResponseStream* response);
    void      GetInitData1Wire(AsyncResponseStream* response);

    void      LoadJsonConfig();
    void      getWebJsParameter(AsyncResponseStream *response);
    void      ReceiveMQTT(String topic, int value);
    uint8_t   Get1WireCountDevices();
    uint8_t   Refresh1WireDevices();
    
  private:
    //void      addValve(uint8_t Port, String SubTopic);
    valve*    GetValveItem(uint8_t Port);
    valve*    GetValveItem(String SubTopic);
    void      handleDeps(String topic, int value); //prueft die Relationen
    String    GetJsonKeyMatch(JsonDocument* doc, String key);

    valveHardware* ValveHW = NULL;
    std::vector<valve>* Valves = NULL;
    
    uint8_t pin_sda = SDA;
    uint8_t pin_scl = SCL;
    uint8_t parallelThreads = 0;
};

#endif
