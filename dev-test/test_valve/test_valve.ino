#pragma once

//#include <FS.h> 
#include <Wire.h>
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


#include <vector>
#include "valveStructure.h"
#include "MQTT.h"

  
valveStructure* VStruct = NULL;
MQTT* mqtt = NULL;

//test
valveRelation* ValveRel = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ready");
  
  // put your setup code here, to run once:
  mqtt = new MQTT("192.168.10.10", 1883, "PumpControlDev");
  mqtt->setCallback(myMQTTCallBack);

  VStruct = new valveStructure(0,4);

  VStruct->SetMQTTClass(mqtt);
  VStruct->OnForTimer("Valve1", 10);

  delay(1000);
  VStruct->ReceiveMQTT("pumpHost/TestVirtualValve1/on-for-timer", "10");
  VStruct->ReceiveMQTT("testhost/TestVirtualValve1/on-for-timer", "10");
}

void myMQTTCallBack(char* topic, byte* payload, unsigned int length) {
  String* msg;
  Serial.print("Message arrived [");Serial.print(topic);Serial.print("] ");
  
  for (int i = 0; i < length; i++) { msg->concat((char)payload[i]); }
  Serial.print("Message: ");Serial.println(msg->c_str());
  
  VStruct->ReceiveMQTT(topic, msg->c_str());
}

void loop() {
  // put your main code here, to run repeatedly:
  VStruct->loop();
  mqtt->loop();
}
