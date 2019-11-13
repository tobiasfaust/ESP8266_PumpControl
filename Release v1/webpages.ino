#include "web_css.h"
#include "web_js.h"

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleRoot() {
  server.send(200, "text/html", getPage_Status());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handlePinConfig() {
  server.send(200, "text/html", getPage_PinConfig());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleSensorConfig() {
  server.send(200, "text/html", getPage_SensorConfig());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleVentilConfig() {
  server.send(200, "text/html", getPage_VentilConfig());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleAutoConfig() {
  server.send(200, "text/html", getPage_AutoConfig());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleRelations() {
  server.send(200, "text/html", getPage_Relations());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleCSS() {
  server.send(200, "text/css", STYLE_CSS);
}

void handleJS() {
  server.send(200, "text/javascript", JAVASCRIPT);
}

void handleJSParam() {
  server.send(200, "text/javascript", getJSParam());
}

void handleReboot() {
  server.sendHeader("Location","/");
  server.send(303); 
  delay(3000);
  ESP.restart();  
}

void handleStorePinConfig() {
  strcpy(mqtt_server, server.arg("mqttserver").c_str());
  mqtt_port = atoi(server.arg("mqttport").c_str())|1883;
  strcpy(mqtt_root, server.arg("mqttroot").c_str());
  pin_hcsr04_trigger = atoi(server.arg("pinhcsr04trigger").c_str())|0;
  pin_hcsr04_echo = atoi(server.arg("pinhcsr04echo").c_str())|0;
  pin_sda = atoi(server.arg("pinsda").c_str())|0;
  pin_scl = atoi(server.arg("pinscl").c_str())|0;
  i2caddress_oled = strtoul(server.arg("i2coled").c_str(), NULL, 16); // hex convert to dec
  
  //save the custom parameters to FS
  if (true) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_root"] = mqtt_root;
    json["pin_hcsr04_trigger"] = pin_hcsr04_trigger;
    json["pin_hcsr04_echo"] = pin_hcsr04_echo;
    json["pin_sda"] = pin_sda;
    json["pin_scl"] = pin_scl;
    json["i2caddress_oled"] = i2caddress_oled;
    
    File configFile = SPIFFS.open("/PinConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open PinConfig.json file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  
  server.sendHeader("Location","/PinConfig");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleStoreSensorConfig() {
  char buffer[20] = {0};
  memset(buffer, 0, sizeof(buffer));

  measurecycle = atoi(server.arg("measurecycle").c_str());
  measureDistMin = atoi(server.arg("measureDistMin").c_str());
  measureDistMax = atoi(server.arg("measureDistMax").c_str());
  
  if (strcmp(server.arg("selection").c_str(), "analog")==0) { measureType=ANALOG; }
  if (strcmp(server.arg("selection").c_str(), "hcsr04")==0) { measureType=HCSR04; }
  
  if (true) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["measurecycle"] = measurecycle;
    json["measureDistMin"] = measureDistMin;
    json["measureDistMax"] = measureDistMax;
    json["measureType"] = toLowerCase(measureType);
    
    File configFile = SPIFFS.open("/SensorConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open SensorConfig.json file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
    
  server.sendHeader("Location","/SensorConfig");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

//#############################################
void handleStoreVentilConfig2() {
  //https://arduinojson.org/v5/api/jsonobject/begin_end/
  char buffer[20] = {0};
  unsigned int i;
  memset(buffer, 0, sizeof(buffer));
  
  //char json[] = "{\"first\":\"hello\",\"second\":\"world\"}";
  //strcpy(pcf8574dev[i].subtopic, server.arg(buffer).c_str());
  String json = server.arg("json");
  
  Serial.print("json empfangen: ");
  Serial.println(json);
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root.printTo(Serial);
    
  if (root.success()) {
    File configFile = SPIFFS.open("/VentilConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open VentilConfig.json file for writing");
    }
  
    root.printTo(Serial);
    root.printTo(configFile);
    configFile.close();
  
    // ReRead and initialize Objects
    PCF8574_setup();
    MQTT_reconnect(); // ggf neue virtuelle Ports müssen subscribed werden
    
  } else {
      Serial.println("something went wrong to parse json string");
  }
  server.sendHeader("Location","/VentilConfig");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);     
}

//#############################################
void handleStoreAutoConfig() {
  char buffer[20] = {0};
  memset(buffer, 0, sizeof(buffer));
  uint8_t enabled = 0;

  treshold_min  = atoi(server.arg("treshold_min").c_str());
  treshold_max  = atoi(server.arg("treshold_max").c_str());
  syncswitch_port       = atoi(server.arg("syncswitch_port").c_str());
  ventil3wege_port      = atoi(server.arg("ventil3wege_port").c_str());
  max_parallel          = atoi(server.arg("max_parallel").c_str());
  
  enabled = atoi(server.arg("enable_syncswitch").c_str());
  if (enabled == 1) { enable_syncswitch = true;} else { enable_syncswitch = false;}
  enabled = atoi(server.arg("enable_3wege").c_str());
  if (enabled == 1) { enable_3wege = true;} else { enable_3wege = false;}
  
  if (true) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["treshold_min"] = treshold_min;
    json["treshold_max"] = treshold_max;
    json["syncswitch_port"] = syncswitch_port;
    json["ventil3wege_port"] = ventil3wege_port;
    json["max_parallel"] = max_parallel;
    json["enable_syncswitch"] = (enable_syncswitch?1:0);
    json["enable_3wege"] = (enable_3wege?1:0);
    
    File configFile = SPIFFS.open("/AutoConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open AutoConfig.json file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save

    // ReRead and initialize Objects
    PCF8574_setup();
  }
  
  server.sendHeader("Location","/AutoConfig");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleStoreRelations() {
  char buffer[20] = {0};
  unsigned int i;
  memset(buffer, 0, sizeof(buffer));
  
  String json = server.arg("json");
  
  Serial.print("json empfangen: ");
  Serial.println(json);
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root.printTo(Serial);
    
  if (root.success()) {
    File configFile = SPIFFS.open("/Relations.json", "w");
    if (!configFile) {
      Serial.println("failed to open Relations.json file for writing");
    }
  
    root.printTo(Serial);
    root.printTo(configFile);
    configFile.close();

    // load Relations
    loadValveRelations();
  
  } else {
      Serial.println("something went wrong to parse json string");
  }
  server.sendHeader("Location","/Relations");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);     
}
