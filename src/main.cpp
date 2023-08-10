#include <vector>
#include "BaseConfig.h"
#include "valveStructure.h"
#include "mqtt.h"
#include "MyWebServer.h"
#include "sensor.h"
#include "oled.h"

AsyncWebServer server(80);
DNSServer dns;

i2cdetect* I2Cdetect = NULL;
BaseConfig* Config = NULL;
valveRelation* ValveRel = NULL;
valveStructure* VStruct = NULL;
MyMQTT* mqtt = NULL;
sensor* LevelSensor = NULL;
OLED* oled = NULL;
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
  if (Config->GetDebugLevel() >= 4) { Serial.printf("Message arrived [%s]\n", topic); }

  for (u_int16_t i = 0; i < length; i++) {
    msg.concat((char)payload[i]);
  }
  Serial.print("Message: "); Serial.println(msg.c_str());

  if (LevelSensor->GetExternalSensor() == topic && atoi(msg.c_str()) > 0) {
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

  Serial.print(F("Starting WIRE at (SDA, SCL)): ")); Serial.print(Config->GetPinSDA()); Serial.print(", "); Serial.println(Config->GetPinSCL());
  Wire.begin(Config->GetPinSDA(), Config->GetPinSCL());

  oled = new OLED();    
  if (Config->EnabledOled() ) oled->init(Config->GetPinSDA(), Config->GetPinSCL(), Config->GetI2cOLED());
  oled->Enable(Config->EnabledOled());

  Serial.println("Starting Wifi and MQTT");
  mqtt = new MyMQTT(&server, &dns, Config->GetMqttServer().c_str(), Config->GetMqttPort(), Config->GetMqttBasePath().c_str(), Config->GetMqttRoot().c_str());
  mqtt->SetOled(oled);
  mqtt->setCallback(myMQTTCallBack);

  Serial.println("Starting I2CDetect");
  I2Cdetect = new i2cdetect(Config->GetPinSDA(), Config->GetPinSCL());

  Serial.println("Starting Sensor");
  LevelSensor = new sensor();
  LevelSensor->SetOled(oled);

  Serial.println("Valve Relations");
  ValveRel = new valveRelation();

  Serial.println("Starting Valve Structure");
  VStruct = new valveStructure(Config->GetPinSDA(), Config->GetPinSCL());

  Serial.println("Starting WebServer");
  mywebserver = new MyWebServer(&server, &dns);

  //VStruct->OnForTimer("Valve1", 10); // Test
}

void loop() {
  // put your main code here, to run repeatedly:
  VStruct->loop();
  mqtt->loop();
  LevelSensor->loop();
  mywebserver->loop();
  Config->loop();
  oled->loop();  
}
