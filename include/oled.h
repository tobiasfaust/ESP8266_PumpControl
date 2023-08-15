#ifndef OLED_H
#define OLED_H

#include "CommonLibs.h"
#include <Wire.h>
#include "BaseConfig.h"
#include "SSD1306Wire.h"
#include "SH1106.h"

extern BaseConfig* Config;

class OLEDWrapper {

  public:
    OLEDWrapper(uint8_t sda, uint8_t scl, uint8_t i2cAddress);
    void flipScreenVertically();
    bool init();
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height);
    void setColor(OLEDDISPLAY_COLOR color);
    void setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment);
    void setFont(const uint8_t *fontData);
    void drawString(int16_t x, int16_t y, String text);
    void drawHorizontalLine(int16_t x, int16_t y, int16_t length);
    uint16_t getStringWidth(String text);
    void clear(void);
    void display(void);
  
  private:
    void* oled;
};


class OLED {
   
  public:
    OLED();
    void      loop();
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
    uint8_t   type;
    
    //SSD1306Wire* ssd;
    //SH1106* ssd;
    OLEDWrapper* ssd;
    
    void      display_header();
    void      display_wifibars();
    void      display_MqttConnectInfo();
    void      SetLevel(uint8_t Level, bool force);
    void      UpdateAll();
        
};

#endif
