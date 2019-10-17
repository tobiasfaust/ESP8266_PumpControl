#pragma once

//#include <FS.h> 
#include <Wire.h>
#include "PCF8574.h"
#include "Grove_Motor_Driver_TB6612FNG.h"
#include <ArduinoJson.h>
#include <i2cdetect.h>
  
#include <vector>
#include "valveStructure.h"


valveStructure* VStruct = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ready");
  
  // put your setup code here, to run once:
  VStruct = new valveStructure(0,4);

  VStruct->OnForTimer("Valve1", 10);
}

void loop() {
  // put your main code here, to run repeatedly:
  VStruct->loop();
}
