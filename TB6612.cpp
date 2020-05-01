#include "TB6612.h"

tb6612::tb6612() {  
}

void tb6612::init(uint8_t address) {
  M1 = new Motor(address,_MOTOR_A, 1000);
  M2 = new Motor(address,_MOTOR_B, 1000);
  Serial.println("TB6612 initialize");
}

void tb6612::setOff(uint8_t port) {
  if (port==0) {
    M1->setmotor(_STOP);
  } else if (port==1) {
    M2->setmotor(_STOP);
  }
  Serial.println("Motor Stop");
}

void tb6612::setOn(uint8_t port, bool dir) {
  if (port==0) {
    M1->setmotor( (dir?_CW:_CCW));
  } else if (port==1) {
    M2->setmotor( (dir?_CW:_CCW));
  }
  Serial.println("Motor On");
}

