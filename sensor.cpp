#include "sensor.h"

sensor::sensor() : Type(NONE), measureDistMin(0), measureDistMax(0), measurecycle(10), level(0), raw(0) {
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
  setSensorType(EXTERN);
  this->measurecycle = 10;
  mqtt->Subscribe(externalSensor, MQTT::SENSOR);
}

void sensor::init_ads1115(uint8_t i2c, uint8_t port) {
  if (Config->GetDebugLevel() >=4) Serial.printf("Init ADS1115 at i2cAdress 0x%02x \n", i2c);
  
  this->ads1115_i2c = i2c;
  this->ads1115_port = port;
  setSensorType(ADS1115);
  
  ADS1115_WE* adc = new ADS1115_WE(0x48);
 
  if(!adc->init()){
    Serial.printf("Could not connect to ADS1115 at i2cAdress 0x%02x \n", i2c );
  } else {
    adc->setVoltageRange_mV(ADS1115_RANGE_4096);
    this->Device = adc;
  }
}

void sensor::SetOled(OLED* oled) {
  this->oled = oled;
}

void sensor::setSensorType(sensorType_t t) {
  this->Type = t;
}

void sensor::SetLvl(uint8_t lvl) {
  this->level = lvl;
  if(this->oled) this->oled->SetLevel(this->level);
}

void sensor::loop_analog() {
  this->raw = 0;
  this->level = 0;
  uint8_t pinanalog = this->pinAnalog;

  #ifdef ESP8266
    pinanalog = A0;;
  #endif

  if (Config->GetDebugLevel() >=4) Serial.printf("start measure, using analog Sensor pin: %d \n", pinanalog);

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
    Serial.println("Out of range");
  } else {
    if (this->measureDistMax - this->measureDistMin > 0) {
      this->level = (((this->measureDistMax - this->raw)*100)/(this->measureDistMax - this->measureDistMin));
    }
  }
}

void sensor::loop_ads1115() {
  this->raw = 0;
  this->level = 0;

  if (!this->Device) {
    if (Config->GetDebugLevel() >=3) Serial.printf("Measure of analog Sensor ADS1115 port %d requested, but not ADS1115 found. Stop measure! \n", this->ads1115_port);
  } else {

    if (Config->GetDebugLevel() >=4) Serial.printf("start measure, use analog Sensor ADS1115 port: %d \n", this->ads1115_port);
  
    switch (this->ads1115_port) {
      case 0:
        this->raw = readADS1115Channel(ADS1115_COMP_0_GND);
      break;
      case 1:
        this->raw = readADS1115Channel(ADS1115_COMP_1_GND);
      break;
      case 2:
        this->raw = readADS1115Channel(ADS1115_COMP_2_GND);
      break;
      case 3:
        this->raw = readADS1115Channel(ADS1115_COMP_3_GND);
      break;
      default:
         Serial.printf("portnummer %d not available \n", this->ads1115_port);
      break;
    }
    this->level = map(this->raw, measureDistMin, measureDistMax, 0, 100); // 0-100%
  }
}

uint16_t sensor::readADS1115Channel(ADS1115_MUX channel) {
  int16_t raw = 0;
  ADS1115_WE* adc = static_cast<ADS1115_WE*>(this->Device);
  adc->setCompareChannels(channel);
  adc->startSingleMeasurement();
  while(adc->isBusy()){}
  raw = adc->getResultWithRange(-4096,4096); 
  return (uint16_t) abs(raw);
}

void sensor::loop() {
  if (millis() - this->previousMillis > this->measurecycle*1000) {
    this->previousMillis = millis();
   
    if (this->Type == ONBOARD_ANALOG) {loop_analog();}
    if (this->Type == ADS1115) {loop_ads1115();}
    if (this->Type == HCSR04) {loop_hcsr04();}

    if (this->Type != NONE && this->level !=0 && Config->Enabled3Wege()) {
      if (this->level < this->threshold_min) { VStruct->SetOn(Config->Get3WegePort()); }
      if (this->level > this->threshold_max) { VStruct->SetOff(Config->Get3WegePort()); }
    }
    if (this->Type != NONE && this->Type != EXTERN && mqtt) {
      if (this->raw > 0 )   { mqtt->Publish_Int((const char*)"raw", (int)this->raw); }
      if (this->level > 0 ) { mqtt->Publish_Int((const char*)"level", (int)this->level); }
    }
    
    if (this->Type != NONE && this->Type != EXTERN) {
      if(this->oled) this->oled->SetLevel(this->level);
    }

     if (Config->GetDebugLevel() >=4) {
      Serial.printf("measured sensor raw value: %d \n", this->raw);
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

  #ifdef ESP8266
    uint8_t pinAnalogDefault = 0;
  #elif ESP32
    uint8_t pinAnalogDefault = 36; // ADC1_CH0 (GPIO 36) 
  #endif
  
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
        if (json.containsKey("pinanalog"))              {this->pinAnalog = atoi(json["pinanalog"]) - 200;} else {this->pinAnalog = pinAnalogDefault; }
        if (json.containsKey("treshold_min"))         { this->threshold_min = atoi(json["treshold_min"]);}
        if (json.containsKey("treshold_max"))         { this->threshold_max = atoi(json["treshold_max"]);}
        if (json.containsKey("ads1115_i2c"))          { this->ads1115_i2c = strtoul(json["ads1115_i2c"], NULL, 16);} // hex convert to dec 
        if (json.containsKey("ads1115_port"))          { this->ads1115_port = atoi(json["ads1115_port"]);}
        if (json.containsKey("externalSensor"))       { this->externalSensor = json["externalSensor"].as<String>();}
        if(strcmp(json["selection"],"analog")==0)        { init_analog(this->pinAnalog); }
          else if(strcmp(json["selection"],"hcsr04")==0) { init_hcsr04(this->pinTrigger, this->pinEcho); }
          else if(strcmp(json["selection"],"extern")==0) { init_extern(this->externalSensor); }
          else if(strcmp(json["selection"],"ads1115")==0) { init_ads1115(this->ads1115_i2c, this->ads1115_port); }
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
    this->pinAnalog = pinAnalogDefault; 

    loadDefaultConfig = false; //set back
  }
}

void sensor::GetWebContent(WM_WebServer* server) {
  char buffer[250] = {0};
  memset(buffer, 0, sizeof(buffer));
  String html = "";

  html.concat("<form id='DataForm'>\n");
  html.concat("<table id='maintable' class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("  <tr>\n");
  html.concat("    <td style='width: 250px;'>Name</td>\n");
  html.concat("    <td style='width: 200px;'>Wert</td>\n");
  html.concat("  </tr>\n");
  html.concat("</thead>\n");
  html.concat("<tbody>\n");

  html.concat("<tr>\n");
  html.concat("  <td colspan='2'>\n");
  
  html.concat("    <div class='inline'>\n");
  snprintf(buffer, sizeof(buffer), "    <input type='radio' id='sel0' name='selection' value='none' %s onclick=\"radioselection([''],['all_1','all_2','all_3','analog_0','analog_1','analog_2','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','extern_1','ads1115_0','ads1115_1'])\"/>", (this->Type==NONE)?"checked":"");
  html.concat(buffer);
  html.concat("    <label for='sel0'>keine Füllstandsmessung</label></div>\n");
  html.concat("    \n");
  
  html.concat("    <div class='inline'>\n");
  snprintf(buffer, sizeof(buffer), "    <input type='radio' id='sel1' name='selection' value='hcsr04' %s ", (this->Type==HCSR04)?"checked":"");
  html.concat(buffer);
  html.concat("onclick=\"radioselection(['all_1','all_2','all_3','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4'],['analog_0','analog_1','analog_2','extern_1','ads1115_0','ads1115_1'])\"/>\n");
  html.concat("    <label for='sel1'>Füllstandsmessung mit Ultraschallsensor HCSR04</label></div>\n");
  html.concat("    \n");
  
  html.concat("    <div class='inline'>\n");
  snprintf(buffer, sizeof(buffer), "    <input type='radio' id='sel2' name='selection' value='analog' %s ", (this->Type==ONBOARD_ANALOG)?"checked":"");
  html.concat(buffer);
  html.concat("onclick=\"radioselection(['all_1','all_2','all_3','analog_0','analog_1','analog_2'],['hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','extern_1','ads1115_0','ads1115_1'])\"/>\n");
  html.concat("    <label for='sel2'>Füllstandsmessung mit Analogsignal am ESP</label></div>\n");
  html.concat("    \n");
  
  html.concat("    <div class='inline'>\n");
  snprintf(buffer, sizeof(buffer), "    <input type='radio' id='sel3' name='selection' value='ads1115' %s ", (this->Type==ADS1115)?"checked":"");
  html.concat(buffer);
  html.concat("onclick=\"radioselection(['all_1','all_2','all_3','analog_1','analog_2','ads1115_0','ads1115_1'],['hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','extern_1','analog_0'])\"/>\n");
  html.concat("    <label for='sel3'>Füllstandsmessung mit Analogsignal am ADS1115 </label></div>\n");
  html.concat("    \n");
  
  html.concat("    <div class='inline'>\n");
  snprintf(buffer, sizeof(buffer), "    <input type='radio' id='sel4' name='selection' value='extern' %s ", (this->Type==EXTERN)?"checked":"");
  html.concat(buffer);
  html.concat("onclick=\"radioselection(['all_2','all_3','extern_1'],['all_1','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','analog_0','analog_1','analog_2','ads1115_0','ads1115_1'])\"/>\n");
  html.concat("    <label for='sel4'>Füllstandsmessung mit externem Signal per MQTT</label></div>\n");
  
  html.concat("  </td>\n");
  html.concat("</tr>\n");

  server->sendContent(html.c_str()); html = "";
  
  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='all_1'>\n", (this->Type==NONE||this->Type==EXTERN?"hide":""));
  html.concat(buffer);
  html.concat("<td>Messintervall</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='254' name='measurecycle' type='number' value='%d'/></td>\n", this->measurecycle);
  html.concat(buffer);
  html.concat("</tr>\n");

  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='hcsr04_1'>\n", (this->Type==HCSR04?"":"hide"));
  html.concat(buffer);
  html.concat("<td>Abstand Sensor min (in cm)</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='254' name='measureDistMin' type='number' value='%d'/></td>\n", this->measureDistMin);
  html.concat(buffer);
  html.concat("</tr>\n");

  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='hcsr04_2'>\n", (this->Type==HCSR04?"":"hide"));
  html.concat(buffer);
  html.concat("<td>Abstand Sensor max (in cm)</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='254' name='measureDistMax' type='number' value='%d'/></td>\n", this->measureDistMax);
  html.concat(buffer);
  html.concat("</tr>\n");

  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='hcsr04_3'>\n", (this->Type==HCSR04?"":"hide"));
  html.concat(buffer);
  html.concat("<td>Pin HC-SR04 Trigger</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='15' id='GpioPin_1' name='pinhcsr04trigger' type='number' value='%d'/></td>\n", this->pinTrigger + 200);
  html.concat(buffer);
  html.concat("</tr>\n");

  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='hcsr04_4'>\n", (this->Type==HCSR04?"":"hide"));
  html.concat(buffer);
  html.concat("<td>Pin HC-SR04 Echo</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='15' id='GpioPin_2' name='pinhcsr04echo' type='number' value='%d'/></td>\n", this->pinEcho + 200);
  html.concat(buffer);
  html.concat("</tr>\n");

  server->sendContent(html.c_str()); html = "";
  
  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='analog_0'>\n", (this->Type==ONBOARD_ANALOG?"":"hide"));
  html.concat(buffer);
  html.concat("<td>GPIO an welchem das Signal anliegt</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' size='15' id='AnalogPin_1' name='pinanalog' type='number' value='%d'/></td>\n", this->pinAnalog + 200);
  html.concat(buffer);
  html.concat("</tr>\n");

  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='ads1115_0'>\n", (this->Type==ADS1115?"":"hide"));
  html.concat(buffer);
  html.concat("<td>i2c Adresse des ADS1115</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input maxlength='2'  name='ads1115_i2c' type='text' value='%02x'/></td>\n", this->ads1115_i2c);
  html.concat(buffer);
  html.concat("</tr>\n");
  
  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='ads1115_1'>\n", (this->Type==ADS1115?"":"hide"));
  html.concat(buffer);
  html.concat("<td>Portnummer am ADS1115 bei dem das Signal anliegt</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='4' size='15'  name='ads1115_port' type='number' value='%d'/></td>\n", this->ads1115_port);
  html.concat(buffer);
  html.concat("</tr>\n");
  
  #ifdef ESP32
    uint16_t maxAnalogRaw = 4096;
  #elif ESP8266
    uint16_t maxAnalogRaw = 1024;
  #endif

  server->sendContent(html.c_str()); html = "";
  
  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='analog_1'>\n", (this->Type==ONBOARD_ANALOG || this->Type==ADS1115?"":"hide"));
  html.concat(buffer);
  html.concat("<td>Kalibrierung: 0% entspricht RAW Wert</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' size='5' name='measureDistMin' type='number' value='%d'/></td>\n", this->measureDistMin);
  html.concat(buffer);
  html.concat("</tr>\n");

  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='analog_2'>\n", (this->Type==ONBOARD_ANALOG || this->Type==ADS1115?"":"hide"));
  html.concat(buffer);
  html.concat("<td>Kalibrierung: 100% entspricht RAW Wert</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='%d' name='measureDistMax' type='number' value='%d'/></td>\n", maxAnalogRaw, this->measureDistMax);
  html.concat(buffer);
  html.concat("</tr>\n");

  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='extern_1'>\n", (this->Type==EXTERN?"":"hide"));
  html.concat(buffer);
  html.concat("<td>MQTT-Topic des externen Sensors (Füllstand in %)</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input size='30' name='externalSensor' type='text' value='%s'/></td>\n", this->externalSensor.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");
  
  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='all_2'>\n", (this->Type==NONE?"hide":""));
  html.concat(buffer);
  html.concat("<td >Sensor Treshold Min für 3WegeVentil</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='254' name='treshold_min' type='number' value='%d'/></td>\n", this->threshold_min);
  html.concat(buffer);
  html.concat("</tr>\n");

  snprintf(buffer, sizeof(buffer), "<tr class='%s' id='all_3'>\n", (this->Type==NONE?"hide":""));
  html.concat(buffer);
  html.concat("<td>Sensor Treshold Max für 3WegeVentil</td>\n");
  snprintf(buffer, sizeof(buffer), "<td><input min='0' max='254' name='treshold_max' type='number' value='%d'/></td>\n", this->threshold_max);
  html.concat(buffer);
  html.concat("</tr>\n");
  html.concat("<tr>\n");

  html.concat("</tbody>\n");
  html.concat("</table>\n");
  html.concat("</form>\n\n<br />\n");
  html.concat("<form id='jsonform' action='StoreSensorConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html.concat("  <input type='text' id='json' name='json' />\n");
  html.concat("  <input type='submit' value='Speichern' />\n");
  html.concat("</form>\n\n");
  
  server->sendContent(html.c_str()); html = "";
}
