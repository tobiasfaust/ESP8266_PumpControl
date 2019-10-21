String getJSParam() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  // http://jsfiddle.net/mwPb5/
  // http://jsfiddle.net/mwPb5/1058/
  
  html_str = "";
  // bereits belegte Ports, können nicht ausgewählt werden (zb.vi2c-ports)
  // const gpio_disabled = Array(0,4);
  sprintf(buffer, "const gpio_disabled = [%d,%d,%d,%d];\n", pin_sda, pin_scl, pin_hcsr04_trigger, pin_hcsr04_echo);
  html_str += buffer;

  // anhand gefundener pcf Devices die verfügbaren Ports (jeweils Port0)
  //const pcf_found = [65];
  html_str += "const pcf_found = [";
  uint8_t count=0;
  for (uint8_t i=56; i<=63; i++) {
    if(I2Cdetect->i2cIsPresent(i)) {
      sprintf(buffer, "%s%d", (count>0?",":"") ,((i-55)*8)+57);
      html_str += buffer;
      count++;
    }
  }
  html_str += "];\n";

  //konfigurierte Ports / Namen
  //const configuredPorts = [ {port:65, name:"Ventil1"}, {port:67, name:"Ventil2"}]
  html_str += "const configuredPorts = [";
  for(int i=0; i < pcf8574devCount; i++) {
    sprintf(buffer, "{port:%d, name:'%s'}%s", pcf8574dev[i].port ,pcf8574dev[i].subtopic, (i<pcf8574devCount-1?",":""));
    html_str += buffer;
  }
  html_str += "];\n";
  
  return html_str.c_str();
}
  
String getPageHeader(int pageactive) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  html_str  = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n";
  html_str += "<meta charset='utf-8'>\n";
  html_str += "<link rel='stylesheet' type='text/css' href='/style.css'>\n";
  html_str += "<script language='javascript' type='text/javascript' src='/parameter.js'></script>\n";
  html_str += "<script language='javascript' type='text/javascript' src='/javascript.js'></script>\n";
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
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/PinConfig'>Basis Config</a></td>\n", (pageactive==2)?"navi_active":"");
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
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/Relations'>Relations</a></td>\n", (pageactive==6)?"navi_active":"");
  html_str += buffer;
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  html_str += "   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/ESP8266_PumpControl/wiki' target='_blank'>Wiki</a></td>\n";
  html_str += "   <td class='navi' style='width: 50px'></td>\n";
  html_str += " </tr>\n";
  html_str += "  <tr>\n";
  html_str += "   <td colspan='13'>\n";
  html_str += "<p />\n";

  return html_str.c_str();  
}

void setPage_Footer() {
  html_str += "  </td></tr>";
  html_str += "</table>";
  html_str += "</body>\n";
  html_str += "</html>\n";
}

String getPage_Status() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  bool count = false;

  html_str = getPageHeader(1);
  html_str += "<table class='editorDemoTable'>\n";
  html_str += "<thead>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 250px;'>Name</td>\n";
  html_str += "<td style='width: 200px;'>Wert</td>\n";
  html_str += "</tr>\n";
  html_str += "</thead>\n";
  html_str += "<tbody>\n";
  
  html_str += "<tr>\n";
  html_str += "<td>IP-Adresse:</td>\n";
  sprintf(buffer, "<td>%s</td>\n", WiFi.localIP().toString().c_str());
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>WiFi Name:</td>\n";
  sprintf(buffer, "<td>%s</td>\n", WiFi.SSID().c_str());
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>i2c Bus:</td>\n";
  html_str += "<td>";
  html_str += I2Cdetect->i2cGetPrintOut();
  html_str += "</td>\n";
  html_str += "</tr>\n";
  
  html_str += "<tr>\n";
  html_str += "<td>MAC:</td>\n";
  sprintf(buffer, "<td>%s</td>\n", WiFi.macAddress().c_str());
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>WiFi RSSI:</td>\n";
  sprintf(buffer, "<td>%d</td>\n", WiFi.RSSI());
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Uptime:</td>\n";
  sprintf(buffer, "<td>%d</td>\n", UpTime->getFormatUptime());
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Free Memory:</td>\n";
  sprintf(buffer, "<td>%d</td>\n", ESP.getFreeHeap());
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>aktuell geöffnete Ventile</td>\n";
  html_str += "<td>\n";
  count=false;
  for(int i=0; i < pcf8574devCount; i++) {
    if (pcf8574dev[i].active) {
      sprintf(buffer, "%s noch %d/%d sek<br>\n", pcf8574dev[i].subtopic, (pcf8574dev[i].startmillis+pcf8574dev[i].lengthmillis-millis())/1000, pcf8574dev[i].lengthmillis / 1000);
      html_str += buffer; 
      count=true; 
    }
  }
  if (!count) { html_str += "alle Ventile geschlossen\n"; }
  html_str += "</td></tr>\n";

  if (measureType != NONE) {  
    html_str += "<tr>\n";
    html_str += "<td>Sensor RAW Value:</td>\n";
    sprintf(buffer, "<td>%d %%</td>\n", sensor_RawValue);
    html_str += buffer;
    html_str += "</tr>\n";
  
    html_str += "<tr>\n";
    html_str += "<td>Füllstand in %:</td>\n";
    sprintf(buffer, "<td>%d %%</td>\n", sensor_level);
    html_str += buffer;
    html_str += "</tr>\n";
  }
  
  html_str += "<tr>\n";
  html_str += "<td>Firmware Update</td>\n";
  html_str += "<td><form action='update'><input class='button' type='submit' value='Update' /></form></td>\n";
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Device Reboot</td>\n";
  html_str += "<td><form action='reboot'><input class='button' type='submit' value='Reboot' /></form></td>\n";
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
  html_str += "<td>Device Name</td>\n";
  sprintf(buffer, "<td><input maxlength='40' name='mqttroot' type='text' value='%s'/></td>\n", mqtt_root);
  html_str += buffer;
  html_str += "</tr>\n";
  
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

  if(measureType != NONE) {
    html_str += "<tr>\n";
    html_str += "<td>Pin HC-SR04 Trigger</td>\n";
    sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_0' name='pinhcsr04trigger' type='number' value='%d'/></td>\n", pin_hcsr04_trigger); 
    html_str += buffer;
    html_str += "</tr>\n";
  
    html_str += "<tr>\n";
    html_str += "<td>Pin HC-SR04 Echo</td>\n";
    sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_1' name='pinhcsr04echo' type='number' value='%d'/></td>\n", pin_hcsr04_echo);
    html_str += buffer;
    html_str += "</tr>\n";
  }
  
  html_str += "<tr>\n";
  html_str += "<td>Pin i2c SDA</td>\n";
  sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_2' name='pinsda' type='number' value='%d'/></td>\n", pin_sda);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  html_str += "<td>Pin i2c SCL</td>\n";
  sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_3' name='pinscl' type='number' value='%d'/></td>\n", pin_scl);
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
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  html_str = getPageHeader(3);
  String hide = "";
  
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
  html_str += "  <td colspan='2'>\n";
  sprintf(buffer, "    <div class='inline'><input type='radio' id='sel0' name='selection' value='none' %s onclick=\"radioselection([''],['analog_1','analog_2','hcsr04_1','hcsr04_2'])\"/><label for='sel0'>keine Füllstandsmessung</label></div>\n", (measureType==NONE)?"checked":"");
  html_str += buffer;
  sprintf(buffer, "    <div class='inline'><input type='radio' id='sel1' name='selection' value='hcsr04' %s onclick=\"radioselection(['all_1','hcsr04_1','hcsr04_2'],['analog_1','analog_2'])\"/><label for='sel1'>Füllstandsmessung mit Ultraschallsensor HCSR04</label></div>\n", (measureType==HCSR04)?"checked":"");
  html_str += buffer;
  sprintf(buffer, "    <div class='inline'><input type='radio' id='sel2' name='selection' value='analog' %s onclick=\"radioselection(['all_1','analog_1','analog_2'],['hcsr04_1','hcsr04_2'])\"/><label for='sel2'>Füllstandsmessung mit analogem Signal (an A0)</label></div>\n", (measureType==ANALOG)?"checked":"");
  html_str += buffer;
  
  html_str += "  </td>\n";
  html_str += "</tr>\n";

  if (measureType==NONE) {hide="hide";} else {hide="";}
  sprintf(buffer, "<tr class='%s' id='all_1'>\n", hide.c_str());
  html_str += buffer;
  html_str += "<td>Messintervall</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='measurecycle' type='number' value='%d'/></td>\n", measurecycle);
  html_str += buffer;
  html_str += "</tr>\n";

  if (measureType==HCSR04) {hide="";} else {hide="hide";}
  sprintf(buffer, "<tr class='%s' id='hcsr04_1'>\n", hide.c_str());
  html_str += buffer;
  html_str += "<td>Abstand Sensor min (in cm)</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='measureDistMin' type='number' value='%d'/></td>\n", measureDistMin);
  html_str += buffer;
  html_str += "</tr>\n";

  if (measureType==HCSR04) {hide="";} else {hide="hide";}
  sprintf(buffer, "<tr class='%s' id='hcsr04_2'>\n", hide.c_str());
  html_str += buffer;
  html_str += "<td>Abstand Sensor max (in cm)</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='measureDistMax' type='number' value='%d'/></td>\n", measureDistMax);
  html_str += buffer;
  html_str += "</tr>\n";

  if (measureType==ANALOG) {hide="";} else {hide="hide";}
  sprintf(buffer, "<tr class='%s' id='analog_1'>\n", hide.c_str());
  html_str += buffer;
  html_str += "<td>Kalibrierung: 0% entsricht RAW Wert</td>\n";
  sprintf(buffer, "<td><input min='0' size='5' name='measureDistMin' type='number' value='%d'/></td>\n", measureDistMin);
  html_str += buffer;
  html_str += "</tr>\n";

  if (measureType==ANALOG) {hide="";} else {hide="hide";}
  sprintf(buffer, "<tr class='%s' id='analog_2'>\n", hide.c_str());
  html_str += buffer;
  html_str += "<td>Kalibrierung: 100% entsricht RAW Wert</td>\n";
  sprintf(buffer, "<td><input min='0' max='1024' name='measureDistMax' type='number' value='%d'/></td>\n", measureDistMax);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "</tbody>\n";
  html_str += "</table>\n";
  html_str += "<p><br /><input class='button' type='submit' value='Speichern' /></form>\n";

  setPage_Footer();
  return html_str.c_str();  
}


String getPage_VentilConfig() {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  html_str = getPageHeader(4);

  html_str += "<p><input type='button' value='&#10010; add new Port' onclick='addrow()'></p>\n";
  html_str += "<form id='submitForm'>\n";
  html_str += "<table id='maintable' class='editorDemoTable'>\n";
  html_str += "<thead>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 25px;'>Nr</td>\n";
  html_str += "<td style='width: 25px;'>Active</td>\n";
  html_str += "<td style='width: 250px;'>MQTT SubTopic</td>\n";
  html_str += "<td style='width: 210 px;'>Port</td>\n";
  html_str += "<td style='width: 80px;'>Type</td>\n";
  html_str += "<td style='width: 25px;'>Delete</td>\n";

  html_str += "</tr>\n";
  html_str += "</thead>\n";
  html_str += "<tbody>\n\n";
  
  for(int i=0; i < pcf8574devCount; i++) {
    html_str += "<tr>\n";
    sprintf(buffer, "  <td>%d</td>\n", i+1);
    html_str += buffer;
    html_str += "  <td>\n";
    html_str += "    <div class='onoffswitch'>";
    sprintf(buffer, "      <input type='checkbox' name='active_%d' class='onoffswitch-checkbox' id='myonoffswitch_%d' %s>\n", i, i, (pcf8574dev[i].enabled?"checked":""));
    html_str += buffer;
    sprintf(buffer, "      <label class='onoffswitch-label' for='myonoffswitch_%d'>\n", i);
    html_str += buffer;
    html_str += "        <span class='onoffswitch-inner'></span>\n";
    html_str += "        <span class='onoffswitch-switch'></span>\n";
    html_str += "      </label>\n";
    html_str += "    </div>\n";
    html_str += "  </td>\n";
    
    sprintf(buffer, "  <td><input maxlength='20' name='mqtttopic_%d' type='text' value='%s'/></td>\n", i, pcf8574dev[i].subtopic);
    html_str += buffer;
    sprintf(buffer, "  <td id='tdport_%d'>\n", i);
    html_str += buffer;
    
    if (strcmp(pcf8574dev[i].type, "b")==0) {
      sprintf(buffer, "    <div id='PortA_%d'>\n", i);
      html_str += buffer;
      html_str += "    <div class='inline'>\n";
      sprintf(buffer, "      <input id='AllePorts_%d_0' name='pcfport_%d_0' type='number' min='0' max='220' value='%d'/></div>\n",i, i, pcf8574dev[i].port);
      html_str += buffer;
      html_str += "      <label>for</label>\n";
      sprintf(buffer, "      <input id='imp_%d_0' name='imp_%d_0'  value='%d' type='number' min='10' max='999'/>\n",i, i, pcf8574dev[i].portms);
      html_str += buffer;
      html_str += "      <label>ms</label>\n";
      html_str += "    </div>\n";
      
      sprintf(buffer, "    <div id='PortB_%d'>\n", i);
      html_str += buffer;
      html_str += "      <div class='inline'>\n";
      sprintf(buffer, "      <input id='AllePorts_%d_1' name='pcfport_%d_1' type='number' min='0' max='220' value='%d'/></div>\n",i, i, pcf8574dev[i].port2);
      html_str += buffer;
      html_str += "      <label>for</label>\n";
      sprintf(buffer, "      <input id='imp_%d_1' name='imp_%d_1'  value='%d' type='number' min='10' max='999'/>\n",i, i, pcf8574dev[i].port2ms);
      html_str += buffer;
      html_str += "      <label>ms</label>\n";
      html_str += "    </div>\n";
    } else if (strcmp(pcf8574dev[i].type, "n")==0) {
      sprintf(buffer, "    <div id='Port_%d'>\n", i);
      html_str += buffer;
      sprintf(buffer, "      <input id='AllePorts_%d' name='pcfport_%d_0' type='number' min='0' max='220' value='%d'/>\n",i, i, pcf8574dev[i].port);
      html_str += buffer;
      html_str += "    </div>\n";
    } else if (strcmp(pcf8574dev[i].type, "n")==0) {
      //do nothing
    }
    html_str += "  </td>\n";
    html_str += "  <td>\n";
    sprintf(buffer, "    <div class='inline'><input type='radio' id='type_%d_0' name='type_%d' value='n' %s onclick='chg_type(this.id)' /><label for='type_%d_0'>normal</label></div>\n",i, i, ((strcmp(pcf8574dev[i].type, "n")==0)?"checked":""),i);
    html_str += buffer;
    sprintf(buffer, "    <div class='inline'><input type='radio' id='type_%d_1' name='type_%d' value='b' %s onclick='chg_type(this.id)' /><label for='type_%d_1'>bistabil</label></div>\n",i, i, ((strcmp(pcf8574dev[i].type, "b")==0)?"checked":""),i);
    html_str += buffer;
    sprintf(buffer, "    <div class='inline'><input type='radio' id='type_%d_2' name='type_%d' value='v' %s onclick='chg_type(this.id)' /><label for='type_%d_2'>virtual</label></div>\n",i, i, ((strcmp(pcf8574dev[i].type, "v")==0)?"checked":""),i);
    html_str += buffer;
    
    html_str += "  </td>\n";
    html_str += "  <td><input type='button' value='&#10008;' onclick='delrow(this)'></td>\n";
    html_str += "</tr>\n";
  }
  html_str += "</tbody>\n";
  html_str += "</table>\n";
  html_str += "</form>\n\n<br />\n";
  html_str += "<form id='jsonform' action='StoreVentilConfig' method='POST' onsubmit='return onSubmit()'>\n";
  html_str += "  <input type='text' id='json' name='json' />\n";
  html_str += "  <input type='submit' value='Speichern' />\n";
  html_str += "</form>\n\n";
  html_str += "<div id='ErrorText' class='errortext'></div>\n";
    
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
  html_str += "<td style='text-align: center;'>&nbsp;</td>\n";
  html_str += "<td >Sensor Treshold Min</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='treshold_min' type='number' value='%d'/></td>\n", treshold_min);
  html_str += buffer;
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  html_str += "<td style='text-align: center;'>&nbsp;</td>\n";
  html_str += "<td>Sensor Treshold Max</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' name='treshold_max' type='number' value='%d'/></td>\n", treshold_max);
  html_str += buffer;
  html_str += "</tr>\n";
  html_str += "<tr>\n";
  
  sprintf(buffer, "<td style='text-align: center;'><input name='enable_syncswitch' type='checkbox' value='1' %s /></td>\n", (enable_syncswitch?"checked":""));
  html_str += buffer;
  html_str += "<td>Ventil Trinkwasser Bypass</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' id='ConfiguredPorts_0' name='syncswitch_port' type='number' value='%d'/></td>\n", syncswitch_port);
  html_str += buffer;
  html_str += "</tr>\n";

  html_str += "<tr>\n";
  sprintf(buffer, "<td style='text-align: center;'><input name='enable_3wege' type='checkbox' value='1' %s /></td>\n", (enable_3wege?"checked":""));
  html_str += buffer;
  html_str += "<td>3WegeVentil Trinkwasser Bypass</td>\n";
  sprintf(buffer, "<td><input min='0' max='254' id='ConfiguredPorts_1' name='ventil3wege_port' type='number' value='%d'/></td>\n", ventil3wege_port);
  html_str += buffer;
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

String getPage_Relations() {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  html_str = getPageHeader(6);

  html_str += "<p><input type='button' value='&#10010; add new Port' onclick='addrow()'></p>\n";
  html_str += "<form id='submitForm'>\n";
  html_str += "<table id='maintable' class='editorDemoTable'>\n";
  html_str += "<thead>\n";
  html_str += "<tr>\n";
  html_str += "<td style='width: 25px;'>Nr</td>\n";
  html_str += "<td style='width: 25px;'>Active</td>\n";
  html_str += "<td style='width: 250px;'>Trigger Topic</td>\n";
  html_str += "<td style='width: 250px;'>Port</td>\n";
  html_str += "<td style='width: 25px;'>Delete</td>\n";

  html_str += "</tr>\n";
  html_str += "</thead>\n";
  html_str += "<tbody>\n\n";

  for(int i=0; i < valveRelCount; i++) {
    html_str += "<tr>\n";
    sprintf(buffer, "  <td>%d</td>\n", i+1);
    html_str += buffer;
    html_str += "  <td>\n";
    html_str += "    <div class='onoffswitch'>";
    sprintf(buffer, "      <input type='checkbox' name='active_%d' class='onoffswitch-checkbox' id='myonoffswitch_%d' %s>\n", i, i, (valveRel[i].enabled?"checked":""));
    html_str += buffer;
    sprintf(buffer, "      <label class='onoffswitch-label' for='myonoffswitch_%d'>\n", i);
    html_str += buffer;
    html_str += "        <span class='onoffswitch-inner'></span>\n";
    html_str += "        <span class='onoffswitch-switch'></span>\n";
    html_str += "      </label>\n";
    html_str += "    </div>\n";
    html_str += "  </td>\n";

    sprintf(buffer, "  <td><input id='mqtttopic_%d' name='mqtttopic_%d' type='text' size='10' value='%s'/></td>\n", i, i, valveRel[i].portA->subtopic);
    html_str += buffer;
    sprintf(buffer, "  <td><input id='ConfiguredTopics_%d' name='port_%d' type='text' size='10' value='%s'/></td>\n", i, i, valveRel[i].portB->subtopic );
    html_str += buffer;
    html_str += "  <td><input type='button' value='&#10008;' onclick='delrow(this)'></td>\n";
    html_str += "</tr>\n";
  }

  html_str += "</tbody>\n";
  html_str += "</table>\n";
  html_str += "</form>\n\n<br />\n";
  html_str += "<form id='jsonform' action='StoreRelations' method='POST' onsubmit='return onSubmit()'>\n";
  html_str += "  <input type='text' id='json' name='json' />\n";
  html_str += "  <input type='submit' value='Speichern' />\n";
  html_str += "</form>\n\n";
  html_str += "<div id='ErrorText' class='errortext'></div>\n";
  
  setPage_Footer();
  return html_str.c_str();
}

