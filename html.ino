String getCSS() {
  html_str  = "  body {\n";
  html_str += "   font-size:140%;\n";
  html_str += "   font-family: Verdana,Arial,Helvetica,sans-serif;\n";
  html_str += " } \n";
  html_str += " h2 {\n";
  html_str += "   color: #2e6c80;\n";
  html_str += "   text-align: center;\n";
  html_str += " } \n";
  html_str += " td, th {\n";
  html_str += "     font-size: 14px;\n";
  html_str += " }\n";
  html_str += " .button { \n";
  html_str += "     padding: 4px 16px; \n";
  html_str += "     margin: 4px;\n";
  html_str += "     background-color: #07D;\n";
  html_str += "     color: #FFF;\n";
  html_str += "     text-decoration: none;\n";
  html_str += "     border-radius: 4px;\n";
  html_str += "     border: none;\n";
  html_str += " }\n";
  html_str += ".button:hover {background: #369;}\n";
  html_str += " input, select, textarea {margin: 4px; padding: 4px 8px; border-radius: 4px; background-color: #eee; border-style: solid; border-width: 1px; border-color: gray;}\n";
  html_str += " input:hover {background-color: #ccc; }\n";
  html_str += " select:hover {background-color: #ccc; }\n";
  html_str += " textarea:hover {background-color: #ccc; }\n";
  html_str += " .editorDemoTable {\n";
  html_str += "     border-spacing: 0;\n";
  html_str += "     background-color: #FFF8C9;\n";
  html_str += "     margin-left: auto; margin-right: auto;\n";
  html_str += " } \n";
  html_str += " .editorDemoTable thead {\n";
  html_str += "     color: #000000;\n";
  html_str += "     background-color: #2E6C80;\n";
  html_str += " }\n";
  html_str += " .editorDemoTable thead td {\n";
  html_str += "     font-weight: bold;\n";
  html_str += "     font-size: 13px;\n";
  html_str += " }\n";
  html_str += " .editorDemoTable td {\n";
  html_str += "     border: 1px solid #777;\n";
  html_str += "     margin: 0 !important;\n";
  html_str += "     padding: 2px 3px;\n";
  html_str += " }\n";
  html_str += " .navi {\n";
  html_str += "   border-bottom: 3px solid #777;\n";
  html_str += "   padding: 5px;\n";
  html_str += "   text-align: center;\n";
  html_str += " }\n";
  html_str += " .navi_active {\n";
  html_str += "   background-color: #CCCCCC;\n";
  html_str += "   border: 3px solid #777;;\n";
  html_str += "   border-bottom: none;\n";
  html_str += " }\n";
  
  return html_str.c_str();  
}

String getPageHeader(int pageactive) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  html_str  = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n";
  html_str += "<meta charset='utf-8'>\n";
  html_str += "<link rel='stylesheet' type='text/css' href='/style.css'>\n";
  html_str += "<title>Bewässerungssteuerung</title></head>\n";
  html_str += "<body>\n";
  html_str += "<table>\n";
  html_str += "  <tr>\n";
  html_str += "   <td colspan='13'>\n";
  html_str += "     <h2>Konfiguration</h2>\n";
  html_str += "   </td>\n";
  html_str += " </tr>\n";
  html_str += " <tr>\n";
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==1)?"navi_active":"");
  html_str += buffer;
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/PinConfig'>Pin Config</a></td>\n", (pageactive==2)?"navi_active":"");
  html_str += buffer;
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/SensorConfig'>Sensor Config</a></td>\n", (pageactive==3)?"navi_active":"");
  html_str += buffer;
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/VentilConfig'>Ventil Config</a></td>\n", (pageactive==4)?"navi_active":"");
  html_str += buffer;
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/AutoConfig'>Automatik</a></td>\n", (pageactive==5)?"navi_active":"");
  html_str += buffer;
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  html_str += "   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/ESP8266_PumpControl/wiki' target='_blank'>Wiki</a></td>\n";
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  html_str += " </tr>\n";
  html_str += "</table>\n";
  html_str += "<p />\n";

  return html_str.c_str();  
}

void setPage_Footer() {
  html_str += "</body>\n";
  html_str += "</html>\n";
}
String getPage_Status() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));

  html_str = getPageHeader(1);
  html_str += "<table style='width: 100%;'>\n";
  html_str += "<tbody>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 12%; text-align: right;'>IP-Adresse:</td>\n";
  sprintf(buffer, "<td style='width: 12%; text-align: left;'>%s</td>\n", WiFi.localIP().toString().c_str());
  html_str += buffer;
  html_str += "<td style='width: 12%; text-align: right;'>WiFi Name:</td>\n";
  sprintf(buffer, "<td style='width: 12%; text-align: left;'>%s</td>\n", WiFi.SSID().c_str());
  html_str += buffer;
  html_str += "<td style='width: 12%; text-align: right;'>i2c Bus:</td>\n";
  html_str += "<td style='width: 12%; text-align: left;'>";
  for (uint8_t i=0; i<8; i++) {
    if (i2c_adresses[i] > 0) {
      sprintf(buffer, " %02x", i2c_adresses[i]);
      html_str += buffer;
      html_str += ", ";
    }
  }
  html_str += "</td>\n";
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 12%; text-align: right;'>MAC:</td>\n";
  sprintf(buffer, "<td style='width: 12%; text-align: left;'>%s</td>\n", WiFi.macAddress().c_str());
  html_str += buffer;
  html_str += "<td style='width: 12%; text-align: right;'>WiFi RSSI:</td>\n";
  sprintf(buffer, "<td style='width: 12%; text-align: left;'>%d</td>\n", WiFi.RSSI());
  html_str += buffer;
  html_str += "<td style='width: 12%; text-align: right;'>&nbsp;</td>\n";
  html_str += "<td style='width: 12%; text-align: left;'>&nbsp;</td>\n";
  html_str += "</tr>\n";
  html_str += "</tbody>\n";
  html_str += "</table>\n";

  setPage_Footer();
  return html_str.c_str();  
}



String getPage_PinConfig() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  html_str = getPageHeader(2);
  
  html_str += "<form id='F1' action='StorePinConfig' method='POST'>\n";
  html_str += "<table class='editorDemoTable'>\n";
  html_str += "<thead>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 250px;'>Name</td>\n";
  html_str += "<td style='width: 200px;'>Wert</td>\n";
  html_str += "</tr>\n";
  html_str += "</thead>\n";
  html_str += "<tbody>\n";
  html_str += "<tr>\n";
  html_str += "<td>MQTT Server IP</td>\n";
  sprintf(buffer, "<td><input maxlength='15' name='mqttserver' type='text' value='%s'/></td>\n", mqtt_server);
  html_str += buffer;
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td>MQTT Server Port</td>\n";
  sprintf(buffer, "<td><input maxlength='5' name='mqttport' type='text' value='%d'/></td>\n", mqtt_port);
  html_str += buffer;
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td>MQTT Root</td>\n";
  sprintf(buffer, "<td><input maxlength='40' name='mqttroot' type='text' value='%s'/></td>\n", mqtt_root);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Pin HC-SR04 Trigger</td>\n";
  sprintf(buffer, "<td><input min='0' max='15' name='pinhcsr04trigger' type='number' value='%d'/></td>\n", pin_hcsr04_trigger); 
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Pin HC-SR04 Echo</td>\n";
  sprintf(buffer, "<td><input min='0' max='15' name='pinhcsr04echo' type='number' value='%d'/></td>\n", pin_hcsr04_echo);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Pin i2c SDA</td>\n";
  sprintf(buffer, "<td><input min='0' max='15' name='pinsda' type='number' value='%d'/></td>\n", pin_sda);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Pin i2c SCL</td>\n";
  sprintf(buffer, "<td><input min='0' max='15' name='pinscl' type='number' value='%d'/></td>\n", pin_scl);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>i2c Adresse PFC8574</td>\n";
  sprintf(buffer, "<td><input maxlength='2' name='i2cpfc8574' type='text' value='%02x'/></td>\n", i2caddress_pfc8574);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>i2c Adresse OLED 1306</td>\n";
  sprintf(buffer, "<td><input maxlength='2' name='i2coled' type='text' value='%02x'/></td>\n", i2caddress_oled);
  html_str += buffer;
  html_str += "</tr>\n";
 
  html_str += "</tbody>\n";
  html_str += "</table>\n";
  html_str += "<p><br /><input class='button' type='submit' value='Speichern' /></form>\n";

  setPage_Footer();
  return html_str.c_str();  
}


String getPage_SensorConfig() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  html_str = getPageHeader(3);

  html_str += "<form id='F2' action='StoreSensorConfig' method='POST'>\n";
  html_str += "<table class='editorDemoTable'>\n";
  html_str += "<thead>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 250px;'>Name</td>\n";
  html_str += "<td style='width: 200px;'>Wert</td>\n";
  html_str += "</tr>\n";
  html_str += "</thead>\n";
  html_str += "<tbody>\n";

  html_str += "<tr>\n";
  html_str += "<td>Messintervall HC-SR04</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='hcsr04interval' type='number' value='%d'/></td>\n", hc_sr04_interval);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Abstand Sensor min (in cm)</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='hcsr04distmin' type='number' value='%d'/></td>\n", hc_sr04_distmin);
  html_str += buffer;
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td>Abstand Sensor max (in cm)</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='hcsr04distmax' type='number' value='%d'/></td>\n", hc_sr04_distmax);
  html_str += buffer;
  html_str += "</tr>\n";
  html_str += "</tbody>\n";
  html_str += "</table>\n";
  html_str += "<p><br /><input class='button' type='submit' value='Speichern' /></form>\n";

  setPage_Footer();
  return html_str.c_str();  
}


String getPage_VentilConfig() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  html_str = getPageHeader(4);

  html_str += "<form id='F2' action='StoreVentilConfig' method='POST'>\n";
  html_str += "<table class='editorDemoTable'>\n";
  html_str += "<thead>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 25px;'>Nr</td>\n";
  html_str += "<td style='width: 25px;'>Active</td>\n";
  html_str += "<td style='width: 250px;'>MQTT SubTopic</td>\n";
  html_str += "<td style='width: 50 px;'>PCF8574 Port</td>\n";
  html_str += "</tr>\n";
  html_str += "</thead>\n";
  html_str += "<tbody>\n\n";
  
  html_str += "<tr>\n";
  for(int i=0; i < pcf8574devCount; i++) {
    sprintf(buffer, "<tr><td>%d</td>", i);
    html_str += buffer;
    sprintf(buffer, "<td><input name='active_%d' type='checkbox' value='1' %s/></td>", i, (pcf8574dev[i].enabled?"checked":""));
    html_str += buffer;
    sprintf(buffer, "<td><input maxlength='25' name='mqtttopic_%d' type='text' value='%s'/></td>", i, pcf8574dev[i].subtopic);
    html_str += buffer;
    sprintf(buffer, "<td><input name='pcfport_%d' type='number' min='0' max='128' value='%d'/></td>", i, pcf8574dev[i].port);
    html_str += buffer;
    html_str += "</tr>\n";
  }
  html_str += "</tbody>\n";
  html_str += "</table>\n";
  html_str += "<br /><input class='button' type='submit' value='Speichern' /></form>\n";

  setPage_Footer();
  return html_str.c_str();  
}


String getPage_AutoConfig() {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  html_str = getPageHeader(5);
  html_str += "<form id='F2' action='StoreAutoConfig' method='POST'>\n";
  html_str += "<table class='editorDemoTable'>\n";
  html_str += "<thead>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 25px;'>Active</td>\n";
  html_str += "<td style='width: 400px;'>Name</td>\n";
  html_str += "<td style='width: 200px;'>Wert</td>\n";
  html_str += "</tr>\n";
  html_str += "</thead>\n";
  html_str += "<tbody>\n";
  html_str += "<tr>\n";
  sprintf(buffer, "<td style='text-align: center; width: 46px;'><input name='enable_waterswitch' type='checkbox' value='1' /></td>\n", (enable_waterswitch?"checked":""));
  html_str += buffer;
  html_str += "<td>Ventil, welches bei leerem F&uuml;llstand angeschaltet werden soll (Wechsel zwischen Regen- und Trinkwasser)</td>\n";
  html_str += "<td><select name='waterswitch_port' size='1'>";
  for(int i=0; i < pcf8574devCount; i++) {
    if(pcf8574dev[i].enabled) {
      sprintf(buffer, "<option value='%d' %s>%s</option>\n", pcf8574dev[i].port, (waterswitch_port==pcf8574dev[i].port?"selected":""), pcf8574dev[i].subtopic);
      html_str += buffer;
    }
  }
  html_str += "</select></td>\n";
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='text-align: center;'>&nbsp;</td>\n";
  html_str += "<td >Sensor Treshold Min</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='treshold_min' type='number' value='%d'/></td>\n", hc_sr04_treshold_min);
  html_str += buffer;
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='text-align: center;'>&nbsp;</td>\n";
  html_str += "<td>Sensor Treshold Max</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='treshold_max' type='number' value='%d'/></td>\n", hc_sr04_treshold_max);
  html_str += buffer;
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  sprintf(buffer, "<td style='text-align: center;'><input name='enable_syncswitch' type='checkbox' value='1' /></td>\n", (enable_syncswitch?"checked":""));
  html_str += buffer;
  html_str += "<td>Ventil Trinkwasser Bypass</td>\n";
  html_str += "<td><select name='syncswitch_port' size='1'>";
  for(int i=0; i < pcf8574devCount; i++) {
    if(pcf8574dev[i].enabled) {
      sprintf(buffer, "<option value='%d' %s>%s</option>\n", pcf8574dev[i].port, (syncswitch_port==pcf8574dev[i].port?"selected":""), pcf8574dev[i].subtopic);
      html_str += buffer;
    }
  }
  html_str += "</select></td>\n";
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  sprintf(buffer, "<td style='text-align: center;'><input name='enable_3wege' type='checkbox' value='1' /></td>\n", (enable_3wege?"checked":""));
  html_str += buffer;
  html_str += "<td>3WegeVentil Trinkwasser Bypass</td>\n";
  html_str += "<td><select name='ventil3wege_port' size='1'>";
  for(int i=0; i < pcf8574devCount; i++) {
    if(pcf8574dev[i].enabled) {
      sprintf(buffer, "<option value='%d' %s>%s</option>\n", pcf8574dev[i].port, (ventil3wege_port==pcf8574dev[i].port?"selected":""), pcf8574dev[i].subtopic);
      html_str += buffer;
    }
  }
  html_str += "</select></td>\n";
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td style='text-align: center;'>&nbsp;</td>\n";
  html_str += "<td>Max. parallel</td>\n";
  sprintf(buffer, "<td><input min='0' max='16' name='max_parallel' type='number' value='%d'/></td>\n", max_parallel);
  html_str += buffer;
  html_str += "</tr>\n";
  
  html_str += "</tbody>\n";
  html_str += "</table>\n";
  html_str += "<br /><input class='button' type='submit' value='Speichern' /></form>\n";

  setPage_Footer();
  return html_str.c_str();  
}

