#include <vector>
#include "valve.h"

valveHardware* ValveHW = NULL;
std::vector<valve*> Valves;

void setup() {
  // put your setup code here, to run once:
  ValveHW = new valveHardware(0,4); //SDA, SCL
  
  for (uint8_t i=200;i<211; i++) {
    valve* myValve = NULL;
    myValve->init(ValveHW, i, "abc");
    Valves.push_back(myValve);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  for (const auto &element : Valves) {
    element->loop();
  }
}
