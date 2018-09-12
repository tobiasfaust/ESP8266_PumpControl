String getCSS() {
  html_str  = "body {font-size:140%;} \n";
  html_str += "h2 {color: #2e6c80;} \n";
  html_str += ".button { \n";
  html_str += "  padding:10px 10px 10px 10px; \n";
  html_str += "  width:100%;\n";
  html_str += "  background-color: #4CAF50;\n";
  html_str += "  font-size: 120%;\n";
  html_str += "}\n";
  html_str  = ".editorDemoTable thead td {\n";
  html_str += "    font-weight: bold;\n";
  html_str += "    font-size: 13px;\n";
  html_str += "}\n";
  html_str += ".editorDemoTable td {\n";
  html_str += "    border: 1px solid #777;\n";
  html_str += "    margin: 0 !important;\n";
  html_str += "    padding: 2px 3px;\n";
  html_str += "}\n";
  html_str += "td, th {\n";
  html_str += "    font-family: Verdana,Arial,Helvetica,sans-serif;\n";
  html_str += "    font-size: 14px;\n";
  html_str += "}\n";
  html_str += ".editorDemoTable thead {\n";
  html_str += "    color: #000000;\n";
  html_str += "    background-color: #2E6C80;\n";
  html_str += "}\n";
  html_str += ".editorDemoTable {\n";
  html_str += "    border-spacing: 0;\n";
  html_str += "    background-color: #FFF8C9;\n";
  html_str += "}\n";
  
  return html_str.c_str();  
}

String getMainPage() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  html_str  = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n";
  html_str += "<meta charset='utf-8'>\n";
  html_str += "<link rel='stylesheet' type='text/css' href='/style.css'>\n";
  html_str += "<style> </style>\n";
  html_str += "<title>Bewässerungssteuerung</title></head><body>\n";
  html_str += "<table style='margin-left: auto; margin-right: auto;'>\n";
  html_str += "<tbody>\n";
  html_str += "<tr>\n";
  html_str += "<td style='text-align: center;' colspan='3'>\n";
  html_str += "<h2 style='color: #2e6c80;'>Einstellungen</h2>\n";
  html_str += "</td>\n";
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='text-align: center;' colspan='3'>\n";
  
  html_str += "<table style='width: 100%;'>\n";
  html_str += "<tbody>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 10%;' rowspan='2'>\n";
  html_str += "<h2>Infos</h2>\n";
  html_str += "</td>\n";
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
  
  html_str += "</td>\n";
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 33%; text-align: left; vertical-align: top;'>\n";
  html_str += "<h2>ToDo</h2>\n";
  
  /*html_str += "<table>\n";
  html_str += "<tbody>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 250px;'>IP-Adresse</td>\n";
  html_str += "<td>\n";
  html_str += WiFi.localIP().toString().c_str();
  html_str += "</td>\n";
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 250px;'>MAC</td>\n";
  html_str += "<td>\n";
  html_str += WiFi.macAddress().c_str();
  html_str += "</td>\n";
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 250px;'>WiFi Name</td>\n";
  html_str += "<td>\n";
  html_str += WiFi.SSID().c_str();
  html_str += "</td>\n";
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 250px;'>WiFi RSSI</td>\n";
  html_str += "<td>\n";
  itoa(WiFi.RSSI(), buffer, 10);
  html_str += buffer;

  html_str += "</td>\n";
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 250px;'>i2c Bus</td>\n";
  html_str += "<td>\n";
  for (uint8_t i=0; i<8; i++) {
    if (i2c_adresses[i] > 0) {
      sprintf(buffer, " %02x", i2c_adresses[i]);
      html_str += buffer;
      html_str += ", ";
    }
  }
  html_str += "</td>\n";
  html_str += "</tr>\n";

  html_str += "</tbody>\n";
  html_str += "</table>\n";
*/
  
  html_str += "</td>\n";
  html_str += "<td style='width: 33%; text-align: left; vertical-align: top;'>\n";
  html_str += "<h2>HW-Konfiguration:</h2>\n";
  html_str += "<form id='F1' action='StoreParam' method='POST'>\n";
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
  html_str += "<td>Messintervall HC-SR04</td>\n";
  sprintf(buffer, "<td><input min='0' max='3600' name='hcsr04interval' type='number' value='%d'/></td>\n", hc_sr04_interval);
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
  html_str += "<br /><input class='button' type='submit' value='Speichern' /></form>\n";
  html_str += "<p>&nbsp;</p>\n";
  html_str += "</td>\n";
  

  html_str += "<td style='width: 33%; text-align: left; vertical-align: top;'>\n";
  html_str += "<h2>Ventil Konfiguration</h2>\n";
  html_str += "<form id='F2' action='StoreSwitchConfig' method='POST'>\n";
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

  html_str += "</td>\n";
  html_str += "</tr>\n";
  html_str += "</body>\n";

  return html_str.c_str();
}
