#ifndef COMMONLIBS_H
#define COMMONLIBS_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

//#if defined(ESP8266) || defined(ESP32)
//  #define min(x,y) _min(x,y)
//#endif

#ifdef ESP8266
  extern "C" {
      #include "user_interface.h"
  }

  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif ESP32
  #include <WiFi.h> 
  #include <AsyncTCP.h>
#endif

#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

#if defined(USE_OLED) || defined(USE_PCF8574) || defined(USE_TB6612)
  #define USE_I2C
#endif

#ifdef ESP8266
  #define ESP_getChipId() ESP.getChipId() 
  #define ESP_GetMaxFreeAvailableBlock() ESP.getMaxFreeBlockSize()
#elif ESP32
  #define ESP_getChipId() (uint32_t)ESP.getEfuseMac()   // Unterschied zu ESP.getFlashChipId() ???
  #define ESP_GetMaxFreeAvailableBlock() ESP.getMaxAllocHeap()
#endif


#endif