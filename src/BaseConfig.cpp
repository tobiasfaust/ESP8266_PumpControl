#include "BaseConfig.h"

BaseConfig::BaseConfig(): debuglevel(3) {
  ESPUpdate = new updater;
  
  LoadJsonConfig();
}

void BaseConfig::StoreJsonConfig(String* json) {

  StaticJsonDocument<1024> doc;
  deserializeJson(doc, *json);
  JsonObject root = doc.as<JsonObject>();

  if (!root.isNull()) {
    File configFile = LittleFS.open("/BaseConfig.json", "w");
    if (!configFile) {
      if (this->GetDebugLevel() >=0) {Serial.println("failed to open BaseConfig.json file for writing");}
    } else {  
      if (this->GetDebugLevel() >= 3) { 
        serializeJsonPretty(doc, Serial); 
        Serial.println();
      }

      if (serializeJson(doc, configFile) == 0) {
        if (this->GetDebugLevel() >=0) {Serial.println(F("Failed to write to file"));}
      }
      configFile.close();
  
      LoadJsonConfig();
    }
  }
}

void BaseConfig::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  if (LittleFS.exists("/BaseConfig.json")) {
    //file exists, reading and loading
    Serial.println("reading BaseConfig.json file");
    File configFile = LittleFS.open("/BaseConfig.json", "r");
    if (configFile) {
      Serial.println("opened BaseConfig.json file");
      
      //StaticJsonDocument<1024> json; // TODO Use computed size??
      DynamicJsonDocument json(1024);
      DeserializationError error = deserializeJson(json, configFile);

      if (!error) {
        if (this->GetDebugLevel() >= 3) {
          serializeJsonPretty(json, Serial); 
          Serial.println();
        }

        if (json.containsKey("mqttroot"))         { this->mqtt_root = json["mqttroot"].as<String>();} else {this->mqtt_root = "PumpControl";}
        if (json.containsKey("mqttserver"))       { this->mqtt_server = json["mqttserver"].as<String>();} else {this->mqtt_server = "test.mosquitto.org";}
        if (json.containsKey("mqttport"))         { this->mqtt_port = json["mqttport"].as<int>();} else {this->mqtt_port = 1883;}
        if (json.containsKey("mqttuser"))         { this->mqtt_username = json["mqttuser"].as<String>();} else {this->mqtt_username = "";}
        if (json.containsKey("mqttpass"))         { this->mqtt_password = json["mqttpass"].as<String>();} else {this->mqtt_password = "";}
        if (json.containsKey("mqttbasepath"))     { this->mqtt_basepath = json["mqttbasepath"].as<String>();} else {this->mqtt_basepath = "home/";}
        if (json.containsKey("sel_UseRandomClientID")){ if (strcmp(json["sel_UseRandomClientID"], "none")==0) { this->mqtt_UseRandomClientID=false;} else {this->mqtt_UseRandomClientID=true;}} else {this->mqtt_UseRandomClientID = true;}
        if (json.containsKey("keepalive"))        { if (json["keepalive"].as<int>() == 0) { this->keepalive = 0;} else { this->keepalive = _max(json["keepalive"].as<int>(), 10);}} else {this->keepalive = 0; }
        if (json.containsKey("debuglevel"))       { this->debuglevel = _max(json["debuglevel"].as<int>(), 0);} else {this->debuglevel = 0; }
        if (json.containsKey("pinsda"))           { this->pin_sda = (json["pinsda"].as<int>()) - 200;} else {this->pin_sda = 5;}
        if (json.containsKey("pinscl"))           { this->pin_scl = (json["pinscl"].as<int>()) - 200;} else {this->pin_scl = 4;}
        if (json.containsKey("pin1wire"))         { this->pin_1wire = (json["pin1wire"].as<int>()) - 200;} else {this->pin_1wire = 0;}
        if (json.containsKey("sel_oled"))         { if (strcmp(json["sel_oled"], "none")==0) { this->enable_oled=false;} else {this->enable_oled=true;}} else {this->enable_oled = false;}
        if (json.containsKey("sel_1wire"))        { if (strcmp(json["sel_1wire"], "none")==0) { this->enable_1wire=false;} else {this->enable_1wire=true;}} else {this->enable_1wire = false;}
        if (json.containsKey("sel_3wege"))        { if (strcmp(json["sel_3wege"], "none")==0) { this->enable_3wege=false;} else {this->enable_3wege=true;}} else {this->enable_3wege = false;}
        if (json.containsKey("sel_update"))       { if (strcmp(json["sel_update"], "manu")==0) { this->enable_autoupdate=false;} else {this->enable_autoupdate=true;}} else {this->enable_autoupdate = false;}
        if (json.containsKey("autoupdate_url"))   { this->autoupdate_url = json["autoupdate_url"].as<String>(); }                   
        if (json.containsKey("autoupdate_stage")) { if (json["autoupdate_stage"] == "PROD") { this->autoupdate_stage = (stage_t)PROD; }
                                                    if (json["autoupdate_stage"] == "PRE")  { this->autoupdate_stage = (stage_t)PRE; }
                                                    if (json["autoupdate_stage"] == "DEV")  { this->autoupdate_stage = (stage_t)DEV; }             
                                                  }
        if (json.containsKey("i2coled"))          { this->i2caddress_oled = strtoul(json["i2coled"], NULL, 16);} // hex convert to dec    
        if (json.containsKey("oled_type"))        { this->oled_type = json["oled_type"].as<int>();} else {this->oled_type = 0;}; 
        if (json.containsKey("ventil3wege_port")) { this->ventil3wege_port = json["ventil3wege_port"].as<int>();}
      } else {
        Serial.printf("Cannot parse the BaseConfig.json: %s\n", error.c_str());
        loadDefaultConfig = true;
      }
    } else {
      Serial.println("cannot open existing BaseConfig.json config File, load default BaseConfig");
      loadDefaultConfig = true;
    }
  } else {
    Serial.println("BaseConfig.json config File not exists, load default BaseConfig");
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    this->mqtt_server = "test.mosquitto.org";
    this->mqtt_port  = 1883;
    this->mqtt_root = "PumpControl";
    this->mqtt_basepath = "home/";
    this->mqtt_UseRandomClientID = true;
    this->keepalive = 0;
    this->debuglevel = 0;
    this->pin_sda = 5;
    this->pin_scl = 4;
    this->pin_1wire = 0;
    this->enable_oled = false;
    this->oled_type = 0;
    this->enable_1wire = false;
    this->i2caddress_oled = 60; //0x3C;
    this->enable_3wege = false;
    this->ventil3wege_port = 0;
    this->max_parallel = 0;
    this->enable_autoupdate = false;
    this->autoupdate_stage = (stage_t)PROD;

    loadDefaultConfig = false; //set back
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

void BaseConfig::GetWebContent(AsyncResponseStream *response) {
  response->println("<form id='DataForm'>\n");
  response->println("<table id='maintable' class='editorDemoTable'>\n");
  response->println("<thead>\n");
  response->println("<tr>\n");
  response->println("<td style='width: 250px;'>Name</td>\n");
  response->println("<td style='width: 200px;'>Wert</td>\n");
  response->println("</tr>\n");
  response->println("</thead>\n");
  response->println("<tbody>\n");

  response->println("<tr>\n");
  response->println("<td>Device Name</td>\n");
  response->printf("<td><input size='30' maxlength='40' name='mqttroot' type='text' value='%s'/></td>\n", this->mqtt_root.c_str());
  response->println("</tr>\n");
  
  response->println("<tr>\n");
  response->println("<td>MQTT Server IP</td>\n");
  response->printf("<td><input size='30' name='mqttserver' type='text' value='%s'/></td>\n", this->mqtt_server.c_str());
  response->println("</tr>\n");
  
  response->println("<tr>\n");
  response->println("<td>MQTT Server Port</td>\n");
  response->printf("<td><input maxlength='5' name='mqttport' type='text' value='%d'/></td>\n", this->mqtt_port);
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("<td>MQTT Authentification: Username (optional)</td>\n");
  response->printf("<td><input size='30' name='mqttuser' type='text' value='%s'/></td>\n", this->mqtt_username.c_str());
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("<td>MQTT Authentification: Password (optional)</td>\n");
  response->printf("<td><input size='30' name='mqttpass' type='text' value='%s'/></td>\n", this->mqtt_password.c_str());
  response->println("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>MQTT Topic Base Path (example: home/inverter)</td>\n");
  response->printf("<td><input size='30' maxlength='40' name='mqttbasepath' type='text' value='%s'/></td>\n", this->mqtt_basepath.c_str());
  response->print("</tr>\n");

  response->println("<tr>\n");
  response->println("  <td colspan='2'>\n");
  
  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='sel_URCID1' name='sel_UseRandomClientID' value='none' %s />", (this->mqtt_UseRandomClientID)?"":"checked");
  response->println("<label for='sel_URCID1'>benutze statische MQTT ClientID</label></div>\n");
  
  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='sel_URCID2' name='sel_UseRandomClientID' value='yes' %s />", (this->mqtt_UseRandomClientID)?"checked":"");
  response->println("<label for='sel_URCID2'>benutze dynamische MQTT ClientID</label></div>\n");
    
  response->println("  </td>\n");
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("<td>Senden einer KeepAlive Message via MQTT (in sek > 10, 0=disabled)</td>\n");
  response->printf("<td><input min='10' max='65000' name='keepalive' type='number' value='%d'/></td>\n", this->keepalive);
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("<td>DebugMode (0 [off] .. 5 [max]</td>\n");
  response->printf("<td><input min='0' max='5' name='debuglevel' type='number' value='%d'/></td>\n", this->debuglevel);
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("<td>Pin i2c SDA</td>\n");
  response->printf("<td><input min='0' max='15' id='GpioPin_0' name='pinsda' type='number' value='%d'/></td>\n", this->pin_sda +200 );
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("<td>Pin i2c SCL</td>\n");
  response->printf("<td><input min='0' max='15' id='GpioPin_1' name='pinscl' type='number' value='%d'/></td>\n", this->pin_scl +200 );
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("  <td colspan='2'>\n");

  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='ow1' name='sel_1wire' value='none' %s onclick=\"radioselection([''],['onewire_0'])\"/>", (this->enable_1wire)?"":"checked");
  response->println("<label for='ow1'>kein OneWire</label></div>\n");
    
  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='ow2' name='sel_1wire' value='1wire' %s onclick=\"radioselection(['onewire_0'],[''])\"/>", (this->enable_1wire)?"checked":"");
  response->println("<label for='ow2'>OneWire Aktiv</label></div>\n");
  
  response->println("  </td>\n");
  response->println("</tr>\n");

  response->printf("<tr class='%s' id='onewire_0'>\n", (this->enable_1wire?"":"hide"));
  response->println("<td>Welcher Pin nutzt 1Wire?</td>\n");
  response->printf("<td><input min='0' max='15' id='GpioPin_3' name='pin1wire' type='number' value='%d'/></td>\n", this->pin_1wire +200 );
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("  <td colspan='2'>\n");

  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='sel_oled1' name='sel_oled' value='none' %s onclick=\"radioselection([''],['oled_0','oled_1'])\"/>", (this->enable_oled)?"":"checked");
  response->println("<label for='sel_oled1'>kein OLED</label></div>\n");
  
  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='sel_oled2' name='sel_oled' value='oled' %s onclick=\"radioselection(['oled_0','oled_1'],[''])\"/>", (this->enable_oled)?"checked":"");
  response->println("<label for='sel_oled2'>mit OLED</label></div>\n");
    
  response->println("  </td>\n");
  response->println("</tr>\n");

  response->printf("<tr class='%s' id='oled_0'>\n", (this->enable_oled?"":"hide"));
  response->println("<td>i2c Adresse OLED</td>\n");
  response->printf("<td><input maxlength='2' name='i2coled' type='text' value='%02x'/></td>\n", this->i2caddress_oled);
  response->println("</tr>\n");

  response->printf("<tr class='%s' id='oled_1'>\n", (this->enable_oled?"":"hide"));
  response->println("<td>OLED Typ</td>\n");
  response->println("<td><select name='oled_type' size='1'> \n");
  response->printf("  <option %s value='0'/>OLED SSD1306</option>\n", (this->oled_type==0?"selected":""));
  response->printf("  <option %s value='1'/>OLED SH1106</option>\n", (this->oled_type==1?"selected":""));
  response->println("</select></td>\n");
  response->println("</tr>\n");
  
  response->println("<tr>\n");
  response->println("  <td colspan='2'>\n");
  
  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='sel_3wege_0' name='sel_3wege' value='none' %s onclick=\"radioselection([''],['3wege_0'])\"/>", (this->enable_3wege)?"":"checked");
  response->println("<label for='sel_3wege_0'>kein Trinkwasser Bypass</label></div>\n");
  
  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='sel_3wege_1' name='sel_3wege' value='3wege' %s onclick=\"radioselection(['3wege_0'],[''])\"/>", (this->enable_3wege)?"checked":"");
  response->println("<label for='sel_3wege_1'>mit Trinkwasser ByPass Ventil</label></div>\n");
    
  response->println("  </td>\n");
  response->println("</tr>\n");

  response->printf("<tr class='%s' id='3wege_0'>\n", (this->enable_3wege?"":"hide"));
  response->println("<td>3WegeVentil Trinkwasser Bypass</td>\n");
  response->printf("<td><input min='0' max='254' id='ConfiguredPorts_0' name='ventil3wege_port' type='number' value='%d'/></td>\n", this->ventil3wege_port);
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("  <td colspan='2'>\n");
  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='sel_update_0' name='sel_update' value='auto' %s onclick=\"radioselection(['update_0'],['update_1'])\"/>", (this->enable_autoupdate)?"checked":"");
  response->println("<label for='sel_update_0'>Automatisches Update</label></div>\n");
  response->println("    <div class='inline'>");
  response->printf("<input type='radio' id='sel_update_1' name='sel_update' value='manu' %s onclick=\"radioselection(['update_1'],['update_0'])\"/>", (this->enable_autoupdate)?"":"checked");
  response->println("<label for='sel_update_1'>Manuelles Update</label></div>\n");
  response->println("  </td>\n");
  response->println("</tr>\n");

  response->println("<tr>\n");
  response->println("<td>Update URL</td>\n");
  response->printf("<td><input size='30' name='autoupdate_url' type='text' value='%s'/></td>\n", this->autoupdate_url.c_str());
  response->println("</tr>\n");

  response->printf("<tr class='%s' id='update_0'>\n", (this->enable_autoupdate?"":"hide"));
  response->println("<td>Auswahl der Stage</td>\n");
  response->println("<td>\n");
  response->println("  <select id='autoupdate_stage' name='autoupdate_stage'>\n");
  response->printf("  <option value='%s' %s>%s</option>\n", "PROD", (this->autoupdate_stage==PROD?"selected":""), "Production");
  response->printf("  <option value='%s' %s>%s</option>\n", "PRE", (this->autoupdate_stage==PRE?"selected":""), "PreLive/QS");
  response->printf("  <option value='%s' %s>%s</option>\n", "DEV", (this->autoupdate_stage==DEV?"selected":""), "Development");
  response->println("  </select>\n");
  response->println("</td>\n");
  response->println("</tr>\n");
  
  response->printf("<tr class='%s' id='update_1'>\n", (this->enable_autoupdate?"hide":""));
  response->println("<td>verfügbare Releases\n");
  response->println("<a href='#' onclick='RefreshReleases()' class='ButtonRefresh' title='Die JSON Liste neu laden'>&#8634;</a>");
  response->println("</td><td>\n");

  response->println("  <select id='releases' name='releases'>\n");
  std::vector<release_t>* rel = ESPUpdate->GetReleases();
  
  response->println("  <optgroup label='Produktiv'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "PROD") {
      response->printf("<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
    }
  }
  response->println("  </optgroup>\n");

  response->println("  <optgroup label='PreLive'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "PRE") {
      response->printf("<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
    }
  }
  response->println("  </optgroup>\n");

  response->println("  <optgroup label='Development'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "DEV") {
      response->printf("<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
    }
  }
  response->println("  </optgroup>\n");

  response->println("  </select>\n");
  response->println("<input type='button' class='button' onclick='InstallRelease()' value='Install' />");
  response->println("</td>\n");
  response->println("</tr>\n");
 
  response->println("  </td>\n");
  response->println("</tr>\n");
  
  response->println("</tbody>\n");
  response->println("</table>\n");


  response->println("</form>\n\n<br />\n");
  response->println("<form id='jsonform' action='StoreBaseConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  response->println("  <input type='text' id='json' name='json' />\n");
  response->println("  <input type='submit' value='Speichern' />\n");
  response->println("</form>\n\n");
}
