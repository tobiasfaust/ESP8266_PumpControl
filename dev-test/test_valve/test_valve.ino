#pragma once

#include <Wire.h>
#include "PCF8574.h"
#include "Grove_Motor_Driver_TB6612FNG.h"
  
#include <vector>
#include "valve.h"

valveHardware* ValveHW = NULL;
std::vector<valve> *Valves = NULL;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ready");
  
  // put your setup code here, to run once:
  ValveHW = new valveHardware(0,4); //SDA, SCL
  Valves  = new std::vector<valve>{};
  
  for (uint8_t i=66;i<71; i++) {
    valve myValve;
    myValve.init(ValveHW, i, "abc");
    Valves->push_back(myValve);
  }

  valve myValve;
  myValve.init(ValveHW, 202, "NormalTest");
  Valves->push_back(myValve);
  
  myValve.init(ValveHW, 10, 0, 250, 50, "bistableTest");
  Valves->push_back(myValve);

  delay(2000);
  Valves->at(0).OnForTimer(10);
  Valves->at(5).OnForTimer(8);
  Valves->at(6).OnForTimer(5);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (uint8_t i=0; i<Valves->size(); i++) {
    Valves->at(i).loop();
  }
}
