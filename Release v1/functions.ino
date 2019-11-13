
void ReadConfigParam() {
  boolean loadDefaultConfig = false;
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");

    // ############### PIN CONFIG ##################
    if (SPIFFS.exists("/PinConfig.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/PinConfig.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          if (json.containsKey("mqtt_server"))         { strcpy(mqtt_server, json["mqtt_server"]);}
          if (json.containsKey("mqtt_root"))           { strcpy(mqtt_root, json["mqtt_root"]);}
          if (json.containsKey("mqtt_port"))           { mqtt_port = json["mqtt_port"];}
          if (json.containsKey("pin_hcsr04_trigger"))  { pin_hcsr04_trigger = json["pin_hcsr04_trigger"];}
          if (json.containsKey("pin_hcsr04_echo"))     { pin_hcsr04_echo = json["pin_hcsr04_echo"];}
          if (json.containsKey("pin_sda"))             { pin_sda = json["pin_sda"];}
          if (json.containsKey("pin_scl"))             { pin_scl = json["pin_scl"];}
          if (json.containsKey("i2caddress_oled"))     { i2caddress_oled = json["i2caddress_oled"];}

        } else {
          Serial.println("failed to load json config, load default config");
          loadDefaultConfig = true;
        }
      }
    } else {
      Serial.println("PinConfig.json config File not exists, load default config");
      loadDefaultConfig = true;
    }

    if (!loadDefaultConfig) {
      // do something
      loadDefaultConfig = false; //set back
    }

    // ############### SENSOR CONFIG ##################
    if (SPIFFS.exists("/SensorConfig.json")) {
      //file exists, reading and loading
      Serial.println("reading sensor config file");
      File configFile = SPIFFS.open("/SensorConfig.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          if (json.containsKey("measurecycle"))   { measurecycle = max(atoi(json["measurecycle"]), 10);}
          if (json.containsKey("measureDistMin")) { measureDistMin = atoi(json["measureDistMin"]);}
          if (json.containsKey("measureDistMax")) { measureDistMax = atoi(json["measureDistMax"]);}
          if (json.containsKey("measureType"))    { 
            if(strcmp(json["measureType"],"analog")==0)      {measureType=ANALOG;}
            else if(strcmp(json["measureType"],"hcsr04")==0) {measureType=HCSR04;}              
            else if(strcmp(json["measureType"],"none")==0)   {measureType=NONE;}  
          }
          
        } else {
          Serial.println("failed to load json config, load default config");
          loadDefaultConfig = true;
        }
      }
    } else {
      Serial.println("SensorConfig.json config File not exists, load default config");
      loadDefaultConfig = true;
    }

    if (loadDefaultConfig) {
      // do something
      loadDefaultConfig = false; //set back
    }

    // ############### Automatik CONFIG ##################
    if (SPIFFS.exists("/AutoConfig.json")) {
      //file exists, reading and loading
      Serial.println("reading Automatik config file");
      File configFile = SPIFFS.open("/AutoConfig.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          if (json.containsKey("treshold_min"))      { treshold_min = json["treshold_min"];}
          if (json.containsKey("treshold_max"))      { treshold_max = json["treshold_max"];}
          if (json.containsKey("syncswitch_port"))   { syncswitch_port = json["syncswitch_port"];}
          if (json.containsKey("ventil3wege_port"))  { ventil3wege_port = json["ventil3wege_port"];}
          if (json.containsKey("max_parallel"))      { max_parallel = json["max_parallel"];}

          if (strcmp(json["enable_syncswitch"],"1")==0) {enable_syncswitch = true;} else {enable_syncswitch = false;}
          if (strcmp(json["enable_3wege"],"1")==0)      {enable_3wege = true;}      else {enable_3wege = false;}
        } else {
          Serial.println("failed to load json config, load default config");
          loadDefaultConfig = true;
        }
      }
    } else {
      Serial.println("AutoConfig.json config File not exists, load default config");
      loadDefaultConfig = true;
    }

    if (loadDefaultConfig) {
      // do something
      loadDefaultConfig = false; //set back
    }

  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void CallWiFiManager() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  //set config save notify callback
  //wifiManager.setSaveConfigCallback(saveConfigCallback);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
  }
  
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

}

void MQTT_reconnect() {
  char topic[50];
  memset(&topic[0], 0, sizeof(topic));
  // Loop until we're reconnected
  //while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    //snprintf (topic, sizeof(topic), "%s-%s", mqtt_root, WiFi.macAddress().c_str());
    snprintf (topic, sizeof(topic), "ESP8266Client_%s", WiFi.macAddress().c_str());
    if (client.connect(topic)) {
      Serial.println("connected, subscribe to:");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      snprintf (topic, sizeof(topic), "%s/#", mqtt_root);
      Serial.println(topic);
      client.subscribe(topic);

      for(int i=0; i < pcf8574devCount; i++) {
        if (strcmp(pcf8574dev[i].type, "v")==0) {
         // auf alle virtuellen Ports subscriben
          sprintf(topic, "/%s/#", pcf8574dev[i].subtopic);
          Serial.println(topic);
          client.subscribe(topic);
        }
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  //}
}

void MQTT_publish(const char* subtopic, bool b) {
  char b1[2];
  memset(&b1[0], 0, sizeof(b1));
  if(b) {strcpy(b1, "1");} else {strcpy(b1, "0");}
  MQTT_publish(subtopic, b1);
}

void MQTT_publish(const char* subtopic, int* number ) {
  char buffer[10];
  memset(&buffer[0], 0, sizeof(buffer));
  itoa(*number, buffer, 10);
  MQTT_publish(subtopic, buffer);
}

void MQTT_publish(const char* subtopic, char* value ) {
  char topic[50];
  memset(&topic[0], 0, sizeof(topic));
  snprintf (topic, sizeof(topic), "%s/%s", mqtt_root, subtopic);
  client.publish(topic, value);
  Serial.print("Publish "); Serial.print(topic); Serial.print(": "); Serial.println(value);
}


void MQTT_callback(char* topic, byte* payload, unsigned int length) {
  char msg[length+1];
  memset(msg,'-', length);
  msg[length]='\0';

  char buffer[length+1];
  memset(buffer, 0, length);
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    msg[i]=(char)payload[i];  
  }
  Serial.print("Message: ");Serial.println(msg);

  int duration = atoi(msg);
  //Serial.print("Duration: ");Serial.println(duration);

  if(strstr(topic,mqtt_root)) {
    //Serial.println(topic+strlen(MQTT_ROOT));
    /* ------------ Status PLAY / PAUSE / STOP ------------- */
    if (strcmp(topic+strlen(mqtt_root),"/test/on-for-timer")==0)                { PCF8574_onfortimer(&duration, &pcf8574dev[0]); }
    
    for(int i=0; i < pcf8574devCount; i++) {
      sprintf(buffer, "/%s/on-for-timer", pcf8574dev[i].subtopic);
      //Serial.print("Check ");Serial.println(buffer);
      if (strcmp(topic+strlen(mqtt_root), buffer)==0 && pcf8574dev[i].enabled) { 
        //Serial.print("on-for-timer: Pin gefunden: ");Serial.println(pcf8574dev[i].port);
        PCF8574_onfortimer(&duration, &pcf8574dev[i]);
      }

      sprintf(buffer, "/%s/set", pcf8574dev[i].subtopic);
      if (strcmp(topic+strlen(mqtt_root), buffer)==0 && strcmp(msg, "off")==0 && pcf8574dev[i].enabled) {
        handleSwitch(&pcf8574dev[i], false);
      }
    }
  } else {
    // TODO: virtuelle Ports
    Serial.print("Virtual Port recognized: "); Serial.println(topic);
    for(int i=0; i < pcf8574devCount; i++) {
      if (strcmp(pcf8574dev[i].type, "v")==0) {
        sprintf(buffer, "/%s/on-for-timer", pcf8574dev[i].subtopic);
        if (strcmp(topic, buffer)==0) {
          PCF8574_onfortimer(&duration, &pcf8574dev[i]);
        }
      }
    }
  }
}
