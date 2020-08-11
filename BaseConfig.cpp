#include "BaseConfig.h"

BaseConfig::BaseConfig() {
  SPIFFS.begin();
  ESPUpdate = new updater;
  LoadJsonConfig();
}

void BaseConfig::StoreJsonConfig(String* json) {
  //https://arduinojson.org/v5/api/jsonobject/begin_end/
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(*json);
    
  if (root.success()) {
    File configFile = SPIFFS.open("/BaseConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open BaseConfig.json file for writing");
    } else {  
      root.printTo(Serial);
      root.printTo(configFile);
      configFile.close();
  
      LoadJsonConfig();
    }
  }
}

void BaseConfig::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  if (SPIFFS.exists("/BaseConfig.json")) {
    //file exists, reading and loading
    Serial.println("reading sensor config file");
    File configFile = SPIFFS.open("/BaseConfig.json", "r");
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
        if (json.containsKey("mqttroot"))         { this->mqtt_root = json["mqttroot"].as<String>();}
        if (json.containsKey("mqttserver"))       { this->mqtt_server = json["mqttserver"].as<String>();}
        if (json.containsKey("mqttport"))         { this->mqtt_port = atoi(json["mqttport"]);}
        if (json.containsKey("mqttuser"))         { this->mqtt_username = json["mqttuser"].as<String>();}
        if (json.containsKey("mqttpass"))         { this->mqtt_password = json["mqttpass"].as<String>();}
        if (json.containsKey("sel_UseRandomClientID")){ if (strcmp(json["sel_UseRandomClientID"], "none")==0) { this->mqtt_UseRandomClientID=false;} else {this->mqtt_UseRandomClientID=true;}} else {this->mqtt_UseRandomClientID = true;}
        if (json.containsKey("pinsda"))           { this->pin_sda = atoi(json["pinsda"]) - 200;}
        if (json.containsKey("pinscl"))           { this->pin_scl = atoi(json["pinscl"]) - 200;}
        if (json.containsKey("sel_oled"))         { if (strcmp(json["sel_oled"], "none")==0) { this->enable_oled=false;} else {this->enable_oled=true;}}
        if (json.containsKey("sel_3wege"))        { if (strcmp(json["sel_3wege"], "none")==0) { this->enable_3wege=false;} else {this->enable_3wege=true;}}
        if (json.containsKey("sel_update"))       { if (strcmp(json["sel_update"], "manu")==0) { this->enable_autoupdate=false;} else {this->enable_autoupdate=true;}
                                                    ESPUpdate->setAutoMode(this->enable_autoupdate);
                                                  }
        if (json.containsKey("autoupdate_url"))   { this->autoupdate_url = json["autoupdate_url"].as<String>(); 
                                                    ESPUpdate->setIndexJson(this->autoupdate_url);}
        if (json.containsKey("autoupdate_stage")) { if (json["autoupdate_stage"] == "PROD") { this->autoupdate_stage = (stage_t)PROD; }
                                                    if (json["autoupdate_stage"] == "PRE")  { this->autoupdate_stage = (stage_t)PRE; }
                                                    if (json["autoupdate_stage"] == "DEV")  { this->autoupdate_stage = (stage_t)DEV; }
                                                    ESPUpdate->setStage(this->autoupdate_stage);
                                                  }
        if (json.containsKey("i2coled"))          { this->i2caddress_oled = strtoul(json["i2coled"], NULL, 16);} // hex convert to dec        
        if (json.containsKey("ventil3wege_port")) { this->ventil3wege_port = atoi(json["ventil3wege_port"]);}
        
      } else {
        Serial.println("failed to load json config, load default config");
        loadDefaultConfig = true;
      }
    }
  } else {
    Serial.println("BaseConfig.json config File not exists, load default config");
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    this->mqtt_server = "192.178.168.1";
    this->mqtt_port  = 1883;
    this->mqtt_root = "PumpControl";
    this->mqtt_UseRandomClientID = true;
    this->pin_sda = 5;
    this->pin_scl = 4;
    this->enable_oled = false;
    this->i2caddress_oled = 60; //0x3C;
    this->enable_3wege = false;
    this->ventil3wege_port = 0;
    this->max_parallel = 0;
    this->enable_autoupdate = true;
    this->autoupdate_url="http://tfa-releases.s3-website.eu-central-1.amazonaws.com/ESP8266_PumpControl/releases.json";
    
    loadDefaultConfig = false; //set back
  }

  if (this->enable_oled) {oled->init(this->pin_sda, this->pin_scl, this->i2caddress_oled);}
  oled->Enable(this->enable_oled);

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

void BaseConfig::GetWebContent(String* html) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  html->concat("<form id='DataForm'>\n");
  html->concat("<table id='maintable' class='editorDemoTable'>\n");
  html->concat("<thead>\n");
  html->concat("<tr>\n");
  html->concat("<td style='width: 250px;'>Name</td>\n");
  html->concat("<td style='width: 200px;'>Wert</td>\n");
  html->concat("</tr>\n");
  html->concat("</thead>\n");
  html->concat("<tbody>\n");

  html->concat("<tr>\n");
  html->concat("<td>Device Name</td>\n");
  sprintf(buffer, "<td><input size='30' maxlength='40' name='mqttroot' type='text' value='%s'/></td>\n", this->mqtt_root.c_str());
  html->concat(buffer);
  html->concat("</tr>\n");
  
  html->concat("<tr>\n");
  html->concat("<td>MQTT Server IP</td>\n");
  sprintf(buffer, "<td><input size='30' name='mqttserver' type='text' value='%s'/></td>\n", this->mqtt_server.c_str());
  html->concat(buffer);
  html->concat("</tr>\n");
  
  html->concat("<tr>\n");
  html->concat("<td>MQTT Server Port</td>\n");
  sprintf(buffer, "<td><input maxlength='5' name='mqttport' type='text' value='%d'/></td>\n", this->mqtt_port);
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>MQTT Authentification: Username (optional)</td>\n");
  sprintf(buffer, "<td><input size='30' name='mqttuser' type='text' value='%s'/></td>\n", this->mqtt_username.c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>MQTT Authentification: Password (optional)</td>\n");
  sprintf(buffer, "<td><input size='30' name='mqttpass' type='text' value='%s'/></td>\n", this->mqtt_password.c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("  <td colspan='2'>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel_URCID1' name='sel_UseRandomClientID' value='none' %s />", (this->mqtt_UseRandomClientID)?"":"checked");
  html->concat(buffer);
  html->concat("<label for='sel_URCID1'>benutze statische MQTT ClientID</label></div>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel_URCID2' name='sel_UseRandomClientID' value='yes' %s />", (this->mqtt_UseRandomClientID)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel_URCID2'>benutze dynamische MQTT ClientID</label></div>\n");
    
  html->concat("  </td>\n");
  html->concat("</tr>\n");
  
  html->concat("<tr>\n");
  html->concat("<td>Pin i2c SDA</td>\n");
  sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_0' name='pinsda' type='number' value='%d'/></td>\n", this->pin_sda +200 );
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Pin i2c SCL</td>\n");
  sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_1' name='pinscl' type='number' value='%d'/></td>\n", this->pin_scl +200 );
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("  <td colspan='2'>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel2' name='sel_oled' value='none' %s onclick=\"radioselection([''],['oled_0'])\"/>", (this->enable_oled)?"":"checked");
  html->concat(buffer);
  html->concat("<label for='sel2'>kein OLED</label></div>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel3' name='sel_oled' value='oled' %s onclick=\"radioselection(['oled_0'],[''])\"/>", (this->enable_oled)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel3'>mit OLED</label></div>\n");
    
  html->concat("  </td>\n");
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='oled_0'>\n", (this->enable_oled?"":"hide"));
  html->concat(buffer);
  html->concat("<td>i2c Adresse OLED 1306</td>\n");
  sprintf(buffer, "<td><input maxlength='2' name='i2coled' type='text' value='%02x'/></td>\n", this->i2caddress_oled);
  html->concat(buffer);
  html->concat("</tr>\n");
 
  html->concat("<tr>\n");
  html->concat("  <td colspan='2'>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel_3wege_0' name='sel_3wege' value='none' %s onclick=\"radioselection([''],['3wege_0'])\"/>", (this->enable_3wege)?"":"checked");
  html->concat(buffer);
  html->concat("<label for='sel_3wege_0'>kein Trinkwasser Bypass</label></div>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel_3wege_1' name='sel_3wege' value='3wege' %s onclick=\"radioselection(['3wege_0'],[''])\"/>", (this->enable_3wege)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel_3wege_1'>mit Trinkwasser ByPass Ventil</label></div>\n");
    
  html->concat("  </td>\n");
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='3wege_0'>\n", (this->enable_3wege?"":"hide"));
  html->concat(buffer);
  html->concat("<td>3WegeVentil Trinkwasser Bypass</td>\n");
  sprintf(buffer, "<td><input min='0' max='254' id='ConfiguredPorts_0' name='ventil3wege_port' type='number' value='%d'/></td>\n", this->ventil3wege_port);
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("  <td colspan='2'>\n");
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel_update_0' name='sel_update' value='auto' %s onclick=\"radioselection(['update_0'],['update_1'])\"/>", (this->enable_autoupdate)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel_update_0'>Automatisches Update</label></div>\n");
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel_update_1' name='sel_update' value='manu' %s onclick=\"radioselection(['update_1'],['update_0'])\"/>", (this->enable_autoupdate)?"":"checked");
  html->concat(buffer);
  html->concat("<label for='sel_update_1'>Manuelles Update</label></div>\n");
  html->concat("  </td>\n");
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Update URL</td>\n");
  sprintf(buffer, "<td><input size='30' name='autoupdate_url' type='text' value='%s'/></td>\n", this->autoupdate_url.c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='update_0'>\n", (this->enable_autoupdate?"":"hide"));
  html->concat(buffer);
  html->concat("<td>Auswahl der Stage</td>\n");
  html->concat("<td>\n");
  html->concat("  <select id='autoupdate_stage' name='autoupdate_stage'>\n");
  sprintf(buffer, "  <option value='%s' %s>%s</option>\n", "PROD", (this->autoupdate_stage==PROD?"selected":""), "Production");
  html->concat(buffer);
  sprintf(buffer, "  <option value='%s' %s>%s</option>\n", "PRE", (this->autoupdate_stage==PRE?"selected":""), "PreLive/QS");
  html->concat(buffer);
  sprintf(buffer, "  <option value='%s' %s>%s</option>\n", "DEV", (this->autoupdate_stage==DEV?"selected":""), "Development");
  html->concat(buffer);
  html->concat("  </select>\n");
  html->concat("</td>\n");
  html->concat("</tr>\n");
  
  sprintf(buffer, "<tr class='%s' id='update_1'>\n", (this->enable_autoupdate?"hide":""));
  html->concat(buffer);
  html->concat("<td>verfügbare Releases\n");
  html->concat("<a href='#' onclick='RefreshReleases()' title='Die JSON Liste neu laden'>&#8634;</a>");
  html->concat("</td><td>\n");
  
  html->concat("  <select id='releases' name='releases'>\n");
  std::vector<release_t>* rel = ESPUpdate->GetReleases();
  
  html->concat("  <optgroup label='Produktiv'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "PROD") {
      sprintf(buffer, "<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
      html->concat(buffer);
    }
  }
  html->concat("  </optgroup>\n");

  html->concat("  <optgroup label='PreLive'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "PRE") {
      sprintf(buffer, "<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
      html->concat(buffer);
    }
  }
  html->concat("  </optgroup>\n");

  html->concat("  <optgroup label='Development'>\n");
  for (uint8_t i=0; i < rel->size(); i++) {
    if (ESPUpdate->Stage2String(rel->at(i).stage) == "DEV") {
      sprintf(buffer, "<option value='%d' %s>%s (%d)</option>\n", rel->at(i).number, (rel->at(i).number==ESPUpdate->GetCurrentRelease()->number?"disabled":""), rel->at(i).name.c_str(), rel->at(i).subversion);
      html->concat(buffer);
    }
  }
  html->concat("  </optgroup>\n");
  
  html->concat("  </select>\n");
  html->concat("<input type='button' class='button' onclick='InstallRelease()' value='Install' />");
  html->concat("</td>\n");
  html->concat("</tr>\n");
 
  html->concat("  </td>\n");
  html->concat("</tr>\n");
  
  html->concat("</tbody>\n");
  html->concat("</table>\n");


  html->concat("</form>\n\n<br />\n");
  html->concat("<form id='jsonform' action='StoreBaseConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html->concat("  <input type='text' id='json' name='json' />\n");
  html->concat("  <input type='submit' value='Speichern' />\n");
  html->concat("</form>\n\n");
  html->concat("<div id='ErrorText' class='errortext'></div>\n");  
}
