#ifndef OLED_H
#define OLED_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <Wire.h>
#include "SSD1306Wire.h"

class OLED {

  public:
    OLED();
    void      init(uint8_t sda, uint8_t scl, uint8_t i2cAddress);
    
    void      SetLevel(uint8_t Level);
    void      SetDeviceName(String s);
    void      Enable(bool e);
    void      SetIP(String ip);
    void      SetSSID(String ssid);
    void      SetRSSI(int rssi);
    void      SetWiFiConnected(bool c);
    void      SetMqttConnected(bool c);
    
    const bool& GetEnabled() const {return enabled;}
    
  private:
    uint8_t   SensorLevel;
    String    DeviceName;
    bool      enabled;
    String    ip = "";
    String    ssid = "";
    int       rssi;
    bool      WiFiConnected;
    bool      MqttConnected;
    
    uint8_t   pin_sda;
    uint8_t   pin_scl;
    uint8_t   i2cAddress;
    SSD1306Wire* ssd;

    void      display_header();
    void      display_wifibars();
    void      display_MqttConnectInfo();
    void      SetLevel(uint8_t Level, bool force);
    void      UpdateAll();
        
};

#endif

