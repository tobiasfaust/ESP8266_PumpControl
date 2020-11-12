#include "valve.h"

valve::valve() : enabled(true), active(false), ValveType(NONE), autooff(0), reverse(false) {
}

void valve::init(valveHardware* vHW, uint8_t Port, String SubTopic) {
  this->valveHWClass = vHW;
  this->myHWdev = valveHWClass->RegisterPort(Port);
  this->ValveType = NORMAL;
  this->port1 = Port;
  this->subtopic = SubTopic;
}

void valve::AddPort1(valveHardware* Device, uint8_t Port1) {
  this->valveHWClass = Device;
  this->myHWdev = Device->RegisterPort(Port1);
  this->port1 = Port1;  
}

void valve::AddPort2(valveHardware* Device, uint8_t Port2) {
  this->myHWdev = Device->RegisterPort(Port2);
  this->port2 = Port2;
}

bool valve::OnForTimer(int duration) {
  bool ret;
  if (enabled && ActiveTimeLeft() < duration) {ret = this->HandleSwitch(true, duration);}
  if (duration == 0) { SetOff(); }
  return ret;
}

bool valve::SetOn() {
  bool ret = false;
  if (this->enabled && !this->active) {
    if (this->autooff > 0) {
      ret = this->HandleSwitch(true, this->autooff);
    } else {
      ret = this->HandleSwitch(true, NULL);
    }
  }
  return ret;
}

bool valve::SetOff() {
  bool ret = false;
  if (this->active) {ret = this->HandleSwitch(false, NULL);}
  return ret;
}

bool valve::HandleSwitch (bool state, int duration) {
  char buffer[50] = {0};
  memset(buffer, 0, sizeof(buffer));
  if (ValveType == NORMAL) {
    Serial.printf("Schalte Standard Ventil %s: Port %d (0x%02X) \n", (state?"An":"Aus"), this->port1, myHWdev->i2cAddress);
    valveHWClass->SetPort(myHWdev, this->port1, state, this->reverse);
  } else if (ValveType == BISTABIL) {
    Serial.printf("Schalte Bistabiles Ventil %s: Port %d/%d, ms: %d/%d (0x%02X) \n", (state?"An":"Aus"), port1, port2, port1ms, port2ms, myHWdev->i2cAddress);
    valveHWClass->SetPort(myHWdev, this->port1, this->port2, state, this->reverse, (state?this->port1ms:this->port2ms));
  }

  this->active = state;
  
  if (state && duration) {
    startmillis = millis();
    lengthmillis = duration * 1000;
  } else {
    startmillis = lengthmillis = 0;
  }

  if(mqtt) {
    snprintf (buffer, sizeof(buffer), "%s/state", this->subtopic.c_str());
    mqtt->Publish_Bool(buffer, state);
  }

  return true;
}

int valve::ActiveTimeLeft() {
  return _max((lengthmillis - (millis() - startmillis)),0);
}

void valve::SetValveType(String type) {
  if (type == "n") { ValveType = NORMAL; }
  else if (type=="b") { ValveType = BISTABIL; }
  else { ValveType = NONE; }
}

String valve::GetValveType() {
  if (ValveType == NORMAL) { return "n"; }
  else if (ValveType == BISTABIL) { return "b"; }
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
