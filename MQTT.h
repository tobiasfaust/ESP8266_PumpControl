#ifndef MQTT_H
#define MQTT_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <vector>
#include "oled.h"
#include "BaseConfig.h"

extern OLED* oled;
extern BaseConfig* Config;

#if defined(ESP8266) || defined(ESP32)
  #include <functional>
  #define CALLBACK_FUNCTION std::function<void(char*, uint8_t*, unsigned int)> MyCallback
#else
  #define CALLBACK_FUNCTION void (*MyCallback)(char*, uint8_t*, unsigned int)
#endif

class MQTT {

  public:

    enum MqttSubscriptionType_t {RELATION, SENSOR};
    
    typedef struct {
      String subscription = "";
      MqttSubscriptionType_t identifier;
      bool active;
    } subscription_t;
    
    MQTT(const char* server, uint16_t port, String root);
    void    loop();
    void    Publish_Bool(const char* subtopic, bool b);
    void    Publish_Int(const char* subtopic, int* number);
    void    Publish_String(const char* subtopic, char* value);
    void    Publish_IP();
    void    setCallback(CALLBACK_FUNCTION);
    String  GetRoot();
    void    Subscribe(String topic, MqttSubscriptionType_t identifier);
    void    ClearSubscriptions(MqttSubscriptionType_t identifier);

  private:
    WiFiClient espClient;
    PubSubClient* mqtt;
    CALLBACK_FUNCTION;
    void    reconnect();
    void    callback(char* topic, byte* payload, unsigned int length);
    std::vector<subscription_t>* subscriptions = NULL;

    String  mqtt_root = "";
    unsigned long mqttreconnect_lasttry = 0;
  
};

#endif

