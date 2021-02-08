#ifndef OW2408_H
#define OW2408_H

#include "CommonLibs.h"
#include "DS2408.h"     // https://github.com/queezythegreat/arduino-ds2408
                                     // https://github.com/PaulStoffregen/OneWire

class ow2408 {
  
  public:
    ow2408();
    void        init(uint8_t pin);
    bool        setOn(uint8_t port); 
    bool        setOff(uint8_t port);
    bool        setPort(uint8_t port, bool state);
    void        setDebugMode(uint8_t debugmode);
    bool        isValidPort(uint8_t port);
    uint8_t    findDevices();
    const uint8_t& GetCountDevices() const {return device_count;}
    
    void        GetWebContent1Wire(WM_WebServer* server);
  private:
    DS2408* ow;
    Devices devices;
    uint8_t device_count;
    uint8_t debugmode;

    void setup_devices();
    String print_device(uint8_t index);
    void print_byte(uint8_t data); // nur test
    bool handlePort(uint8_t port, bool state);
    
};

#endif
