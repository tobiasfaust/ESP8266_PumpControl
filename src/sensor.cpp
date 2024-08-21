#include "sensor.h"

sensor::sensor() : 
  Type(NONE), 
  measureDistMin(0), 
  measureDistMax(0), 
  measurecycle(10), 
  level(0), 
  raw(0),
  pinTrigger(5),
  pinEcho(6),
  threshold_min(26),
  threshold_max(30),
  moistureEnabled(false) {
  
  #ifdef ESP8266
    uint8_t pinAnalogDefault = 0;
  #elif ESP32
    uint8_t pinAnalogDefault = 36; // ADC1_CH0 (GPIO 36) 
  #endif
  
  this->pinAnalog = pinAnalogDefault;
  
  LoadJsonConfig(); 
}

void sensor::init_analog(uint8_t pinAnalog) {
  setSensorType(ONBOARD_ANALOG);
  this->pinAnalog = pinAnalog;
  this->MAX_DIST=500; // is maximum by default
}

void sensor::init_hcsr04(uint8_t pinTrigger, uint8_t pinEcho) {
  setSensorType(HCSR04);
  this->MAX_DIST = 23200; // Anything over 400 cm (400*58 = 23200 us pulse) is "out of range"
  this->pinTrigger = pinTrigger;
  this->pinEcho = pinEcho;
  pinMode(this->pinTrigger, OUTPUT);
  pinMode(this->pinEcho, INPUT);
}

void sensor::init_extern(String externalSensor) {
  this->setSensorType(EXTERN);
  this->measurecycle = 10;
  mqtt->Subscribe(externalSensor);
}

void sensor::setSensorType(sensorType_t t) {
  this->Type = t;
}

void sensor::SetLvl(uint8_t lvl) {
  if (Config->GetDebugLevel() >= 4) {
    dbg.printf("Sensor: Set Level from extern: %d\n", lvl);
  }
  this->level = lvl;
}

void sensor::loop_analog() {
  this->raw = 0;
  this->level = 0;
  uint8_t pinanalog = this->pinAnalog;

  #ifdef ESP8266
    pinanalog = A0;;
  #endif

  if (Config->GetDebugLevel() >=4) dbg.printf("start measure, using analog Sensor pin: %d \n", pinanalog);

  this->raw = analogRead(pinanalog);
  
  this->level = map(this->raw, measureDistMin, measureDistMax, 0, 100); // 0-100%
}

void sensor::loop_hcsr04() {
  this->raw = 0;
  this->level = 0;
  
  digitalWrite(this->pinTrigger, LOW);
  delayMicroseconds(2);

  digitalWrite(this->pinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(this->pinTrigger, LOW);

  this->raw = pulseIn(this->pinEcho, HIGH, MAX_DIST); 
  this->raw = (this->raw / 2) / 29.1; //Distance in CM's, use /148 for inches.

  if (this->raw == 0){//Reached timeout
    dbg.println("Out of range");
  } else {
    if (this->measureDistMax - this->measureDistMin > 0) {
      this->level = (((this->measureDistMax - this->raw)*100)/(this->measureDistMax - this->measureDistMin));
    }
  }
}

void sensor::loop() {
  /*start measuring sensor*/
  if (millis() - this->previousMillis_sensor > this->measurecycle*1000) {
    this->previousMillis_sensor = millis();
   
    if (this->Type == ONBOARD_ANALOG) {loop_analog();}
   
    if (this->Type == HCSR04) {loop_hcsr04();}

    if (this->Type != NONE && this->level !=0 && Config->Enabled3Wege()) {
      if (this->level < this->threshold_min) { VStruct->SetOn(Config->Get3WegePort()); }
      if (this->level > this->threshold_max) { VStruct->SetOff(Config->Get3WegePort()); }
    }
    if (this->Type != NONE && this->Type != EXTERN && mqtt) {
      if (this->raw > 0 )   { mqtt->Publish_Int((const char*)"raw", (int)this->raw, false); }
      if (this->level > 0 ) { mqtt->Publish_Int((const char*)"level", (int)this->level, false); }
    }
  
     if (this->Type != NONE && this->Type != EXTERN && Config->GetDebugLevel() >=4) {
      dbg.printf("measured sensor raw value: %d \n", this->raw);
     }
  }
}

void sensor::LoadJsonConfig() {
  mqtt->ClearSubscriptions();

  String selection = "";

  if (LittleFS.exists("/sensorconfig.json")) {
    //file exists, reading and loading
    dbg.println(F("reading sensorconfig.json file"));
    File configFile = LittleFS.open("/sensorconfig.json", "r");
    if (configFile) {
      if (Config->GetDebugLevel() >=3) dbg.println(F("sensorconfig.json is now open"));
      ReadBufferingStream stream{configFile, 64};
      stream.find("\"data\":[");
      do {

        JsonDocument elem;
        DeserializationError error = deserializeJson(elem, stream); 
        if (error) {
          if (Config->GetDebugLevel() >=1) {
            dbg.printf("Failed to parse sensorconfig.json data: %s, load default config\n", error.c_str()); 
          } 
        } else {
          // Print the result
          if (Config->GetDebugLevel() >=5) {dbg.println(F("parsing partial JSON of sensorconfig.json ok")); }
          if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, dbg);} 
        
          if (elem.containsKey("measurecycle"))         { this->measurecycle = _max(elem["measurecycle"].as<int>(), 10);}
          if (elem.containsKey("measureDistMin"))       { this->measureDistMin = elem["measureDistMin"].as<int>();}
          if (elem.containsKey("measureDistMax"))       { this->measureDistMax = elem["measureDistMax"].as<int>();}
          if (elem.containsKey("pinhcsr04trigger"))     { this->pinTrigger = elem["pinhcsr04trigger"].as<int>() - 200;}
          if (elem.containsKey("pinhcsr04echo"))        { this->pinEcho = elem["pinhcsr04echo"].as<int>() - 200;}
          if (elem.containsKey("pinanalog"))            { this->pinAnalog = elem["pinanalog"].as<int>() - 200;}
          if (elem.containsKey("treshold_min"))         { this->threshold_min = elem["treshold_min"].as<int>();}
          if (elem.containsKey("treshold_max"))         { this->threshold_max = elem["treshold_max"].as<int>();}
          if (elem.containsKey("externalSensor"))       { this->externalSensor = elem["externalSensor"].as<String>();}
          if (elem.containsKey("selection"))            { selection = elem["selection"].as<String>(); }
          if (elem.containsKey("sel_moisture"))         { if (elem["sel_moisture"].as<String>() == "on") {this->moistureEnabled = true;} else {this->moistureEnabled = false;}}
        }
      } while (stream.findUntil(",","]"));
      
    } else {
      dbg.println("cannot open existing sensorconfig.json config File, load default SensorConfig"); // -> constructor
    }
  } else {
    dbg.println("sensorconfig.json config File not exists, load default SensorConfig");
  }
}

void sensor::GetInitData(AsyncResponseStream *response) {
  String ret;
  JsonDocument json;

  json["data"].to<JsonObject>();
  json["data"]["sel0"] = ((this->Type==NONE)?1:0);
  json["data"]["sel1"] = ((this->Type==HCSR04)?1:0);
  json["data"]["sel2"] = ((this->Type==ONBOARD_ANALOG)?1:0);


  json["data"]["sel4"] = ((this->Type==EXTERN)?1:0);
  json["data"]["measurecycle"] = this->measurecycle;
  json["data"]["measureDistMin"] = this->measureDistMin;
  json["data"]["measureDistMax"] = this->measureDistMax;
  json["data"]["pinhcsr04trigger"] = this->pinTrigger + 200;
  json["data"]["pinhcsr04echo"] = this->pinEcho + 200;
  json["data"]["pinanalog"] = this->pinAnalog + 200;
  json["data"]["a_measureDistMin"] = this->measureDistMin;
  json["data"]["a_measureDistMax"] = this->measureDistMax;
  json["data"]["externalSensor"] = this->externalSensor;
  json["data"]["treshold_min"] = this->threshold_min;
  json["data"]["treshold_max"] = this->threshold_max;

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";

  serializeJson(json, ret);
  response->print(ret);
}
