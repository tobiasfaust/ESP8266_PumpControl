#pragma once

/*#include <Wire.h>
#include "PCF8574.h"
#include "Grove_Motor_Driver_TB6612FNG.h"
#include <ArduinoJson.h>
#include <i2cdetect.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include "uptime.h"
#include <i2cdetect.h>
*/

#include <vector>
#include "BaseConfig.h"
#include "valveStructure.h"
#include "MQTT.h"
#include "WebServer.h"
#include "sensor.h"
#include "oled.h"

i2cdetect* I2Cdetect = NULL;
BaseConfig* Config = NULL;
valveRelation* ValveRel = NULL;  
valveStructure* VStruct = NULL;
MQTT* mqtt = NULL;
sensor* LevelSensor = NULL;
OLED* oled = NULL;
WebServer* webserver = NULL;

//test
//valveRelation* ValveRel = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ready");

// Flash Write Issue
// https://github.com/esp8266/Arduino/issues/4061#issuecomment-428007580

  //clean FS, for testing
  //SPIFFS.format();
  
  Config = new BaseConfig();
  
  mqtt = new MQTT(Config->GetMqttServer().c_str(), Config->GetMqttPort(), Config->GetMqttRoot().c_str());
  mqtt->setCallback(myMQTTCallBack);

  I2Cdetect = new i2cdetect(Config->GetPinSDA(), Config->GetPinSDA());
  LevelSensor = new sensor();
  oled = new OLED(Config->GetPinSDA(), Config->GetPinSDA());
  ValveRel = new valveRelation();
  VStruct = new valveStructure(Config->GetPinSDA(), Config->GetPinSDA());
  webserver = new WebServer(); 
 
  //VStruct->OnForTimer("Valve1", 10);

  delay(1000);
  //VStruct->ReceiveMQTT("pumpHost/TestVirtualValve1/on-for-timer", "10");
  //VStruct->ReceiveMQTT("testhost/TestVirtualValve1/on-for-timer", "10");
  
}

void myMQTTCallBack(char* topic, byte* payload, unsigned int length) {
  String msg;
  Serial.print("Message arrived [");Serial.print(topic);Serial.print("] ");
  
  for (int i = 0; i < length; i++) { msg.concat((char)payload[i]); }
  Serial.print("Message: ");Serial.println(msg.c_str());
  
  VStruct->ReceiveMQTT(topic, msg.c_str());
}

void loop() {
  // put your main code here, to run repeatedly:
  VStruct->loop();
  mqtt->loop();
  oled->loop();
  webserver->loop();
}
