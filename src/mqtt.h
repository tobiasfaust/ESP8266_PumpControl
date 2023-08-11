#ifndef MQTT_H
#define MQTT_H

#include "CommonLibs.h" 
#include <PubSubClient.h>
#include <ESPAsyncWiFiManager.h>    // https://github.com/alanswx/ESPAsyncWiFiManager
#include <vector>
#include "oled.h"
#include "BaseConfig.h"

#ifdef ESP8266
  //#define SetHostName(x) wifi_station_set_hostname(x);
  #define WIFI_getChipId() ESP.getChipId() 
#elif ESP32
  //#define SetHostName(x) WiFi.getHostname(x); --> MQTT.cpp TODO
  #define WIFI_getChipId() (uint32_t)ESP.getEfuseMac()   // Unterschied zu ESP.getFlashChipId() ???
#endif

#if defined(ESP8266) || defined(ESP32)
  #include <functional>
  #define CALLBACK_FUNCTION std::function<void(char*, uint8_t*, unsigned int)> MyCallback
#else
  #define CALLBACK_FUNCTION void (*MyCallback)(char*, uint8_t*, unsigned int)
#endif

class MQTT {

  public:

    MQTT(AsyncWebServer* server, DNSServer *dns, const char* MqttServer, uint16_t MqttPort, String MqttBasepath, String MqttRoot, char* APName, char* APpassword);
    void              loop();
    void              Publish_Bool(const char* subtopic, bool b, bool fulltopic);
    void              Publish_Int(const char* subtopic, int number, bool fulltopic);
    void              Publish_Float(const char* subtopic, float number, bool fulltopic);
    void              Publish_String(const char* subtopic, String value, bool fulltopic);
    void              Publish_IP();
    String            getTopic(String subtopic, bool fulltopic);
    void              setCallback(CALLBACK_FUNCTION);
    void              disconnect();
    const String&     GetRoot()  const {return mqtt_root;};
    void              Subscribe(String topic);
    void              ClearSubscriptions();
    
    const bool&       GetConnectStatusWifi()      const {return ConnectStatusWifi;}
    const bool&       GetConnectStatusMqtt()      const {return ConnectStatusMqtt;}

  protected:
    PubSubClient*     mqtt;
    void              reconnect();

  private:
    AsyncWebServer*   server;
    DNSServer*        dns;
    WiFiClient        espClient;
    AsyncWiFiManager* wifiManager;

    CALLBACK_FUNCTION;
    void              callback(char* topic, byte* payload, unsigned int length);
    
    std::vector<String>* subscriptions = NULL;

    String            mqtt_root = "";
    String            mqtt_basepath = "";
    unsigned long     mqttreconnect_lasttry = 0;
    unsigned long     last_keepalive = 0;
    bool              ConnectStatusWifi;
    bool              ConnectStatusMqtt;
  
};

class MyMQTT: public virtual MQTT {
  
  public:
    enum MqttSubscriptionType_t {RELATION, SENSOR};
    
    typedef struct {
      String subscription = "";
      MqttSubscriptionType_t identifier;
      bool active;
    } subscription_t;

    MyMQTT(AsyncWebServer* server, DNSServer *dns, const char* MqttServer, uint16_t MqttPort, String MqttBasepath, String MqttRoot, char* APName, char* APpassword);
  
    void    SetOled(OLED* oled);
    void    loop();
    void    reconnect();
    void    Subscribe(String topic, MqttSubscriptionType_t identifier);
    void    ClearSubscriptions(MqttSubscriptionType_t identifier);

  private:
    OLED* oled;
    std::vector<subscription_t>* subscriptions = NULL;

};

extern MyMQTT* mqtt;

#endif
