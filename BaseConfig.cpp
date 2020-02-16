#include "BaseConfig.h"

BaseConfig::BaseConfig() {
  SPIFFS.begin();
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
        if (json.containsKey("pinsda"))           { this->pin_sda = atoi(json["pinsda"]) - 200;}
        if (json.containsKey("pinscl"))           { this->pin_scl = atoi(json["pinscl"]) - 200;}
        if (json.containsKey("sel_oled"))         { if (strcmp(json["sel_oled"], "none")==0) { this->enable_oled=false;} else {this->enable_oled=true;}}
        if (json.containsKey("sel_3wege"))        { if (strcmp(json["sel_3wege"], "none")==0) { this->enable_3wege=false;} else {this->enable_3wege=true;}}
        
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
    this->mqtt_server = "192.178.10.1";
    this->mqtt_port  = 1883;
    this->mqtt_root = "PumpControl";
    this->pin_sda = 5;
    this->pin_scl = 4;
    this->enable_oled = false;
    this->i2caddress_oled = 60; //0x3C;
    this->enable_3wege = false;
    this->ventil3wege_port = 0;
    this->max_parallel = 0;
    
    loadDefaultConfig = false; //set back
  }

  if (this->enable_oled) {oled->init(this->pin_sda, this->pin_scl, this->i2caddress_oled);}
  oled->Enable(this->enable_oled);

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
  sprintf(buffer, "<input type='radio' id='sel0' name='sel_3wege' value='none' %s onclick=\"radioselection([''],['3wege_0'])\"/>", (this->enable_3wege)?"":"checked");
  html->concat(buffer);
  html->concat("<label for='sel0'>kein Trinkwasser Bypass</label></div>\n");
  
  html->concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel1' name='sel_3wege' value='3wege' %s onclick=\"radioselection(['3wege_0'],[''])\"/>", (this->enable_3wege)?"checked":"");
  html->concat(buffer);
  html->concat("<label for='sel1'>mit Trinkwasser ByPass Ventil</label></div>\n");
    
  html->concat("  </td>\n");
  html->concat("</tr>\n");

  sprintf(buffer, "<tr class='%s' id='3wege_0'>\n", (this->enable_3wege?"":"hide"));
  html->concat(buffer);
  html->concat("<td>3WegeVentil Trinkwasser Bypass</td>\n");
  sprintf(buffer, "<td><input min='0' max='254' id='ConfiguredPorts_0' name='ventil3wege_port' type='number' value='%d'/></td>\n", this->ventil3wege_port);
  html->concat(buffer);
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

