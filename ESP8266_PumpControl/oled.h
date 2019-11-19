#ifndef OLED_H
#define OLED_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "SSD1306Wire.h"

#include "sensor.h"
#include "MQTT.h"

extern sensor* LevelSensor;
extern MQTT* mqtt;

class OLED {

  public:
    OLED(uint8_t sda, uint8_t scl, uint8_t i2cAddress);
    
    void      loop();
    
  private:
    uint8_t   pin_sda;
    uint8_t   pin_scl;
    uint8_t   i2cAddress;
    SSD1306Wire* ssd;

    void      display_header();
    void      display_title(String& title, String& subtitle);
    void      display_wifibars();
};

#endif

