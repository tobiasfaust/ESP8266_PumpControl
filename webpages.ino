void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleRoot() {
  server.send(200, "text/html", getMainPage());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleCSS() {
  server.send(200, "text/css", getCSS());
}

void handleReboot() {
  server.sendHeader("Location","/");
  server.send(303); 
  ESP.restart();  
}

void handleStoreParams() {
  strcpy(mqtt_server, server.arg("mqttserver").c_str());
  mqtt_port = atoi(server.arg("mqttport").c_str());
  strcpy(mqtt_root, server.arg("mqttroot").c_str());
  hc_sr04_interval = atoi(server.arg("hcsr04interval").c_str());
  pin_hcsr04_trigger = atoi(server.arg("pinhcsr04trigger").c_str());
  pin_hcsr04_echo = atoi(server.arg("pinhcsr04echo").c_str());
  pin_sda = atoi(server.arg("pinsda").c_str());
  pin_scl = atoi(server.arg("pinscl").c_str());
  i2caddress_pfc8574 = strtoul(server.arg("i2cpfc8574").c_str(), NULL, 16); // hex convert to dec
  i2caddress_oled = strtoul(server.arg("i2coled").c_str(), NULL, 16);
  
  //save the custom parameters to FS
  if (true) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_root"] = mqtt_root;
    json["hc_sr04_interval"] = hc_sr04_interval;
    json["pin_hcsr04_trigger"] = pin_hcsr04_trigger;
    json["pin_hcsr04_echo"] = pin_hcsr04_echo;
    json["pin_sda"] = pin_sda;
    json["pin_scl"] = pin_scl;
    json["i2caddress_pfc8574"] = i2caddress_pfc8574;
    json["i2caddress_oled"] = i2caddress_oled;
    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}


void handleStoreSwitchConfig() {
  char buffer[20] = {0};
  memset(buffer, 0, sizeof(buffer));
  char enabled[1] = {0};
  
  for(int i=0; i < pcf8574devCount; i++) {
    sprintf(buffer, "mqtttopic_%d", i);
    strcpy(pcf8574dev[i].subtopic, server.arg(buffer).c_str());
    sprintf(buffer, "pcfport_%d", i);
    pcf8574dev[i].port = atoi(server.arg(buffer).c_str());
    sprintf(buffer, "active_%d", i);
    strcpy(enabled, server.arg(buffer).c_str());
    if (strcmp(enabled,"1")==0) { pcf8574dev[i].enabled = true;} else { pcf8574dev[i].enabled = false;}
  }
  
  //save the custom parameters to FS
  if (true) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["count"] = pcf8574devCount;
    for(int i=0; i < pcf8574devCount; i++) {
      sprintf(buffer, "mqtttopic_%d", i);
      json[buffer] = pcf8574dev[i].subtopic;
      sprintf(buffer, "pcfport_%d", i);
      json[buffer] = pcf8574dev[i].port;
      sprintf(buffer, "active_%d", i);
      json[buffer] = (pcf8574dev[i].enabled?"1":"0");
    }
    
    File configFile = SPIFFS.open("/config2.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    
  }
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

