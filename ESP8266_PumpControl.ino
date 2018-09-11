#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h> 

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h>
#include <Wire.h>
#include "Arduino.h"
#include "PCF8574.h"  

typedef struct {
  uint8_t i2cAddress;
  uint8_t port;
} pcf8574Port;

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40] = "192.178.10.1";
char mqtt_port[6] = "1883";
char mqtt_root[40] = "PumpControl/";
uint8_t hc_sr04_interval = 10;
uint8_t pin_hcsr04_trigger = 12;
uint8_t pin_hcsr04_echo = 13;
uint8_t pin_sda = 4;
uint8_t pin_scl = 0;
uint8_t i2caddress_pfc8574 = 0x38;
uint8_t i2caddress_oled = 0x3C;

MDNSResponder mdns;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;
PubSubClient client(espClient);


std::string html_str = "";
uint8_t i2c_adresses[8] = {0};

unsigned long previousMillis = 0;
unsigned long mqttreconnect_lasttry = 0;
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();
    
  CallWiFiManager();
  ReadConfigParam();

  client.setServer(mqtt_server, 1883);
  client.setCallback(MQTT_callback);
  
  if (!MDNS.begin("esp8266"))   {  Serial.println("Error setting up MDNS responder!");  }
  else                          {  Serial.println("mDNS responder started");  }

  server.onNotFound(handleNotFound);
  server.on("/", handleRoot);
  server.on("/StoreParam", HTTP_POST, handleStoreParams);
  server.on("/reboot", HTTP_GET, handleReboot);

  // start a server
  httpUpdater.setup(&server);
  server.begin();
  Serial.println("Server started");
  
  Wire.begin(pin_sda, pin_scl);
  i2cdetect();

  hcsr04_setup();
  oled_setup();
  PCF8574_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  if (!client.connected()) { 
      if (millis() - mqttreconnect_lasttry > 10000) {
        MQTT_reconnect(); 
        mqttreconnect_lasttry = millis();
    }
  }
  client.loop();
  
  if (millis() - previousMillis > hc_sr04_interval*1000) {
    previousMillis = millis();   // aktuelle Zeit abspeichern
    hcsr04_loop();
  }

  PCF8574_loop();
  oled_loop();
}



