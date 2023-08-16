#include "sensor.h"

sensor::sensor() : 
  Type(NONE), 
  measureDistMin(0), 
  measureDistMax(0), 
  measurecycle(10), 
  level(0), 
  raw(0),
  threshold_min(26),
  threshold_max(30) {
  
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
  setSensorType(EXTERN);
  this->measurecycle = 10;
  mqtt->Subscribe(externalSensor, MyMQTT::SENSOR);
}

#ifdef USE_OLED
void sensor::SetOled(OLED* oled) {
  this->oled = oled;
}
#endif

void sensor::setSensorType(sensorType_t t) {
  this->Type = t;
}

void sensor::SetLvl(uint8_t lvl) {
  this->level = lvl;
  #ifdef USE_OLED
    if(this->oled) this->oled->SetLevel(this->level);
  #endif
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

#ifdef USE_ADS1115
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
#endif

void sensor::loop() {
  if (millis() - this->previousMillis > this->measurecycle*1000) {
    this->previousMillis = millis();
   
    if (this->Type == ONBOARD_ANALOG) {loop_analog();}
   
    if (this->Type == HCSR04) {loop_hcsr04();}

    #ifdef USE_ADS1115
      if (this->Type == ADS1115) {loop_ads1115();}
    #endif

    if (this->Type != NONE && this->level !=0 && Config->Enabled3Wege()) {
      if (this->level < this->threshold_min) { VStruct->SetOn(Config->Get3WegePort()); }
      if (this->level > this->threshold_max) { VStruct->SetOff(Config->Get3WegePort()); }
    }
    if (this->Type != NONE && this->Type != EXTERN && mqtt) {
      if (this->raw > 0 )   { mqtt->Publish_Int((const char*)"raw", (int)this->raw, false); }
      if (this->level > 0 ) { mqtt->Publish_Int((const char*)"level", (int)this->level, false); }
    }
    
  #ifdef USE_OLED
    if (this->Type != NONE && this->Type != EXTERN) {
      if(this->oled) this->oled->SetLevel(this->level);
    }
  #endif
  
     if (this->Type != NONE && Config->GetDebugLevel() >=4) {
      Serial.printf("measured sensor raw value: %d \n", this->raw);
     }
  }
}

void sensor::StoreJsonConfig(String* json) {
  File configFile = LittleFS.open("/SensorConfig.json", "w");
  if (!configFile) {
    if (Config->GetDebugLevel() >=0) {Serial.println("failed to open SensorConfig.json file for writing");}
  } else {  
    
    if (!configFile.print(*json)) {
        if (Config->GetDebugLevel() >=0) {Serial.println(F("Failed writing SensorConfig.json to file"));}
    }

    configFile.close();
  
    LoadJsonConfig();
  }
}

void sensor::LoadJsonConfig() {
  mqtt->ClearSubscriptions(MyMQTT::SENSOR);
  
  if (LittleFS.exists("/SensorConfig.json")) {
    //file exists, reading and loading
    Serial.println(F("reading SensorConfig.json file"));
    File configFile = LittleFS.open("/SensorConfig.json", "r");
    if (configFile) {
      if (Config->GetDebugLevel() >=3) Serial.println(F("SensorConfig.json is now open"));
      ReadBufferingStream stream{configFile, 64};
      stream.find("\"data\":[");
      do {

        DynamicJsonDocument elem(512);
        DeserializationError error = deserializeJson(elem, stream); 
        if (error) {
          if (Config->GetDebugLevel() >=1) {
            Serial.printf("Failed to parse SensorConfig.json data: %s, load default config\n", error.c_str()); 
          } 
        } else {
          // Print the result
          if (Config->GetDebugLevel() >=5) {Serial.println(F("parsing partial JSON of SensorConfig.json ok")); }
          if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, Serial);} 
          
          if (elem.containsKey("measurecycle"))         { this->measurecycle = _max(elem["measurecycle"].as<int>(), 10);}
          if (elem.containsKey("measureDistMin"))       { this->measureDistMin = elem["measureDistMin"].as<int>();}
          if (elem.containsKey("measureDistMax"))       { this->measureDistMax = elem["measureDistMax"].as<int>();}
          if (elem.containsKey("pinhcsr04trigger"))     { this->pinTrigger = elem["pinhcsr04trigger"].as<int>() - 200;}
          if (elem.containsKey("pinhcsr04echo"))        { this->pinEcho = elem["pinhcsr04echo"].as<int>() - 200;}
          if (elem.containsKey("pinanalog"))            { this->pinAnalog = elem["pinanalog"].as<int>() - 200;}
          if (elem.containsKey("treshold_min"))         { this->threshold_min = elem["treshold_min"].as<int>();}
          if (elem.containsKey("treshold_max"))         { this->threshold_max = elem["treshold_max"].as<int>();}
          if (elem.containsKey("ads1115_i2c"))          { this->ads1115_i2c = strtoul(elem["ads1115_i2c"].as<String>().c_str(), NULL, 16);} // hex convert to dec 
          if (elem.containsKey("ads1115_port"))         { this->ads1115_port = elem["ads1115_port"].as<int>();}
          if (elem.containsKey("externalSensor"))       { this->externalSensor = elem["externalSensor"].as<String>();}
          
          if(strcmp(elem["selection"].as<String>().c_str(),"analog")==0)        
            { init_analog(this->pinAnalog); }
            else if(strcmp(elem["selection"].as<String>().c_str(),"hcsr04")==0)   { init_hcsr04(this->pinTrigger, this->pinEcho); }
            else if(strcmp(elem["selection"].as<String>().c_str(),"extern")==0)   { init_extern(this->externalSensor); }
            else if(strcmp(elem["selection"].as<String>().c_str(),"none")==0)     { this->Type=NONE; Serial.println(F("No LevelSensor requested")); } 
            
          #ifdef USE_ADS1115  
            else if(strcmp(elem["selection"].as<String>().c_str(),"ads1115")==0)  { init_ads1115(this->ads1115_i2c, this->ads1115_port); }
          #endif
        }
      } while (stream.findUntil(",","]"));
    } else {
      Serial.println("cannot open existing SensorConfig.json config File, load default SensorConfig"); // -> constructor
    }
  } else {
    Serial.println("SensorConfig.json config File not exists, load default SensorConfig");
  }
}

void sensor::GetWebContent(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen) {
  WEB("<form id='DataForm'>\n");
  WEB("<table id='maintable' class='editorDemoTable'>\n");
  WEB("<thead>\n");
  WEB("  <tr>\n");
  WEB("    <td style='width: 250px;'>Name</td>\n");
  WEB("    <td style='width: 200px;'>Wert</td>\n");
  WEB("  </tr>\n");
  WEB("</thead>\n");
  WEB("<tbody>\n");

  WEB("<tr>\n");
  WEB("  <td colspan='2'>\n");
  
  WEB("    <div class='inline'>\n");
  WEB("    <input type='radio' id='sel0' name='selection' value='none' %s ", (this->Type==NONE)?"checked":"");
  WEB("onclick=\"radioselection([''],['all_1','all_2','all_3','analog_0','analog_1','analog_2','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','extern_1','ads1115_0','ads1115_1'])\"/>\n");
  WEB("    <label for='sel0'>keine Füllstandsmessung</label></div>\n");
  WEB("    \n");
  
  WEB("    <div class='inline'>\n");
  WEB("    <input type='radio' id='sel1' name='selection' value='hcsr04' %s ", (this->Type==HCSR04)?"checked":"");
  WEB(" onclick=\"radioselection(['all_1','all_2','all_3','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4'],['analog_0','analog_1','analog_2','extern_1','ads1115_0','ads1115_1'])\"/>\n");
  WEB("    <label for='sel1'>Füllstandsmessung mit Ultraschallsensor HCSR04</label></div>\n");
  WEB("    \n");
  
  WEB("    <div class='inline'>\n");
  WEB("    <input type='radio' id='sel2' name='selection' value='analog' %s ", (this->Type==ONBOARD_ANALOG)?"checked":"");
  WEB("onclick=\"radioselection(['all_1','all_2','all_3','analog_0','analog_1','analog_2'],['hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','extern_1','ads1115_0','ads1115_1'])\"/>\n");
  WEB("    <label for='sel2'>Füllstandsmessung mit Analogsignal am ESP</label></div>\n");
  WEB("    \n");
  
  #ifdef USE_ADS1115
    WEB("    <div class='inline'>\n");
    WEB("    <input type='radio' id='sel3' name='selection' value='ads1115' %s ", (this->Type==ADS1115)?"checked":"");
    WEB("onclick=\"radioselection(['all_1','all_2','all_3','analog_1','analog_2','ads1115_0','ads1115_1'],['hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','extern_1','analog_0'])\"/>\n");
    WEB("    <label for='sel3'>Füllstandsmessung mit Analogsignal am ADS1115 </label></div>\n");
    WEB("    \n");
  #endif

  WEB("    <div class='inline'>\n");
  WEB("    <input type='radio' id='sel4' name='selection' value='extern' %s ", (this->Type==EXTERN)?"checked":"");
  WEB("onclick=\"radioselection(['all_2','all_3','extern_1'],['all_1','hcsr04_1','hcsr04_2','hcsr04_3','hcsr04_4','analog_0','analog_1','analog_2','ads1115_0','ads1115_1'])\"/>\n");
  WEB("    <label for='sel4'>Füllstandsmessung mit externem Signal per MQTT</label></div>\n");
  
  WEB("  </td>\n");
  WEB("</tr>\n");

  WEB("<tr class='%s' id='all_1'>\n", (this->Type==NONE||this->Type==EXTERN?"hide":""));
  WEB("<td>Messintervall</td>\n");
  WEB("<td><input min='0' max='254' name='measurecycle' type='number' value='%d'/></td>\n", this->measurecycle);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='hcsr04_1'>\n", (this->Type==HCSR04?"":"hide"));
  WEB("<td>Abstand Sensor min (in cm)</td>\n");
  WEB("<td><input min='0' max='254' name='measureDistMin' type='number' value='%d'/></td>\n", this->measureDistMin);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='hcsr04_2'>\n", (this->Type==HCSR04?"":"hide"));
  WEB("<td>Abstand Sensor max (in cm)</td>\n");
  WEB("<td><input min='0' max='254' name='measureDistMax' type='number' value='%d'/></td>\n", this->measureDistMax);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='hcsr04_3'>\n", (this->Type==HCSR04?"":"hide"));
  WEB("<td>Pin HC-SR04 Trigger</td>\n");
  WEB("<td><input min='0' max='15' id='GpioPin_1' name='pinhcsr04trigger' type='number' value='%d'/></td>\n", this->pinTrigger + 200);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='hcsr04_4'>\n", (this->Type==HCSR04?"":"hide"));
  WEB("<td>Pin HC-SR04 Echo</td>\n");
  WEB("<td><input min='0' max='15' id='GpioPin_2' name='pinhcsr04echo' type='number' value='%d'/></td>\n", this->pinEcho + 200);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='analog_0'>\n", (this->Type==ONBOARD_ANALOG?"":"hide"));
  WEB("<td>GPIO an welchem das Signal anliegt</td>\n");
  WEB("<td><input min='0' size='15' id='AnalogPin_1' name='pinanalog' type='number' value='%d'/></td>\n", this->pinAnalog + 200);
  WEB("</tr>\n");

  #ifdef USE_ADS1115
    WEB("<tr class='%s' id='ads1115_0'>\n", (this->Type==ADS1115?"":"hide"));
    WEB("<td>i2c Adresse des ADS1115</td>\n");
    WEB("<td><input maxlength='2'  name='ads1115_i2c' type='text' value='%02x'/></td>\n", this->ads1115_i2c);
    WEB("</tr>\n");
    
    WEB("<tr class='%s' id='ads1115_1'>\n", (this->Type==ADS1115?"":"hide"));
    WEB("<td>Portnummer am ADS1115 bei dem das Signal anliegt</td>\n");
    WEB("<td><input min='0' max='4' size='15'  name='ads1115_port' type='number' value='%d'/></td>\n", this->ads1115_port);
    WEB("</tr>\n");
  #endif
  
  #ifdef ESP32
    uint16_t maxAnalogRaw = 4096;
  #elif ESP8266
    uint16_t maxAnalogRaw = 1024;
  #endif

  WEB("<tr class='%s' id='analog_1'>\n", (this->Type==ONBOARD_ANALOG || this->Type==ADS1115?"":"hide"));
  WEB("<td>Kalibrierung: 0% entspricht RAW Wert</td>\n");
  WEB("<td><input min='0' size='5' name='measureDistMin' type='number' value='%d'/></td>\n", this->measureDistMin);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='analog_2'>\n", (this->Type==ONBOARD_ANALOG || this->Type==ADS1115?"":"hide"));
  WEB("<td>Kalibrierung: 100% entspricht RAW Wert</td>\n");
  WEB("<td><input min='0' max='%d' name='measureDistMax' type='number' value='%d'/></td>\n", maxAnalogRaw, this->measureDistMax);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='extern_1'>\n", (this->Type==EXTERN?"":"hide"));
  WEB("<td>MQTT-Topic des externen Sensors (Füllstand in %)</td>\n");
  WEB("<td><input size='30' name='externalSensor' type='text' value='%s'/></td>\n", this->externalSensor.c_str());
  WEB("</tr>\n");
  
  WEB("<tr class='%s' id='all_2'>\n", (this->Type==NONE?"hide":""));
  WEB("<td >Sensor Treshold Min für 3WegeVentil</td>\n");
  WEB("<td><input min='0' max='254' name='treshold_min' type='number' value='%d'/></td>\n", this->threshold_min);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='all_3'>\n", (this->Type==NONE?"hide":""));
  WEB("<td>Sensor Treshold Max für 3WegeVentil</td>\n");
  WEB("<td><input min='0' max='254' name='treshold_max' type='number' value='%d'/></td>\n", this->threshold_max);
  WEB("</tr>\n");
  WEB("<tr>\n");

  WEB("</tbody>\n");
  WEB("</table>\n");
  WEB("</form>\n\n<br />\n");
  WEB("<form id='jsonform' action='StoreSensorConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\", \".*\")'>\n");
  WEB("  <input type='text' id='json' name='json' />\n");
  WEB("  <input type='submit' value='Speichern' />\n");
  WEB("</form>\n\n");
}
