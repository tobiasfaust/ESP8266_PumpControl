#include "valve.h"

valve::valve() : enabled(true), active(false), ValveType(NONE){
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

void valve::AddPort1(valveHardware* Device, uint8_t Port1) {
  myHWdev = valveHWClass->RegisterPort(Port1);
  port1 = Port1;  
}

void valve::AddPort2(valveHardware* Device, uint8_t Port2) {
  myHWdev = valveHWClass->RegisterPort(Port2);
  port2 = Port2;  
}

/*
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
*/

void valve::OnForTimer(int duration) {
  if (enabled) {HandleSwitch(true, duration);}
}

void valve::SetOn() {
  if (enabled) {HandleSwitch(true, NULL);}
}

void valve::SetOff() {
  HandleSwitch(false, NULL);
}

void valve::HandleSwitch (bool state, int duration) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  sprintf(buffer, "Starte Change Status Ventil Port %d : %s -> %s vom Type %d", port1, vState(active), vState(state), ValveType);
  Serial.println(buffer);
  
  if (ValveType == NORMAL) {
    valveHWClass->SetPort(myHWdev, port1, state);
  } else if (ValveType == BISTABIL) {
    valveHWClass->SetPort(myHWdev, port1, port2, state, (state?port1ms:port2ms));
  }

  active = state;
  
  if (state && duration) {
    startmillis = millis();
    lengthmillis = duration * 1000;
  } else {
    startmillis = lengthmillis = 0;
  }
}

int valve::ActiveTimeLeft() {
  return _max((lengthmillis - (millis() - startmillis)),0);
}

void valve::SetValveType(String type) {
  if (type == "n") { ValveType = NORMAL; }
  else if (type=="b") { ValveType = BISTABIL; }
  else if (type=="v") { ValveType = VIRTUAL; }
  else { ValveType = NONE; }
}

String valve::GetValveType() {
  if (ValveType == NORMAL) { return "n"; }
  else if (ValveType == BISTABIL) { return "b"; }
  else if (ValveType == VIRTUAL) { return "v"; }
  else { return ""; }
}

uint8_t valve::GetPort1() {
  return port1;
}

uint8_t valve::GetPort2() {
  return port2;
}

void valve::loop() {
  if (ActiveTimeLeft()==0) {
    //Serial.print("on-for-timer abgelaufen: Pin");Serial.println(i);
    SetOff();
  }
}

