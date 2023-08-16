#include "BaseConfig.h"

BaseConfig::BaseConfig(): 
  mqtt_server ("test.mosquitto.org"),
  mqtt_port(1883),
  mqtt_root("PumpControl"),
  mqtt_basepath("home/"),
  mqtt_UseRandomClientID(true),
  keepalive(0),
  debuglevel(3),
  pin_sda(5),
  pin_scl(4),
  pin_1wire(0),
  enable_oled(false),
  oled_type(0),
  enable_1wire(false),
  i2caddress_oled(60), //0x3C;
  enable_3wege(false),
  ventil3wege_port(0),
  max_parallel(0),
  enable_autoupdate(false),
  autoupdate_stage((stage_t)PROD)
  {
  
  ESPUpdate = new updater;
  
  LoadJsonConfig();
}

void BaseConfig::StoreJsonConfig(String* json) {
  File configFile = LittleFS.open("/BaseConfig.json", "w");
  if (!configFile) {
    if (this->GetDebugLevel() >=0) {Serial.println("failed to open BaseConfig.json file for writing");}
  } else {  
    
    if (!configFile.print(*json)) {
        if (this->GetDebugLevel() >=0) {Serial.println(F("Failed writing BaseConfig.json to file"));}
    }

    configFile.close();
  
    LoadJsonConfig();
  }
}

void BaseConfig::LoadJsonConfig() {
  if (LittleFS.exists("/BaseConfig.json")) {
    //file exists, reading and loading
    Serial.println(F("reading BaseConfig.json file"));
    File configFile = LittleFS.open("/BaseConfig.json", "r");
    if (configFile) {
      if (this->GetDebugLevel() >=3) Serial.println(F("BaseConfig.json is now open"));
      ReadBufferingStream stream{configFile, 64};
      stream.find("\"data\":[");
      do {

        DynamicJsonDocument elem(512);
        DeserializationError error = deserializeJson(elem, stream); 
        if (error) {
          if (this->GetDebugLevel() >=1) {
            Serial.printf("Failed to parse BaseConfig.json data: %s, load default config\n", error.c_str()); 
          } 
        } else {
          // Print the result
          if (this->GetDebugLevel() >=5) {Serial.println(F("parsing partial JSON of BaseConfig.json ok")); }
          if (this->GetDebugLevel() >=5) {serializeJsonPretty(elem, Serial);} 
          
          if (elem.containsKey("mqttroot"))         { this->mqtt_root = elem["mqttroot"].as<String>();}
          if (elem.containsKey("mqttserver"))       { this->mqtt_server = elem["mqttserver"].as<String>();}
          if (elem.containsKey("mqttport"))         { this->mqtt_port = elem["mqttport"].as<int>();}
          if (elem.containsKey("mqttuser"))         { this->mqtt_username = elem["mqttuser"].as<String>();}
          if (elem.containsKey("mqttpass"))         { this->mqtt_password = elem["mqttpass"].as<String>();}
          if (elem.containsKey("mqttbasepath"))     { this->mqtt_basepath = elem["mqttbasepath"].as<String>();}
          if (elem.containsKey("sel_UseRandomClientID")){ if (strcmp(elem["sel_UseRandomClientID"], "none")==0) { this->mqtt_UseRandomClientID=false;} else {this->mqtt_UseRandomClientID=true;}}
          if (elem.containsKey("keepalive"))        { if (elem["keepalive"].as<int>() == 0) { this->keepalive = 0;} else { this->keepalive = _max(elem["keepalive"].as<int>(), 10);}}
          if (elem.containsKey("debuglevel"))       { this->debuglevel = _max(elem["debuglevel"].as<int>(), 0);}
          if (elem.containsKey("pinsda"))           { this->pin_sda = (elem["pinsda"].as<int>()) - 200;}
          if (elem.containsKey("pinscl"))           { this->pin_scl = (elem["pinscl"].as<int>()) - 200;}
          if (elem.containsKey("pin1wire"))         { this->pin_1wire = (elem["pin1wire"].as<int>()) - 200;}
          if (elem.containsKey("sel_oled"))         { if (strcmp(elem["sel_oled"], "none")==0) { this->enable_oled=false;} else {this->enable_oled=true;}}
          if (elem.containsKey("sel_1wire"))        { if (strcmp(elem["sel_1wire"], "none")==0) { this->enable_1wire=false;} else {this->enable_1wire=true;}}
          if (elem.containsKey("sel_3wege"))        { if (strcmp(elem["sel_3wege"], "none")==0) { this->enable_3wege=false;} else {this->enable_3wege=true;}}
          if (elem.containsKey("sel_update"))       { if (strcmp(elem["sel_update"], "manu")==0) { this->enable_autoupdate=false;} else {this->enable_autoupdate=true;}}
          if (elem.containsKey("autoupdate_url"))   { this->autoupdate_url = elem["autoupdate_url"].as<String>(); }                   
          if (elem.containsKey("autoupdate_stage")) { if (elem["autoupdate_stage"] == "PROD") { this->autoupdate_stage = (stage_t)PROD; }
                                                      if (elem["autoupdate_stage"] == "PRE")  { this->autoupdate_stage = (stage_t)PRE; }
                                                      if (elem["autoupdate_stage"] == "DEV")  { this->autoupdate_stage = (stage_t)DEV; }             
                                                    }
          if (elem.containsKey("i2coled"))          { this->i2caddress_oled = strtoul(elem["i2coled"], NULL, 16);} // hex convert to dec    
          if (elem.containsKey("oled_type"))        { this->oled_type = elem["oled_type"].as<int>();} 
          if (elem.containsKey("ventil3wege_port")) { this->ventil3wege_port = elem["ventil3wege_port"].as<int>();}
        }
      } while (stream.findUntil(",","]"));
    } else {
      Serial.println("cannot open existing BaseConfig.json config File, load default BaseConfig"); // -> constructor
    }
  } else {
    Serial.println("BaseConfig.json config File not exists, load default BaseConfig");
  }

  if (!this->autoupdate_url || this->autoupdate_url.length() < 10 ) {
    #ifdef ESP8266
      this->autoupdate_url="http://tfa-releases.s3-website.eu-central-1.amazonaws.com/ESP8266_PumpControl/releases_ESP8266.json";
    #elif ESP32
      this->autoupdate_url="http://tfa-releases.s3-website.eu-central-1.amazonaws.com/ESP8266_PumpControl/releases_ESP32.json";
    #endif
  }

  ESPUpdate->setAutoMode(this->enable_autoupdate);
  ESPUpdate->setIndexJson(this->autoupdate_url);
  ESPUpdate->setStage(this->autoupdate_stage);
  ESPUpdate->SetDebugLevel(this->debuglevel);

  // Data Cleaning
  if(this->mqtt_basepath.endsWith("/")) {
    this->mqtt_basepath = this->mqtt_basepath.substring(0, this->mqtt_basepath.length()-1); 
  }
}

String BaseConfig::GetReleaseName() {
  return ESPUpdate->GetReleaseName();
}

void BaseConfig::InstallRelease(uint32_t ReleaseNumber) {
  ESPUpdate->InstallRelease(ReleaseNumber);
}

void BaseConfig::RefreshReleases() {
  ESPUpdate->RefreshReleases();
}

void BaseConfig::loop() {
  ESPUpdate->loop();  
}

/****************************************************
 * handle one single Row for Webrequest to support chunkedResponse
 * See BaseConfig.h -> #define WEB()
****************************************************/
void BaseConfig::handleOneHtmlRow(const size_t& curRow, 
       std::shared_ptr<uint16_t> processedRows, 
       size_t& len, 
       const size_t& maxLen, 
       uint8_t* buffer, 
       const char* str, 
       ...) {

  //Serial.printf("Start XX mit len: %d\n", len);
  size_t tempLen = 0;
  va_list argptr;
  va_start(argptr, str);

  char buf[200] = {0};
  memset(buf, 0, sizeof(buf));

  if (curRow == (*processedRows)) {
    tempLen = vsprintf(buf, str, argptr);
    //Serial.printf("in XX, tempLen: %d\n", tempLen);

    if ((len + tempLen) < maxLen) {
      //Serial.printf("  row ok: %d \n", curRow);
      strcpy((char*)buffer + len, buf);
      (*processedRows)++; 
      len += tempLen;
    }
  } 
  va_end(argptr);
  //Serial.printf("Ende XX mit len: %d\n", len);
}

/* https://cpp4arduino.com/2018/11/06/what-is-heap-fragmentation.html*/
size_t BaseConfig::getFragmentation() {
  return 100 - ESP_GetMaxFreeAvailableBlock() * 100 / ESP.getFreeHeap();
}

void BaseConfig::GetWebContent(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen) {
  WEB("<form id='DataForm'>\n");
  WEB("<table id='maintable' class='editorDemoTable'>\n");
  WEB("<thead>\n");
  WEB("<tr>\n");
  WEB("<td style='width: 250px;'>Name</td>\n");
  WEB("<td style='width: 200px;'>Wert</td>\n");
  WEB("</tr>\n");
  WEB("</thead>\n");
  WEB("<tbody>\n");

  WEB("<tr>\n");
  WEB("<td>Device Name</td>\n");
  WEB("<td><input size='30' maxlength='40' name='mqttroot' type='text' value='%s'/></td>\n", this->mqtt_root.c_str());
  WEB("</tr>\n");
  
  WEB("<tr>\n");
  WEB("<td>MQTT Server IP</td>\n");
  WEB("<td><input size='30' name='mqttserver' type='text' value='%s'/></td>\n", this->mqtt_server.c_str());
  WEB("</tr>\n");
  
  WEB("<tr>\n");
  WEB("<td>MQTT Server Port</td>\n");
  WEB("<td><input maxlength='5' name='mqttport' type='text' value='%d'/></td>\n", this->mqtt_port);
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("<td>MQTT Authentification: Username (optional)</td>\n");
  WEB("<td><input size='30' name='mqttuser' type='text' value='%s'/></td>\n", this->mqtt_username.c_str());
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("<td>MQTT Authentification: Password (optional)</td>\n");
  WEB("<td><input size='30' name='mqttpass' type='text' value='%s'/></td>\n", this->mqtt_password.c_str());
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("<td>MQTT Topic Base Path (example: home/inverter)</td>\n");
  WEB("<td><input size='30' maxlength='40' name='mqttbasepath' type='text' value='%s'/></td>\n", this->mqtt_basepath.c_str());
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td colspan='2'>\n");
  
  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='sel_URCID1' name='sel_UseRandomClientID' value='none' %s />", (this->mqtt_UseRandomClientID)?"":"checked");
  WEB("<label for='sel_URCID1'>benutze statische MQTT ClientID</label></div>\n");
  
  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='sel_URCID2' name='sel_UseRandomClientID' value='yes' %s />", (this->mqtt_UseRandomClientID)?"checked":"");
  WEB("<label for='sel_URCID2'>benutze dynamische MQTT ClientID</label></div>\n");
    
  WEB("  </td>\n");
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("<td>Senden einer KeepAlive Message via MQTT (in sek > 10, 0=disabled)</td>\n");
  WEB("<td><input min='10' max='65000' name='keepalive' type='number' value='%d'/></td>\n", this->keepalive);
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("<td>DebugMode (0 [off] .. 5 [max]</td>\n");
  WEB("<td><input min='0' max='5' name='debuglevel' type='number' value='%d'/></td>\n", this->debuglevel);
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("<td>Pin i2c SDA</td>\n");
  WEB("<td><input min='0' max='15' id='GpioPin_0' name='pinsda' type='number' value='%d'/></td>\n", this->pin_sda +200 );
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("<td>Pin i2c SCL</td>\n");
  WEB("<td><input min='0' max='15' id='GpioPin_1' name='pinscl' type='number' value='%d'/></td>\n", this->pin_scl +200 );
  WEB("</tr>\n");

#ifdef USE_ONEWIRE
  WEB("<tr>\n");
  WEB("  <td colspan='2'>\n");

  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='ow1' name='sel_1wire' value='none' %s onclick=\"radioselection([''],['onewire_0'])\"/>", (this->enable_1wire)?"":"checked");
  WEB("<label for='ow1'>kein OneWire</label></div>\n");
    
  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='ow2' name='sel_1wire' value='1wire' %s onclick=\"radioselection(['onewire_0'],[''])\"/>", (this->enable_1wire)?"checked":"");
  WEB("<label for='ow2'>OneWire Aktiv</label></div>\n");
  
  WEB("  </td>\n");
  WEB("</tr>\n");

  WEB("<tr class='%s' id='onewire_0'>\n", (this->enable_1wire?"":"hide"));
  WEB("<td>Welcher Pin nutzt 1Wire?</td>\n");
  WEB("<td><input min='0' max='15' id='GpioPin_3' name='pin1wire' type='number' value='%d'/></td>\n", this->pin_1wire +200 );
  WEB("</tr>\n");
#endif

#ifdef USE_OLED
  WEB("<tr>\n");
  WEB("  <td colspan='2'>\n");

  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='sel_oled1' name='sel_oled' value='none' %s onclick=\"radioselection([''],['oled_0','oled_1'])\"/>", (this->enable_oled)?"":"checked");
  WEB("<label for='sel_oled1'>kein OLED</label></div>\n");
  
  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='sel_oled2' name='sel_oled' value='oled' %s onclick=\"radioselection(['oled_0','oled_1'],[''])\"/>", (this->enable_oled)?"checked":"");
  WEB("<label for='sel_oled2'>mit OLED</label></div>\n");
    
  WEB("  </td>\n");
  WEB("</tr>\n");

  WEB("<tr class='%s' id='oled_0'>\n", (this->enable_oled?"":"hide"));
  WEB("<td>i2c Adresse OLED</td>\n");
  WEB("<td><input maxlength='2' name='i2coled' type='text' value='%02x'/></td>\n", this->i2caddress_oled);
  WEB("</tr>\n");

  WEB("<tr class='%s' id='oled_1'>\n", (this->enable_oled?"":"hide"));
  WEB("<td>OLED Typ</td>\n");
  WEB("<td><select name='oled_type' size='1'> \n");
  WEB("  <option %s value='0'/>OLED SSD1306</option>\n", (this->oled_type==0?"selected":""));
  WEB("  <option %s value='1'/>OLED SH1106</option>\n", (this->oled_type==1?"selected":""));
  WEB("</select></td>\n");
  WEB("</tr>\n");
#endif

  WEB("<tr>\n");
  WEB("  <td colspan='2'>\n");
  
  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='sel_3wege_0' name='sel_3wege' value='none' %s onclick=\"radioselection([''],['3wege_0'])\"/>", (this->enable_3wege)?"":"checked");
  WEB("<label for='sel_3wege_0'>kein Trinkwasser Bypass</label></div>\n");
  
  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='sel_3wege_1' name='sel_3wege' value='3wege' %s onclick=\"radioselection(['3wege_0'],[''])\"/>", (this->enable_3wege)?"checked":"");
  WEB("<label for='sel_3wege_1'>mit Trinkwasser ByPass Ventil</label></div>\n");
    
  WEB("  </td>\n");
  WEB("</tr>\n");

  WEB("<tr class='%s' id='3wege_0'>\n", (this->enable_3wege?"":"hide"));
  WEB("<td>3WegeVentil Trinkwasser Bypass</td>\n");
  WEB("<td><input min='0' max='254' id='ConfiguredPorts_0' name='ventil3wege_port' type='number' value='%d'/></td>\n", this->ventil3wege_port);
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td colspan='2'>\n");
  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='sel_update_0' name='sel_update' value='auto' %s onclick=\"radioselection(['update_0'],['update_1'])\"/>", (this->enable_autoupdate)?"checked":"");
  WEB("<label for='sel_update_0'>Automatisches Update</label></div>\n");
  WEB("    <div class='inline'>");
  WEB("<input type='radio' id='sel_update_1' name='sel_update' value='manu' %s onclick=\"radioselection(['update_1'],['update_0'])\"/>", (this->enable_autoupdate)?"":"checked");
  WEB("<label for='sel_update_1'>Manuelles Update</label></div>\n");
  WEB("  </td>\n");
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("<td>Update URL</td>\n");
  WEB("<td><input size='30' name='autoupdate_url' type='text' value='%s'/></td>\n", this->autoupdate_url.c_str());
  WEB("</tr>\n");

  WEB("<tr class='%s' id='update_0'>\n", (this->enable_autoupdate?"":"hide"));
  WEB("<td>Auswahl der Stage</td>\n");
  WEB("<td>\n");
  WEB("  <select id='autoupdate_stage' name='autoupdate_stage'>\n");
  WEB("  <option value='%s' %s>%s</option>\n", "PROD", (this->autoupdate_stage==PROD?"selected":""), "Production");
  WEB("  <option value='%s' %s>%s</option>\n", "PRE", (this->autoupdate_stage==PRE?"selected":""), "PreLive/QS");
  WEB("  <option value='%s' %s>%s</option>\n", "DEV", (this->autoupdate_stage==DEV?"selected":""), "Development");
  WEB("  </select>\n");
  WEB("</td>\n");
  WEB("</tr>\n");
  
  WEB("<tr class='%s' id='update_1'>\n", (this->enable_autoupdate?"hide":""));
  WEB("<td>verfügbare Releases\n");
  WEB("<a href='#' onclick='RefreshReleases()' class='ButtonRefresh' title='Die JSON Liste neu laden'>&#8634;</a>");
  WEB("</td><td>\n");

  WEB("  <select id='releases' name='releases'>\n");
  std::vector<release_t>* rel = ESPUpdate->GetReleases();
  
  WEB("  <optgroup label='Produktiv'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "PROD") {
      WEB("<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
    }
  }
  WEB("  </optgroup>\n");

  WEB("  <optgroup label='PreLive'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "PRE") {
      WEB("<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
    }
  }
  WEB("  </optgroup>\n");

  WEB("  <optgroup label='Development'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "DEV") {
      WEB("<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
    }
  }
  WEB("  </optgroup>\n");

  WEB("  </select>\n");
  WEB("<input type='button' class='button' onclick='InstallRelease()' value='Install' />");
  WEB("</td>\n");
  WEB("</tr>\n");
 
  WEB("  </td>\n");
  WEB("</tr>\n");
  
  WEB("</tbody>\n");
  WEB("</table>\n");


  WEB("</form>\n\n<br />\n");
  WEB("<form id='jsonform' action='StoreBaseConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\", \".*\")'>\n");
  WEB("  <input type='text' id='json' name='json' />\n");
  WEB("  <input type='submit' value='Speichern' />\n");
  WEB("</form>\n\n");
}
