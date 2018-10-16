typedef struct {
  boolean active;
  uint8_t pinnumber;
  char gpioname[10];
} gpiopin;

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
  boolean enabled;
  boolean active;
  unsigned long startmillis;
  unsigned int lengthmillis;
  unsigned int pcfport; // 0-8
  unsigned int port; //0 - 220
  char subtopic[20]; //ohne on-for-timer
} pcf8574Device;

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
uint8_t hc_sr04_interval = 10;
uint8_t hc_sr04_distmin = 5;
uint8_t hc_sr04_distmax = 105;

//Automatik Config
uint8_t hc_sr04_treshold_min = 26;
uint8_t hc_sr04_treshold_max = 30;
boolean enable_syncswitch = false; // Ventil welches bei Trinkwasser syncron geschaltet wird
uint8_t syncswitch_port = 0; // Portnumer des Ventils
boolean enable_3wege = false; // wechsel Regen- /Trinkwasser
uint8_t ventil3wege_port = 0; // Portnummer des Ventils
uint8_t max_parallel = 0;
int     parallelThreads = 0;

uint8_t syncswitchDevice;
uint8_t ventil3wegeDevice;

String html_str = "";
uint8_t i2c_adresses[8] = {0};
int hcsr04_level = 0; // level of HC-SR04

unsigned long previousMillis = 0;
unsigned long mqttreconnect_lasttry = 0;

// MQTT Subtopics
const char* MQTT_distance       = "distance";
const char* MQTT_level          = "level";
