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
//#include "ValveStructure.h"

#if defined(ESP8266) || defined(ESP32)
  #include <functional>
  #define CALLBACK_FUNCTION std::function<void(char*, uint8_t*, unsigned int)> MyCallback
#else
  #define CALLBACK_FUNCTION void (*MyCallback)(char*, uint8_t*, unsigned int)
#endif

  
class MQTT {

  public:
    MQTT(const char* server, uint16_t port, String root);
    void    loop();
    void    Publish(const char* subtopic, bool b);
    void    Publish(const char* subtopic, int* number);
    void    Publish(const char* subtopic, char* value);
    void    setCallback(CALLBACK_FUNCTION);
    String  GetRoot();

  private:
    WiFiClient espClient;
    PubSubClient mqtt;
    CALLBACK_FUNCTION;
    void    reconnect();
    void    callback(char* topic, byte* payload, unsigned int length);

    String  mqtt_root = "";
    unsigned long mqttreconnect_lasttry = 0;
  
};

#endif

