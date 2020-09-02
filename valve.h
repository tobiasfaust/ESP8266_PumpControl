#ifndef VALVE_H
#define VALVE_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "valveHardware.h"
#include "MQTT.h"

extern MQTT* mqtt;

class valve {

  enum vType_t {NONE, BISTABIL, NORMAL};
  
  public:
    valve();
    
    void      loop();
    void      init(valveHardware* Device, uint8_t Port, String SubTopic);
    
    bool      OnForTimer(int duration);
    bool      SetOn();
    bool      SetOff();
    int       ActiveTimeLeft(); 
    void      AddPort1(valveHardware* Device, uint8_t Port1);
    void      AddPort2(valveHardware* Device, uint8_t Port2);
    void      SetValveType(String type);
    String    GetValveType();
    uint8_t   GetPort1();
    uint8_t   GetPort2();
    
    bool      enabled;  //grundsätzlich aktiviert in WebUI
    bool      active;  // Ventil ist gerade aktiv/geöffnet
    bool      reverse; // Ventil schliesst auf ON, oeffnet auf OFF
    uint16_t  port1ms; // millisekunden bei Type "b" für Port1: 10-999
    uint16_t  port2ms; // millisekunden bei Type "b" für Port2: 10-999
    String    subtopic; //ohne on-for-timer
    uint16_t  autooff; // anzahl sek wenn das Ventil nach einem ON automatisch spaetestens schliessen soll -> Sicherheitsabschaltung
    
  private:
    HWdev_t*  myHWdev = NULL;      //Pointer auf das Device
    valveHardware* valveHWClass = NULL; // Pointer auf die Klasse um auf die generischen Funktionen zugreifen zu können
    
    uint8_t   port1; //0 - 220
    uint8_t   port2; //0 - 220 , für bistabile Ventile
    uint32_t  startmillis   = 0;
    uint32_t  lengthmillis  = 0;

    vType_t   ValveType;

    bool      HandleSwitch (bool state, int duration);
};

#endif
