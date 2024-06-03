#ifndef VALVERELATION_H
#define VALVERELATION_H

#include "CommonLibs.h"
#include <vector>
#include <ArduinoJson.h>
#include "MyMqtt.h"

class valveRelation {

  typedef struct {
    bool enabled;
    String TriggerTopic = "";
    uint8_t ActorPort; 
    bool EnableByBypass;
  } relation_t;
  
  typedef struct {
    String TriggerTopic = "";
    uint8_t ActorPort; 
  } subscriber_t;
  
  public:
    valveRelation();
    void      AddRelation(bool enabled, String TriggerTopic, uint8_t Port, bool EnableByBypass);
    void      GetPortDependencies(std::vector<uint8_t>* Ports, String TriggerTopic);
    bool      CheckEnabledByBypass(uint8_t ActorPort, String TriggerTopic);
    void      AddSubscriber(uint8_t Port, String TriggerTopic);
    void      DelSubscriber(String TriggerTopic);
    uint8_t   CountActiveSubscribers(uint8_t ActorPort);
    
    void      GetInitData(AsyncResponseStream* response);
    void      LoadJsonConfig();
    
  private:
    std::vector<relation_t>* _relationen  = NULL;
    std::vector<subscriber_t>* _subscriber  = NULL;
};

#endif
