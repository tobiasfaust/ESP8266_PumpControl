#ifndef MQTT_H
#define MQTT_H

#include "CommonLibs.h" 
#include <PubSubClient.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <vector>
#include "oled.h"
#include "BaseConfig.h"

#ifdef ESP8266
  //#define SetHostName(x) wifi_station_set_hostname(x);
  #define WIFI_getChipId() ESP.getChipId() 
#elif ESP32
  //#define SetHostName(x) WiFi.getHostname(x); --> MQTT.cpp
  #define WIFI_getChipId() (uint32_t)ESP.getEfuseMac()   // Unterschied zu ESP.getFlashChipId() ???
  
#endif


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
    void    Publish_Int(const char* subtopic, int number);
    void    Publish_String(const char* subtopic, String value);
    void    Publish_IP();
    void    setCallback(CALLBACK_FUNCTION);
    void    disconnect();
    String  GetRoot();
    void    Subscribe(String topic, MqttSubscriptionType_t identifier);
    void    ClearSubscriptions(MqttSubscriptionType_t identifier);

    const bool& GetConnectStatusWifi()      const {return ConnectStatusWifi;}
    const bool& GetConnectStatusMqtt()      const {return ConnectStatusMqtt;}

  private:
    WiFiClient espClient;
    PubSubClient* mqtt;
    CALLBACK_FUNCTION;
    void    reconnect();
    void    callback(char* topic, byte* payload, unsigned int length);
    std::vector<subscription_t>* subscriptions = NULL;

    String  mqtt_root = "";
    unsigned long mqttreconnect_lasttry = 0;
    unsigned long last_keepalive = 0;
    bool     ConnectStatusWifi;
    bool     ConnectStatusMqtt;
  
};

#endif
