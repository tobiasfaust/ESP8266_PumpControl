#ifndef VALVEHARDWARE_H
#define VALVEHARDWARE_H
  
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <vector>
#include <Wire.h>
#include "PCF8574.h"     // https://github.com/xreef/PCF8574_library
#include "TB6612.h"

enum HWType_t {GPIO, PCF, TB6612};

typedef struct {
    void* Device;
    HWType_t HWType;
    uint8_t i2cAddress;
  } HWdev_t;

#define vState(x) ((x)?"An":"Aus") // Boolean in lesbare Ausgabe

class valveHardware {
  
  // PortMapping Type
  typedef struct {
    uint8_t i2cAddress;
    HWType_t HWType;
    uint8_t Port;
    uint8_t internalPort;
  } PortMap_t;

  public:
    valveHardware(uint8_t sda, uint8_t scl);
    
    HWdev_t* RegisterPort(uint8_t Port);
    void    SetPort(HWdev_t* dev, uint8_t Port, bool state);
    void    SetPort(HWdev_t* dev, uint8_t Port1, uint8_t Port2, bool state, uint16_t duration);
    bool    IsValidPort(uint8_t Port);
    uint8_t GetI2CAddress(uint8_t Port);
    
  private:
    
    // https://www.learncpp.com/cpp-tutorial/6-16-an-introduction-to-stdvector/
    // https://www.learncpp.com/cpp-tutorial/7-10-stdvector-capacity-and-stack-behavior/
    // https://de.wikibooks.org/wiki/C%2B%2B-Programmierung:_Vector
    std::vector<HWdev_t> *HWDevice;
    
    uint8_t pin_sda = SDA;
    uint8_t pin_scl = SCL;
  
    void    setHWType(HWdev_t* dev);
    void    ConnectHWdevice(HWdev_t* dev);
    void    PortMapping(PortMap_t* Map);
    void    addI2CDevice(uint8_t i2cAddress);
    bool    I2CIsPresent(uint8_t i2cAddress);

    HWdev_t* getI2CDevice(uint8_t i2cAddress);
    
    
};

#endif

