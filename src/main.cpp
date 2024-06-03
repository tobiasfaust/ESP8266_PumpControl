#include <vector>
#include "CommonLibs.h"
#include "baseconfig.h"
#include "valveStructure.h"
#include "MyMqtt.h"
#include "MyWebServer.h"
#include "sensor.h"

#ifdef USE_OLED
  #include "oled.h"
  OLED* oled = NULL;
#endif

#ifdef USE_I2C
  i2cdetect* I2Cdetect = NULL;
#endif

AsyncWebServer server(80);
DNSServer dns;

BaseConfig* Config = NULL;
valveRelation* ValveRel = NULL;
valveStructure* VStruct = NULL;
MyMQTT* mqtt = NULL;
sensor* LevelSensor = NULL;
MyWebServer* mywebserver = NULL;

/* debugmodes
    0 -> nothing
    1 -> major and criticals
    2 -> majors
    3 -> standard
    4 -> more details, plus: available RAM, RSSI via MQTT, WiFi Credentials via Serial
    5 -> max details
*/
//#define debugmode 4  --> in der WebUI -> Basisconfig einstellbar

void myMQTTCallBack(char* topic, byte* payload, unsigned int length) {
  String msg;
  
  for (u_int16_t i = 0; i < length; i++) {
    msg.concat((char)payload[i]);
  }
  
  if (Config->GetDebugLevel() >= 4) { 
    Serial.printf("Message arrived [%s]\nMessage: %s\n", topic, msg.c_str()); 
  }

  if (LevelSensor->GetExternalSensor() && (strcmp(LevelSensor->GetExternalSensor().c_str(), topic)==0)) {
    LevelSensor->SetLvl(atoi(msg.c_str()));
  }
  else if (strstr(topic, "/raw") ||  strstr(topic, "/level") ||  strstr(topic, "/mem") ||  strstr(topic, "/rssi")) {
    /*SensorMeldungen - ignore!*/
  }
  else {
    VStruct->ReceiveMQTT((String)topic, atoi(msg.c_str()));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ready");

  #ifdef ESP8266
    LittleFS.begin();
  #elif ESP32
    LittleFS.begin(true); // true: format LittleFS/NVS if mount fails
  #endif
  
  // Flash Write Issue
  // https://github.com/esp8266/Arduino/issues/4061#issuecomment-428007580
  //LittleFS.format();

  Config = new BaseConfig();

  #ifdef USE_I2C
    Serial.print(F("Starting WIRE at (SDA, SCL)): ")); Serial.print(Config->GetPinSDA()); Serial.print(", "); Serial.println(Config->GetPinSCL());
    Wire.begin(Config->GetPinSDA(), Config->GetPinSCL());
  #endif

  #ifdef USE_OLED
    oled = new OLED();    
    if (Config->EnabledOled() ) oled->init(Config->GetPinSDA(), Config->GetPinSCL(), Config->GetI2cOLED());
    oled->Enable(Config->EnabledOled());
  #endif

  Serial.println("Starting Wifi and MQTT");
  mqtt = new MyMQTT(&server, &dns, 
                    Config->GetMqttServer().c_str(), 
                    Config->GetMqttPort(), 
                    Config->GetMqttBasePath().c_str(), 
                    Config->GetMqttRoot().c_str(),
                    (char*)"AP_PumpControl",
                    (char*)"password"
                  );
  
  #ifdef USE_OLED
    mqtt->SetOled(oled);
  #endif

  mqtt->setCallback(myMQTTCallBack);

  #ifdef USE_I2C
    Serial.println("Starting I2CDetect");
    I2Cdetect = new i2cdetect(Config->GetPinSDA(), Config->GetPinSCL());
  #endif
  
  Serial.println("Starting Sensor");
  LevelSensor = new sensor();
  #ifdef USE_OLED
    LevelSensor->SetOled(oled);
  #endif

  Serial.println("Starting Valve Relations");
  ValveRel = new valveRelation();

  Serial.println("Starting Valve Structure");
  VStruct = new valveStructure(Config->GetPinSDA(), Config->GetPinSCL());

  Serial.println("Starting WebServer");
  mywebserver = new MyWebServer(&server, &dns);

  //VStruct->OnForTimer("Valve1", 10); // Test

  Serial.println("Setup finished");
}

void loop() {
  VStruct->loop();
  mqtt->loop();
  LevelSensor->loop();
  mywebserver->loop();
  Config->loop();

  #ifdef USE_OLED
    oled->loop();  
  #endif
}
