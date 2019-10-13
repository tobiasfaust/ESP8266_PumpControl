#include "valve.h"

valve::valve() : enabled(true), active(false){

}

void valve::init(String SubTopic) {
  ValveType = VIRTUAL;
  subtopic = SubTopic;  
}

void valve::init(valveHardware* vHW, uint8_t Port, String SubTopic) {
  valveHWClass = vHW;
  myHWdev = valveHWClass->RegisterPort(Port);
  ValveType = NORMAL;
  port1 = Port;
  subtopic = SubTopic;  
}

void valve::init(valveHardware* vHW, uint8_t Port1, uint8_t Port2, uint16_t P1ms, uint16_t P2ms, String SubTopic) {
  valveHWClass = vHW;
  myHWdev = valveHWClass->RegisterPort(Port1);
  if (Port2>0) { myHWdev = valveHWClass->RegisterPort(Port2); }
  ValveType = BISTABIL;
  port1 = Port1;
  if (Port2>0) { port2 = Port2; }
  port1ms = P1ms; 
  port2ms = P2ms;
  subtopic = SubTopic;  
}

void valve::OnForTimer(int duration) {
  HandleSwitch(true, duration);
}

void valve::SetOn() {
  HandleSwitch(true, 3600); //Sicherheitsabschaltung nach 1h
}

void valve::SetOff() {
  HandleSwitch(false);
}

void valve::HandleSwitch (bool state) {
  HandleSwitch(state, 0);
}

void valve::HandleSwitch (bool state, int duration) {
  startmillis = millis();
  lengthmillis = duration * 1000;
  if (ValveType == NORMAL) {
    valveHWClass->SetPort(myHWdev, port1, state);
  } else if (ValveType == BISTABIL) {
    valveHWClass->SetPort(myHWdev, port1, port2, state, (state?port1ms:port2ms));
  }
}
    
void valve::loop() {
  if (enabled && startmillis != 0 && (millis() - startmillis > lengthmillis)) {
    //Serial.print("on-for-timer abgelaufen: Pin");Serial.println(i);
    HandleSwitch(false);
  }
}

