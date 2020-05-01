#ifndef TB6612_H
#define TB6612_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "WEMOS_Motor.h"

class tb6612 {
  
  public:
    tb6612();
    void init(uint8_t address);
    void setOn(uint8_t port, bool dir); // Port: A=0; B=1 ;  Direction: true=forward; false=backward
    void setOff(uint8_t port);
    
  private:
    Motor* M1; //Motor A
    Motor* M2; //Motor B
};

#endif
