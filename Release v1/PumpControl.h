// https://arduinojson.org/v5/doc/decoding/

#pragma once

typedef struct {
  PCF8574* pcf8574;
  uint8_t i2cAddress;
} pcf8574HW;

typedef struct {
  uint8_t i2cAddress;
  uint8_t port;
} pcf8574Port;

typedef struct {
  PCF8574* pcf8574;
  boolean enabled; //grundsätzlich aktiviert in WebUI
  boolean active; // Ventil ist gerade aktiv/geöffnet
  char type[1]; // b: bistabiles Ventil, port2 wird benötigt; n: normal, nur Port1; v: virtual, nur mqttTopic
  unsigned long startmillis;
  unsigned int lengthmillis;
  unsigned int pcfport; // 0-8
  unsigned int pcfport2; // 0-8, für bistabile Ventile
  unsigned int port; //0 - 220
  unsigned int port2; //0 - 220 , für bistabile Ventile
  unsigned int portms; // millisekunden bei Type "b" für Port1: 10-999
  unsigned int port2ms; // millisekunden bei Type "b" für Port2: 10-999
  char subtopic[20]; //ohne on-for-timer
} pcf8574Device;
  
typedef struct {
  boolean enabled;
  pcf8574Device* portA; // pointer
  pcf8574Device* portB;  
} valveRelation;

void CallWiFiManager();
void ReadConfigParam();
void MQTT_reconnect();
void MQTT_publish(const char* subtopic, bool b);
void MQTT_publish(const char* subtopic, int* number );
void MQTT_publish(const char* subtopic, char* value );
void MQTT_callback(char* topic, byte* payload, unsigned int length);

void PCF8574_setup();
void PCF8574_loop();
void PCF8574_onfortimer(int* duration, pcf8574Device* mydev);
void handleSwitch (pcf8574Device* mydev, bool state);
void handleSwitch (pcf8574Device* mydev, bool state, int* duration);
void GetPCF8574Port (pcf8574Port* t, uint8_t port);

void loadValveRelations();

void hcsr04_setup();
void analog_setup();
void oled_setup();

void hcsr04_loop();
void analog_loop();
void auto_3wegeVentil();

void handleNotFound();
void handleRoot();
void handlePinConfig();
void handleSensorConfig();
void handleVentilConfig();
void handleAutoConfig();
void handleRelations();
void handleCSS();
void handleJS();
void handleJSParam();
void handleReboot();
void handleStorePinConfig();
void handleStoreSensorConfig();
void handleStoreVentilConfig2();
void handleStoreAutoConfig();
void handleStoreRelations();

void oled_setup();
void oled_loop();
void display_header();
void display_title(String& title, String& subtitle);
void display_wifibars();

//PIN Config
char mqtt_server[40] = "192.178.10.1";
int  mqtt_port  = 1883;
char mqtt_root[40] = "PumpControl";
uint8_t pin_hcsr04_trigger = 12;
uint8_t pin_hcsr04_echo = 13;
uint8_t pin_sda = 4;
uint8_t pin_scl = 0;
uint8_t i2caddress_oled = 60; //0x3C;

//SensorConfig
enum SensorType {NONE, HCSR04, ANALOG};
SensorType measureType = NONE;
uint8_t measurecycle = 10; //in sek
uint8_t measureDistMin = 5; //HCSR04 in mm, A0 in RAW
uint8_t measureDistMax = 105; //HCSR04 in mm, A0 in RAW

//Automatik Config
uint8_t treshold_min = 26;
uint8_t treshold_max = 30;
boolean enable_syncswitch = false; // Ventil welches bei Trinkwasser syncron geschaltet wird
uint8_t syncswitch_port = 0; // Portnumer des Ventils
boolean enable_3wege = false; // wechsel Regen- /Trinkwasser
uint8_t ventil3wege_port = 0; // Portnummer des Ventils
uint8_t max_parallel = 0;
int     parallelThreads = 0;

uint8_t syncswitchDevice;
uint8_t ventil3wegeDevice;

String html_str = "";
int sensor_level = 0; // level of HC-SR04/Analog: 0-100%
int sensor_RawValue = 0;

unsigned long previousMillis = 0;
unsigned long mqttreconnect_lasttry = 0;

// MQTT Subtopics
const char* MQTT_sensorRawValue       = "raw";
const char* MQTT_level          = "level";

