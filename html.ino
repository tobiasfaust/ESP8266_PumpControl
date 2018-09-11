String getCSS() {
  html_str += "body {font-size:140%;} ";
  html_str += "h2 {color: #2e6c80;} ";
  html_str += ".button { ";
  html_str += "  padding:10px 10px 10px 10px; ";
  html_str += "  width:100%;";
  html_str += "  background-color: #4CAF50;";
  html_str += "  font-size: 120%;";
  html_str += "}";
  html_str  = ".editorDemoTable thead td {";
  html_str += "    font-weight: bold;";
  html_str += "    font-size: 13px;";
  html_str += "}";
  html_str += ".editorDemoTable td {";
  html_str += "    border: 1px solid #777;";
  html_str += "    margin: 0 !important;";
  html_str += "    padding: 2px 3px;";
  html_str += "}";
  html_str += "td, th {";
  html_str += "    font-family: Verdana,Arial,Helvetica,sans-serif;";
  html_str += "    font-size: 14px;";
  html_str += "}";
  html_str += ".editorDemoTable thead {";
  html_str += "    color: #000000;";
  html_str += "    background-color: #2E6C80;";
  html_str += "}";
  html_str += ".editorDemoTable {";
  html_str += "    border-spacing: 0;";
  html_str += "    background-color: #FFF8C9;";
  html_str += "}";
  
  return html_str.c_str();  
}

String getMainPage() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  html_str = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>";
  html_str += "<meta charset='utf-8'>";
  html_str += "<link rel='stylesheet' type='text/css' href='/style.css'>";
  html_str += "<style> </style>";
  html_str += "<title>Bewässerungssteuerung</title></head><body>";
  
  html_str += "<table style='margin-left: auto; margin-right: auto;'>";
  html_str += "<tbody>";
  html_str += "<tr>";
  html_str += "<td style='text-align: center;' colspan='2'>";
  html_str += "<h2 style='color: #2e6c80;'>Einstellungen</h2>";
  html_str += "</td>";
  html_str += "</tr>";
  html_str += "<tr>";
  html_str += "<td style='width: 33%; text-align: left; vertical-align: top;'>";
  html_str += "<h2>Infos</h2>";
  html_str += "<table>";
  html_str += "<tbody>";
  html_str += "<tr>";
  html_str += "<td style='width: 250px;'>IP-Adresse</td>";
  html_str += "<td>";
  html_str += WiFi.localIP().toString().c_str();
  html_str += "</td>";
  html_str += "</tr>";
  html_str += "<tr>";
  html_str += "<td style='width: 250px;'>MAC</td>";
  html_str += "<td>";
  html_str += WiFi.macAddress().c_str();
  html_str += "</td>";
  html_str += "</tr>";
  html_str += "<tr>";
  html_str += "<td style='width: 250px;'>WiFi Name</td>";
  html_str += "<td>";
  html_str += WiFi.SSID().c_str();
  html_str += "</td>";
  html_str += "</tr>";
  html_str += "<tr>";
  html_str += "<td style='width: 250px;'>WiFi RSSI</td>";
  html_str += "<td>";
  itoa(WiFi.RSSI(), buffer, 10);
  html_str += buffer;
  html_str += "</td>";
  html_str += "</tr>";

  html_str += "<tr>";
  html_str += "<td style='width: 250px;'>i2c Bus</td>";
  html_str += "<td>";
  for (uint8_t i=0; i<8; i++) {
    if (i2c_adresses[i] > 0) {
      sprintf(buffer, " %02x", i2c_adresses[i]);
      html_str += buffer;
      html_str += ", ";
    }
  }
  html_str += "</td>";
  html_str += "</tr>";

  html_str += "</tbody>";
  html_str += "</table>";
  html_str += "</td>";
  html_str += "<td style='width: 33%; text-align: left; vertical-align: top;'>";
  html_str += "<h2>HW-Konfiguration:</h2>";
  html_str += "<form id='F1' action='StoreParam' method='POST'>";
  html_str += "<table class='editorDemoTable'>";
  html_str += "<thead>";
  html_str += "<tr>";
  html_str += "<td style='width: 250px;'>Name</td>";
  html_str += "<td style='width: 200px;'>Wert</td>";
  html_str += "</tr>";
  html_str += "</thead>";
  html_str += "<tbody>";
  html_str += "<tr>";
  html_str += "<td>MQTT Server IP</td>";
  html_str += "<td><input maxlength='15' name='mqttserver' type='text' value='";
  html_str += mqtt_server;
  html_str += "'/></td>";
  html_str += "</tr>";
  html_str += "<tr>";
  html_str += "<td>MQTT Server Port</td>";
  html_str += "<td><input maxlength='5' name='mqttport' type='text' value='";
  html_str += mqtt_port;
  html_str += "'/></td>";
  html_str += "</tr>";
  html_str += "<tr>";
  html_str += "<td>MQTT Root</td>";
  html_str += "<td><input maxlength='40' name='mqttroot' type='text' value='";
  html_str += mqtt_root;
  html_str += "'/></td>";
  html_str += "</tr>";
  
  html_str += "<td>Messintervall HC-SR04</td>";
  html_str += "<td><input min='0' max='3600' name='hcsr04interval' type='number' value='";
  sprintf(buffer, "%d", hc_sr04_interval);
  html_str += buffer;
  html_str += "'/></td>";
  html_str += "</tr>";

  html_str += "<td>Pin HC-SR04 Trigger</td>";
  html_str += "<td><input min='0' max='15' name='pinhcsr04trigger' type='number' value='";
  sprintf(buffer, "%d", pin_hcsr04_trigger); 
  html_str += buffer;
  html_str += "'/></td>";
  html_str += "</tr>";

  html_str += "<td>Pin HC-SR04 Echo</td>";
  html_str += "<td><input min='0' max='15' name='pinhcsr04echo' type='number' value='";
  sprintf(buffer, "%d", pin_hcsr04_echo);
  html_str += buffer;
  html_str += "'/></td>";
  html_str += "</tr>";

  html_str += "<td>Pin i2c SDA</td>";
  html_str += "<td><input min='0' max='15' name='pinsda' type='number' value='";
  sprintf(buffer, "%d", pin_sda);
  html_str += buffer;
  html_str += "'/></td>";
  html_str += "</tr>";

  html_str += "<td>Pin i2c SCL</td>";
  html_str += "<td><input min='0' max='15' name='pinscl' type='number' value='";
  sprintf(buffer, "%d", pin_scl);
  html_str += buffer;
  html_str += "'/></td>";
  html_str += "</tr>";

  html_str += "<td>i2c Adresse PFC8574</td>";
  html_str += "<td><input maxlength='2' name='i2cpfc8574' type='text' value='";
  sprintf(buffer, "%02x", i2caddress_pfc8574);
  html_str += buffer;
  html_str += "'/></td>";
  html_str += "</tr>";

  html_str += "<td>i2c Adresse OLED 1306</td>";
  html_str += "<td><input maxlength='2' name='i2coled' type='text' value='";
  sprintf(buffer, "%02x", i2caddress_oled);
  html_str += buffer;
  html_str += "'/></td>";
  html_str += "</tr>";
 
  html_str += "</tbody>";
  html_str += "</table>";
  html_str += "<br /><input class='button' type='submit' value='Speichern' /></form>";
  html_str += "<p>&nbsp;</p>";
  html_str += "</td>";
  

  html_str += "<td style='width: 33%; text-align: left; vertical-align: top;'>";
  html_str += "<h2>Ventil Konfiguration</h2>";
  html_str += "<form id='F2' action='StoreSwitchConfig' method='POST'>";
  html_str += "<table class='editorDemoTable'>";
  html_str += "<thead>";
  html_str += "<tr>";
  html_str += "<td style='width: 25px;'>Nr</td>";
  html_str += "<td style='width: 25px;'>Active</td>";
  html_str += "<td style='width: 250px;'>MQTT SubTopic</td>";
  html_str += "<td style='width: 50px;'>PCF8574 Port</td>";
  html_str += "</tr>";
  html_str += "</thead>";
  html_str += "<tbody>\n";
  
  html_str += "<tr>";
  for(int i=0; i < pcf8574devCount; i++) {
    sprintf(buffer, "<td>%d</td>", i);
    html_str += buffer;
    sprintf(buffer, "<td><input name='active_%d' type='checkbox' value='1' %s/></td>", i, (pcf8574dev[i].enabled?"checked":""));
    html_str += buffer;
    //if (pcf8574dev[i].enabled) { html_str += "checked"; }
    //html_str += "/></td>";
    sprintf(buffer, "<td><input maxlength='25' name='mqtttopic_%d' type='text' value='%s'/></td>", i, pcf8574dev[i].subtopic);
    html_str += buffer;
    sprintf(buffer, "<td><input name='pcfport_%d' type='number' min='0' max='128' value='%d'/></td>", i, pcf8574dev[i].port);
    html_str += buffer;
    html_str += "</tr>";
  }
  
  html_str += "</tbody>";
  html_str += "</table>";
  html_str += "<br /><input class='button' type='submit' value='Speichern' /></form>";

  html_str += "</td>";
  html_str += "</tr>";
  html_str += "</body>";

  return html_str.c_str();
}
