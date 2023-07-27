#ifndef COMMONLIBS_H
#define COMMONLIBS_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#pragma once

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

#endif