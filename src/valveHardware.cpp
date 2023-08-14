#ifndef valve_h
  #define valve_h
  #include "valveHardware.h"
#endif

// Constructor
valveHardware::valveHardware(uint8_t sda, uint8_t scl)
  : pin_sda(sda), pin_scl(scl) {

  HWDevice = new std::vector<HWdev_t>{};
  
  // initial immer das GPIO HardwareDevice erstellen
  HWdev_t t; 
  t.HWType=ONBOARD;
  t.i2cAddress=0x00;
  HWDevice->push_back(t);

  if (Config->GetDebugLevel() >=3)  { 
    char buffer[100] = {0};
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Initialisiere HardwareDevice mit GPIO auf ic2Adresse 0x%02X", t.i2cAddress);
    Serial.println(buffer);
  }
}


void valveHardware::add1WireDevice(uint8_t pin_1wire) {
  if (this->Get1WireActive() && this->pin_1wire != pin_1wire) {
    HWdev_t* t = this->getI2CDevice(0x01);
    ow2408* MyDS2408 = static_cast<ow2408*>(t->Device);
    MyDS2408->init(pin_1wire);
    this->pin_1wire = pin_1wire;
    if (Config->GetDebugLevel() >=3) { Serial.printf("1Wire Pin changed successfully, %d devices found\n", MyDS2408->GetCountDevices()); }
  } 
  else if (!this->Get1WireActive()) {    
    ow2408* MyDS2408 = new ow2408();
    MyDS2408->init(pin_1wire); 
    this->pin_1wire = pin_1wire;
    
    HWdev_t t; 
    t.Device = MyDS2408;
    t.HWType=OW2408;
    t.i2cAddress=0x01;
    HWDevice->push_back(t);

    if (Config->GetDebugLevel() >=3)  { Serial.printf("1Wire added successfully, %d devices found\n", MyDS2408->GetCountDevices()); }
  } else {
    if (Config->GetDebugLevel() >=5)  { Serial.println("1wire already present"); }
  }
}

bool valveHardware::Get1WireActive() {
  for (uint8_t i=0; i<HWDevice->size(); i++) {
    if (HWDevice->at(i).i2cAddress == 0x01) {
      return true;
    }
  }
  return false;
}

uint8_t valveHardware::Get1WireCountDevices() {
  if (this->Get1WireActive()) {
    HWdev_t* t = this->getI2CDevice(0x01);
    ow2408* MyDS2408 = static_cast<ow2408*>(t->Device);
    return MyDS2408->GetCountDevices();
  }
  else  return 0;
}

uint8_t valveHardware::Refresh1WireDevices() {
    if (this->Get1WireActive()) {
    HWdev_t* t = this->getI2CDevice(0x01);
    ow2408* MyDS2408 = static_cast<ow2408*>(t->Device);
    return MyDS2408->findDevices();
  }
  else  return 0;
}

void valveHardware::addI2CDevice(uint8_t i2cAddress) {
  if (!I2CIsPresent(i2cAddress)) {
    if (i2cAddress == 0x01) {
      if (Config->GetDebugLevel() >=1)  { Serial.println("cannot add 1wire simply, call 'add1WireDevice(pin)' instead"); }
    } else {
      HWdev_t t; 
      t.i2cAddress = i2cAddress;
      setHWType(&t);
      ConnectHWdevice(&t);
      HWDevice->push_back(t);
    }
  }
}

bool valveHardware::I2CIsPresent(uint8_t i2cAddress) {
  char buffer[100] = {0};
  //for (const auto &element : HWDevice) {
  for (uint8_t i=0; i<HWDevice->size(); i++) {
    if (Config->GetDebugLevel() >=5)  { 
      memset(buffer, 0, sizeof(buffer));
      sprintf(buffer, "Pruefe ic2Adresse 0x%02X ob HW-Element 0x%02X schon existiert", i2cAddress, HWDevice->at(i).i2cAddress);
      Serial.println(buffer);
    }    
    if (HWDevice->at(i).i2cAddress == i2cAddress) {
      if (Config->GetDebugLevel() >=4) { 
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "HW-Element von i2cAdresse 0x%02X gefunden", i2cAddress);
        Serial.println(buffer);
      }
      return true;
    }
  }
  return false;
}

HWdev_t* valveHardware::getI2CDevice(uint8_t i2cAddress) {
  for (uint8_t i=0; i<HWDevice->size(); i++) {
    if (HWDevice->at(i).i2cAddress == i2cAddress) {
      return &HWDevice->at(i);
    }
  }
  return NULL;
}

void valveHardware::ConnectHWdevice(HWdev_t* dev) {
  if(dev->HWType == PCF) {
    PCF8574* pcf8574 = new PCF8574(dev->i2cAddress, this->pin_sda, this->pin_scl);
    pcf8574->begin();
    dev->Device = pcf8574;
  } else if(dev->HWType == TB6612) {
    tb6612* motor = new tb6612();
    motor->init(dev->i2cAddress); 
    dev->Device = motor;
  }

 if (Config->GetDebugLevel() >=3) { 
    char buffer[100] = {0};
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Hardwaredevice fuer Typ %d auf i2c-Adresse 0x%02X erfolgreich erstellt", dev->HWType, dev->i2cAddress);
    Serial.println(buffer);
  }
}

bool valveHardware::RegisterPort(HWdev_t*& dev, uint8_t Port) {
  return this->RegisterPort(dev, Port, false);
}

bool valveHardware::RegisterPort(HWdev_t*& dev, uint8_t Port, bool reverse) {
  char buffer[200] = {0};
  bool success = false;
  if (Config->GetDebugLevel() >=4) { 
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Fordere Registrierung Port %d an", Port);
    Serial.println(buffer);
  }
    
  PortMap_t PortMap;
  PortMap.Port = Port;
  this->PortMapping(&PortMap); // need i2cAddress and internalPort

  bool state = false ^ reverse; // default: OFF
  
  if (PortMap.Port !=0) {
    addI2CDevice(PortMap.i2cAddress);
    dev = getI2CDevice(PortMap.i2cAddress);
    
    if(dev->HWType == PCF) {
      PCF8574* pcf8574 = static_cast<PCF8574*>(dev->Device);
      pcf8574->pinMode(PortMap.internalPort, OUTPUT);
      pcf8574->digitalWrite(PortMap.internalPort, !state); // normal: HIGH
      success = true;
    } else if (dev->HWType == TB6612) {
      tb6612* motor = static_cast<tb6612*>(dev->Device);
      motor->setOff(PortMap.internalPort);
      success = true;
    } else if (dev->HWType == OW2408) {
      ow2408* MyDS2408 = static_cast<ow2408*>(dev->Device);
      MyDS2408->setPort(PortMap.internalPort, state);
      success = true;
    } else if (dev->HWType == ONBOARD) {
      pinMode(PortMap.internalPort, OUTPUT);
      digitalWrite(PortMap.internalPort, state); // normal: LOW
      success = true;
    }
  }
  
  if (Config->GetDebugLevel() >=4) { 
    memset(buffer, 0, sizeof(buffer));
    if (success) {
      sprintf(buffer, "Port %d als internalPort %d fuer HardwareTyp %d auf i2c-Adresse 0x%02X erfolgreich registriert", Port, PortMap.internalPort, dev->HWType, dev->i2cAddress);
    } else {
      sprintf(buffer, "Fehler bei der Registrierung des Ports %d ", Port);
    }
    Serial.println(buffer);
  }
  
  if (success) { return true; }
  else { return false; }
}

bool valveHardware::IsValidPort(uint8_t Port) {
  PortMap_t PortMap;
  PortMap.Port = Port;
  PortMapping(&PortMap);
  if(PortMap.Port == Port) {return true;} else {return false;}
}

uint8_t valveHardware::GetI2CAddress(uint8_t Port) {
  PortMap_t PortMap;
  PortMap.Port = Port;
  PortMapping(&PortMap);
  return PortMap.i2cAddress;
}

void valveHardware::SetPort(HWdev_t* dev, uint8_t Port, bool state, bool reverse) {
  this->SetPort(dev, Port, 0 , state, reverse, 0);
}

void valveHardware::SetPort(HWdev_t* dev, uint8_t Port1, uint8_t Port2, bool state, bool reverse, uint16_t duration) {
  PortMap_t PortMap1, PortMap2;
  PortMap1.Port = Port1; PortMap2.Port = Port2;
  PortMapping(&PortMap1); PortMapping(&PortMap2); // need internalPort

  state = state ^ reverse;
  
  if (dev->HWType == PCF) { //schaltet auf LOW
    PCF8574* pcf8574 = static_cast<PCF8574*>(dev->Device); // , pin_sda, pin_scl
    pcf8574->digitalWrite(PortMap1.internalPort, !state); // Normal: HIGH
    if (Port2 && Port2 > 0) {
      pcf8574->digitalWrite(PortMap2.internalPort, state);
      delay(duration);
      pcf8574->digitalWrite(PortMap1.internalPort, state); 
      pcf8574->digitalWrite(PortMap2.internalPort, !state); 
    }
  } else if (dev->HWType == TB6612) {
    tb6612* motor = static_cast<tb6612*>(dev->Device);
    if (duration && duration > 0) {
      motor->setOn(PortMap1.internalPort, state);
      delay(duration);
      motor->setOff(PortMap1.internalPort);
    } 
    // Port 2 nicht relevant
  } else if (dev->HWType == OW2408) {
    ow2408* MyDS2408 = static_cast<ow2408*>(dev->Device);
    MyDS2408->setPort(PortMap1.internalPort, state);
    if (Port2 && Port2 > 0) {
      MyDS2408->setPort(PortMap2.internalPort, !state); // Normal: HIGH
      delay(duration);
      MyDS2408->setPort(PortMap1.internalPort, !state);
      MyDS2408->setPort(PortMap2.internalPort, state); // Normal: LOW
    }
  } else if (dev->HWType == ONBOARD) {
    digitalWrite(PortMap1.internalPort,  state); // Bistabil: set Direction
    if (Port2 && Port2 > 0) {
      digitalWrite(PortMap2.internalPort, true); // Bistabil: set ON
      delay(duration);
      digitalWrite(PortMap2.internalPort, false); // Bistabil: set OFF
    }
  }

  if (Config->GetDebugLevel() >=5)  { 
    char buffer[100] = {0};
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Aenderung Port %d nach Status: %s ", Port1, vState(state));
    Serial.println(buffer);
  }
}

void valveHardware::setHWType(HWdev_t* dev) {
  if (dev->i2cAddress >= 0x20 and dev->i2cAddress <= 0x27) {
    dev->HWType = PCF;
  } else if (dev->i2cAddress >= 0x38 and dev->i2cAddress <= 0x3F) {
    dev->HWType = PCF;
  } else if (dev->i2cAddress == 0x00) {
    dev->HWType = ONBOARD;
  } else if(dev->i2cAddress >= 0x2D and dev->i2cAddress <= 0x30) {
    dev->HWType = TB6612;
  } else if(dev->i2cAddress == 0x01) {
    dev->HWType = OW2408;
  }
}

// see Definition: https://www.letscontrolit.com/wiki/index.php/PCF8574
void valveHardware::PortMapping(PortMap_t* Map) {
    if (Map->Port >=1 && Map->Port <=8) {
    Map->i2cAddress=0x20;
    Map->internalPort=Map->Port-1;
    Map->HWType = PCF;
  } else if (Map->Port >=9 && Map->Port <=16) {
    Map->i2cAddress=0x21;
    Map->internalPort=Map->Port-9;
    Map->HWType = PCF;
  } else if (Map->Port >=17 && Map->Port <=24) {
    Map->i2cAddress=0x22;
    Map->internalPort=Map->Port-17;
    Map->HWType = PCF;
  } else if (Map->Port >=25 && Map->Port <=32) {
    Map->i2cAddress=0x23;
    Map->internalPort=Map->Port-25;
    Map->HWType = PCF;
  } else if (Map->Port >=33 && Map->Port <=40) {
    Map->i2cAddress=0x24;
    Map->internalPort=Map->Port-33;
    Map->HWType = PCF;
  } else if (Map->Port >=41 && Map->Port <=48) {
    Map->i2cAddress=0x25;
    Map->internalPort=Map->Port-41;
    Map->HWType = PCF;
  } else if (Map->Port >=49 && Map->Port <=56) {
    Map->i2cAddress=0x26;
    Map->internalPort=Map->Port-49;
    Map->HWType = PCF;
  } else if (Map->Port >=57 && Map->Port <=64) {
    Map->i2cAddress=0x27;
    Map->internalPort=Map->Port-57;
    Map->HWType = PCF;
  } else if (Map->Port >=65 && Map->Port <=72) {
    Map->i2cAddress=0x38;
    Map->internalPort=Map->Port-65;
    Map->HWType = PCF;
  } else if (Map->Port >=73 && Map->Port <=80) {
    Map->i2cAddress=0x39;
    Map->internalPort=Map->Port-73;
    Map->HWType = PCF;
  } else if (Map->Port >=81 && Map->Port <=88) {
    Map->i2cAddress=0x3A;
    Map->internalPort=Map->Port-81;
    Map->HWType = PCF;
  } else if (Map->Port >=89 && Map->Port <=96) {
    Map->i2cAddress=0x3B;
    Map->internalPort=Map->Port-89;
    Map->HWType = PCF;
  } else if (Map->Port >=97 && Map->Port <=104) {
    Map->i2cAddress=0x3C;
    Map->internalPort=Map->Port-97;
    Map->HWType = PCF;
  } else if (Map->Port >=105 && Map->Port <=112) {
    Map->i2cAddress=0x3D;
    Map->internalPort=Map->Port-105;
    Map->HWType = PCF;
  } else if (Map->Port >=113 && Map->Port <=112) {
    Map->i2cAddress=0x3E;
    Map->internalPort=Map->Port-113;
    Map->HWType = PCF;
  } else if (Map->Port >=121 && Map->Port <=128) {
    Map->i2cAddress=0x3F;
    Map->internalPort=Map->Port-121;
    Map->HWType = PCF;
  } else if (Map->Port == 130) {
    Map->i2cAddress=0x2D;
    Map->internalPort=0;
    Map->HWType = TB6612;
  } else if (Map->Port == 131) {
    Map->i2cAddress=0x2D;
    Map->internalPort=1;
    Map->HWType = TB6612;
  } else if (Map->Port == 132) {
    Map->i2cAddress=0x2E;
    Map->internalPort=0;
    Map->HWType = TB6612;
  } else if (Map->Port == 133) {
    Map->i2cAddress=0x2E;
    Map->internalPort=1;
    Map->HWType = TB6612;
  } else if (Map->Port == 134) {
    Map->i2cAddress=0x2F;
    Map->internalPort=0;
    Map->HWType = TB6612;
  } else if (Map->Port == 135) {
    Map->i2cAddress=0x2F;
    Map->internalPort=1;
    Map->HWType = TB6612;
  } else if (Map->Port == 136) {
    Map->i2cAddress=0x30;
    Map->internalPort=0;
    Map->HWType = TB6612;
  } else if (Map->Port == 137) {
    Map->i2cAddress=0x30;
    Map->internalPort=1;
    Map->HWType = TB6612;
  } else if (Map->Port >=140 && Map->Port <=199) {
    // nur die Ports anzeigen die auch wirklich vorhanden sind
    if (Config->Enabled1Wire() && this->I2CIsPresent(0x01)) {
      HWdev_t* t = this->getI2CDevice(0x01);
      ow2408* MyDS2408 = static_cast<ow2408*>(t->Device);
      if (MyDS2408->isValidPort(Map->Port-140)) {
        Map->i2cAddress=0x01; //Fake i2c
        Map->internalPort=Map->Port-140;
        Map->HWType = OW2408;
      } else Map->Port = 0;
    } else Map->Port = 0;
  } else if (Map->Port >=200 && Map->Port <=250) {
    // interne GPIO
    Map->i2cAddress=0x00;
    Map->internalPort=Map->Port-200;
    Map->HWType = ONBOARD;
  } else {
    Map->Port = 0;
  }
}

void valveHardware::GetWebContent1Wire(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen) {
  HWdev_t* t = getI2CDevice(0x01);
  ow2408* MyDS2408 = static_cast<ow2408*>(t->Device);
  MyDS2408->GetWebContent1Wire(buffer, processedRows, currentRow, len, maxLen);
}
