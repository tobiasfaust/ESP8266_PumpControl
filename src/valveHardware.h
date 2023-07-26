#ifndef VALVEHARDWARE_H
#define VALVEHARDWARE_H
  
#include "CommonLibs.h"
#include "BaseConfig.h"
#include <vector>
#include <Wire.h>
#include "PCF8574.h"     // https://github.com/xreef/PCF8574_library
#include "TB6612.h"
#include "OW2408.h"

extern BaseConfig* Config;

enum HWType_t {ONBOARD, PCF, TB6612, OW2408};

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
    
    bool    RegisterPort(HWdev_t*& dev, uint8_t Port);
    bool    RegisterPort(HWdev_t*& dev, uint8_t Port, bool reverse);

    void      add1WireDevice(uint8_t pin_1wire);
    void      SetPort(HWdev_t* dev, uint8_t Port, bool state, bool reverse);
    void      SetPort(HWdev_t* dev, uint8_t Port1, uint8_t Port2, bool state, bool reverse, uint16_t duration);
    bool      IsValidPort(uint8_t Port);
    uint8_t  GetI2CAddress(uint8_t Port);
    
    void      GetWebContent1Wire(AsyncResponseStream *response);
    
    bool      Get1WireActive(); // ist 1wire initialisiert?
    uint8_t  Get1WireCountDevices();
    uint8_t  Refresh1WireDevices();
    
    const uint8_t& GetPin1wire()      const {return pin_1wire;}
    
  private:
    
    // https://www.learncpp.com/cpp-tutorial/6-16-an-introduction-to-stdvector/
    // https://www.learncpp.com/cpp-tutorial/7-10-stdvector-capacity-and-stack-behavior/
    // https://de.wikibooks.org/wiki/C%2B%2B-Programmierung:_Vector
    std::vector<HWdev_t> *HWDevice;
    
    uint8_t pin_sda = SDA;
    uint8_t pin_scl = SCL;
    uint8_t pin_1wire = 0;
    //bool     1wireInitDone;
    
    void    setHWType(HWdev_t* dev);
    void    ConnectHWdevice(HWdev_t* dev);
    void    PortMapping(PortMap_t* Map);
    void    addI2CDevice(uint8_t i2cAddress);
    bool    I2CIsPresent(uint8_t i2cAddress);

    HWdev_t* getI2CDevice(uint8_t i2cAddress);
    
    
};

#endif
