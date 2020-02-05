#include "sensor.h"

sensor::sensor() : Type(NONE), measureDistMin(0), measureDistMax(0), measurecycle(10), level(0), raw(0) {
  SPIFFS.begin();
  LoadJsonConfig(); 
}

void sensor::init() {
  setSensorType(ANALOG);
  this->MAX_DIST=500; // is maximum by default
}

void sensor::init(uint8_t pinTrigger, uint8_t pinEcho) {
  setSensorType(HCSR04);
  this->MAX_DIST = 23200; // Anything over 400 cm (400*58 = 23200 us pulse) is "out of range"
  this->pinTrigger = pinTrigger;
  this->pinEcho = pinEcho;
  pinMode(this->pinTrigger, OUTPUT);
  pinMode(this->pinEcho, INPUT);
}

void sensor::init(String externalSensor) {
  setSensorType(EXTERN);
  this->measurecycle = 10;
  mqtt->Subscribe(externalSensor, MQTT::SENSOR);
}

void sensor::setSensorType(sensorType_t t) {
  this->Type = t;
}

void sensor::SetLvl(uint8_t lvl) {
  this->level = lvl;
  oled->SetLevel(this->level);
}

void sensor::loop_analog() {
  this->raw = 0;
  this->level = 0;
  
  this->raw = analogRead(A0);
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
    Serial.println("Out of range");
  } else {
    if (this->measureDistMax - this->measureDistMin > 0) {
      this->level = (((this->measureDistMax - this->raw)*100)/(this->measureDistMax - this->measureDistMin));
    }
  }
}

void sensor::loop() {
  if (millis() - this->previousMillis > this->measurecycle*1000) {
    this->previousMillis = millis();
   
    if (this->Type == ANALOG) {loop_analog();}
    if (this->Type == HCSR04) {loop_hcsr04();}

    if (this->Type != NONE && this->level !=0 && Config->Enabled3Wege()) {
      if (this->level < this->threshold_min) { VStruct->SetOn(Config->Get3WegePort()); }
      if (this->level > this->threshold_max) { VStruct->SetOff(Config->Get3WegePort()); }
    }
    if (this->Type != NONE && this->Type != EXTERN && mqtt) {
      if (this->raw > 0 )   { mqtt->Publish_Int((const char*)"raw", (int*)this->raw); }
      if (this->level > 0 ) { mqtt->Publish_Int((const char*)"level", (int*)this->level); }
    }
    
    if (this->Type != NONE && this->Type != EXTERN) {
      oled->SetLevel(this->level);
    }
  }
}

void sensor::StoreJsonConfig(String* json) {
  //https://arduinojson.org/v5/api/jsonobject/begin_end/
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(*json);
    
  if (root.success()) {
    File configFile = SPIFFS.open("/SensorConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open SensorConfig.json file for writing");
    } else {  
      root.printTo(Serial);
      root.printTo(configFile);
      configFile.close();

      LoadJsonConfig();
    }
  }
}

void sensor::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  
  mqtt->ClearSubscriptions(MQTT::SENSOR);
  
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
        if (json.containsKey("measurecycle"))         { this->measurecycle = _max(atoi(json["measurecycle"]), 10);}
        if (json.containsKey("measureDistMin"))       { this->measureDistMin = atoi(json["measureDistMin"]);}
        if (json.containsKey("measureDistMax"))       { this->measureDistMax = atoi(json["measureDistMax"]);}
        if (json.containsKey("pinhcsr04trigger"))     { this->pinTrigger = atoi(json["pinhcsr04trigger"]) - 200;}
        if (json.containsKey("pinhcsr04echo"))        { this->pinEcho = atoi(json["pinhcsr04echo"]) - 200;}
        if (json.containsKey("treshold_min"))         { this->threshold_min = atoi(json["treshold_min"]);}
        if (json.containsKey("treshold_max"))         { this->threshold_max = atoi(json["treshold_max"]);}
        if (json.containsKey("externalSensor"))       { this->externalSensor = json["externalSensor"].as<String>();}
        if(strcmp(json["selection"],"analog")==0)        { init(); }
          else if(strcmp(json["selection"],"hcsr04")==0) { init(this->pinTrigger, this->pinEcho); }
          else if(strcmp(json["selection"],"extern")==0) { init(this->externalSensor); }
          else if(strcmp(json["selection"],"none")==0)   { this->Type=NONE; Serial.println("No LevelSensor requested"); }  
        
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
    this->threshold_min = 26;
    this->threshold_max = 30;

    loadDefaultConfig = false; //set back
  }
}

void sensor::GetWebContent(String* html) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  html->concat("<form id='DataForm'>\n");
  html->concat("<table id='maintable' class='editorDemoTable'>\n");
  html->concat("<thead>\n");
  html->concat("  <tr>\n");
  html->concat("    <td style='width: 250px;'>Name</td>\n");
  html->concat("    <td style='width: 200px;'>Wert</td>\n");
  html->concat("  </tr>\n");
  html->concat("</thead>\n");
  html->concat("<tbody>\n");

  html->concat("<tr>\n");
  html->concat("  <td colspan='2'>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel0' name='selection' value='none' %s onclick=\"radioselection([''],['all_1','all_2','all_3','analog_1','analog_2','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','extern_1'])\"/>", (this->Type==NONE)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel0'>keine Füllstandsmessung</label></div>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel1' name='selection' value='hcsr04' %s onclick=\"radioselection(['all_1','all_2','all_3','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4'],['analog_1','analog_2','extern_1'])\"/>", (this->Type==HCSR04)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel1'>Füllstandsmessung mit Ultraschallsensor HCSR04</label></div>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel2' name='selection' value='analog' %s onclick=\"radioselection(['all_1','all_2','all_3','analog_1','analog_2'],['hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','extern_1'])\"/>", (this->Type==ANALOG)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel2'>Füllstandsmessung mit analogem Signal (an A0)</label></div>\n");

  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel3' name='selection' value='extern' %s onclick=\"radioselection(['all_2','all_3','extern_1'],['all_1','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','analog_1','analog_2'])\"/>", (this->Type==EXTERN)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel3'>Füllstandsmessung mit externem Signal per MQTT</label></div>\n");
  
  html->concat("  </td>\n");
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='all_1'>\n", (this->Type==NONE||this->Type==EXTERN?"hide":""));
  html->concat(buffer);
  html->concat("<td>Messintervall</td>\n");
  sprintf(buffer, "<td><input min='0' max='254' name='measurecycle' type='number' value='%d'/></td>\n", this->measurecycle);
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='hcsr04_1'>\n", (this->Type==HCSR04?"":"hide"));
  html->concat(buffer);
  html->concat("<td>Abstand Sensor min (in cm)</td>\n");
  sprintf(buffer, "<td><input min='0' max='254' name='measureDistMin' type='number' value='%d'/></td>\n", this->measureDistMin);
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='hcsr04_2'>\n", (this->Type==HCSR04?"":"hide"));
  html->concat(buffer);
  html->concat("<td>Abstand Sensor max (in cm)</td>\n");
  sprintf(buffer, "<td><input min='0' max='254' name='measureDistMax' type='number' value='%d'/></td>\n", this->measureDistMax);
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='hcsr04_3'>\n", (this->Type==HCSR04?"":"hide"));
  html->concat(buffer);
  html->concat("<td>Pin HC-SR04 Trigger</td>\n");
  sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_1' name='pinhcsr04trigger' type='number' value='%d'/></td>\n", this->pinTrigger + 200);
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='hcsr04_4'>\n", (this->Type==HCSR04?"":"hide"));
  html->concat(buffer);
  html->concat("<td>Pin HC-SR04 Echo</td>\n");
  sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_2' name='pinhcsr04echo' type='number' value='%d'/></td>\n", this->pinEcho + 200);
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='analog_1'>\n", (this->Type==ANALOG?"":"hide"));
  html->concat(buffer);
  html->concat("<td>Kalibrierung: 0% entsricht RAW Wert</td>\n");
  sprintf(buffer, "<td><input min='0' size='5' name='measureDistMin' type='number' value='%d'/></td>\n", this->measureDistMin);
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='analog_2'>\n", (this->Type==ANALOG?"":"hide"));
  html->concat(buffer);
  html->concat("<td>Kalibrierung: 100% entsricht RAW Wert</td>\n");
  sprintf(buffer, "<td><input min='0' max='1024' name='measureDistMax' type='number' value='%d'/></td>\n", this->measureDistMax);
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='extern_1'>\n", (this->Type==EXTERN?"":"hide"));
  html->concat(buffer);
  html->concat("<td>MQTT-Topic des externen Sensors (Füllstand in %)</td>\n");
  sprintf(buffer, "<td><input size='30' name='externalSensor' type='text' value='%s'/></td>\n", this->externalSensor.c_str());
  html->concat(buffer);
  html->concat("</tr>\n");
  
  sprintf(buffer, "<tr class='%s' id='all_2'>\n", (this->Type==NONE?"hide":""));
  html->concat(buffer);
  html->concat("<td >Sensor Treshold Min für 3WegeVentil</td>\n");
  sprintf(buffer, "<td><input min='0' max='254' name='treshold_min' type='number' value='%d'/></td>\n", this->threshold_min);
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='all_3'>\n", (this->Type==NONE?"hide":""));
  html->concat(buffer);
  html->concat("<td>Sensor Treshold Max für 3WegeVentil</td>\n");
  sprintf(buffer, "<td><input min='0' max='254' name='treshold_max' type='number' value='%d'/></td>\n", this->threshold_max);
  html->concat(buffer);
  html->concat("</tr>\n");
  html->concat("<tr>\n");
  
  html->concat("</tbody>\n");
  html->concat("</table>\n");
  html->concat("</form>\n\n<br />\n");
  html->concat("<form id='jsonform' action='StoreSensorConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html->concat("  <input type='text' id='json' name='json' />\n");
  html->concat("  <input type='submit' value='Speichern' />\n");
  html->concat("</form>\n\n");
  html->concat("<div id='ErrorText' class='errortext'></div>\n");
}
