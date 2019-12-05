#ifndef VALVERELATION_H
#define VALVERELATION_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <FS.h> 
#include <vector>
#include <ArduinoJson.h>
#include "MQTT.h"

extern MQTT* mqtt;

class valveRelation {

  typedef struct {
    bool enabled;
    String TriggerSubTopic = "";
    uint8_t ActorPort; 
  } relation_t;
  
  public:
    valveRelation();
    void      AddRelation(bool enabled, String SubTopic, uint8_t Port);
    void      GetPortDependencies(std::vector<uint8_t>* Ports, String SubTopic);
    void      AddSubscriberPort(uint8_t Port, String TriggerSubTopic);
    void      DelSubscriberPort(uint8_t Port);
    uint8_t   CountActiveSubscribers(String SubTopic);
    
    void      StoreJsonConfig(String* json); 
    void      LoadJsonConfig();
    void      GetWebContent(ESP8266WebServer* server);
    
  private:
    std::vector<relation_t>* _relationen  = NULL;
    std::vector<relation_t>* _subscriber  = NULL;
};

#endif
