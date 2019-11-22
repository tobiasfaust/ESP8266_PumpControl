#include "WebServer.h"

WebServer::WebServer() : DoReboot(false) {
  server = new ESP8266WebServer(80);

  if (!MDNS.begin("esp82660"))  {  Serial.println(F("Error setting up MDNS responder!"));  }
  else                          {  Serial.println(F("mDNS responder started"));  }

  httpUpdater.setup(server);
  server->begin(); 

  server->onNotFound([this]() { this->handleNotFound(); });
  server->on("/", [this]() {this->handleRoot(); });
  server->on("/BaseConfig", [this]() {this->handleBaseConfig(); });
  server->on("/SensorConfig", [this]() {this->handleSensorConfig(); });
  server->on("/VentilConfig", [this]() {this->handleVentilConfig(); });
  //server->on("/AutoConfig", [this]() {this->handleAutoConfig(); });
  server->on("/Relations", [this]() {this->handleRelations(); });
  
  server->on("/style.css", HTTP_GET, [this]() {this->handleCSS(); });
  server->on("/javascript.js", HTTP_GET, [this]() {this->handleJS(); });
  server->on("/parameter.js", HTTP_GET, [this]() {this->handleJSParam(); });
  
  server->on("/StoreBaseConfig", HTTP_POST, [this]() { this->ReceiveJSONConfiguration(BASECONFIG); });
  server->on("/StoreSensorConfig", HTTP_POST, [this]() { this->ReceiveJSONConfiguration(SENSOR); });
  server->on("/StoreVentilConfig", HTTP_POST, [this]() { this->ReceiveJSONConfiguration(VENTILE); });
  //server->on("/StoreAutoConfig", HTTP_POST, handleStoreAutoConfig);
  server->on("/StoreRelations", HTTP_POST, [this]() { this->ReceiveJSONConfiguration(RELATIONS); });
  server->on("/reboot", HTTP_GET, [this]() { this->handleReboot(); });
  
  Serial.println(F("WebServer started..."));

  UpTime = new uptime();
}

void WebServer::loop() {
  server->handleClient();
  //UpTime->loop(); -> verursacht ESP Absturz
  if (this->DoReboot) {ESP.restart();}
}

/*void WebServer::SetHTMLFunction_Valve (CB_HTML_VALVE) {
  Serial.println("CallBack Function Start");
  this->htmlValve = htmlValve;
}*/

void WebServer::handleNotFound() {
  server->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void WebServer::handleRoot() {
  String html;
  this->getPageHeader(&html, ROOT);
  this->getPage_Status(&html);
  this->getPageFooter(&html);
  server->send(200, "text/html", html.c_str());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void WebServer::handleCSS() {
  server->send(200, "text/css", STYLE_CSS);
}

void WebServer::handleJS() {
  server->send(200, "text/javascript", JAVASCRIPT);
}

void WebServer::handleJSParam() {
  String html;
  VStruct->getWebJsParameter(&html);
  server->send(200, "text/javascript", html.c_str());
}

void WebServer::handleReboot() {
  server->sendHeader("Location","/");
  server->send(303); 
  this->DoReboot = true;  
}

void WebServer::handleBaseConfig() {
  String html;
  this->getPageHeader(&html, BASECONFIG);
  Config->GetWebContent(&html);
  this->getPageFooter(&html);
  server->send(200, "text/html", html.c_str());   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void WebServer::handleVentilConfig() {
  String html;
  this->getPageHeader(&html, VENTILE);
  VStruct->GetWebContent(&html);
  this->getPageFooter(&html);
  server->send(200, "text/html", html.c_str());
}

void WebServer::handleSensorConfig() {
  String html;
  this->getPageHeader(&html, SENSOR);
  LevelSensor->GetWebContent(&html);
  this->getPageFooter(&html);
  server->send(200, "text/html", html.c_str());
}

void WebServer::handleRelations() {
  String html;
  this->getPageHeader(&html, RELATIONS);
  ValveRel->GetWebContent(&html);
  this->getPageFooter(&html);
  server->send(200, "text/html", html.c_str());
}


void WebServer::ReceiveJSONConfiguration(page_t page) {
  String json = server->arg("json");
  String targetPage = "/";
  Serial.print(F("json empfangen: "));
  Serial.println(FPSTR(json.c_str()));  
  
  if (page==BASECONFIG) { Config->StoreJsonConfig(&json); targetPage = "/BaseConfig"; }
  if (page==VENTILE)    { VStruct->StoreJsonConfig(&json); targetPage = "/VentilConfig"; }
  if (page==SENSOR)     { LevelSensor->StoreJsonConfig(&json); targetPage = "/SensorConfig"; }
  if (page==RELATIONS)  { ValveRel->StoreJsonConfig(&json); targetPage = "/Relations"; }

  server->sendHeader("Location", targetPage.c_str());
  server->send(303); 
}

void WebServer::getPageHeader(String* html, page_t pageactive) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  html->concat("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n");
  html->concat("<meta charset='utf-8'>\n");
  html->concat("<link rel='stylesheet' type='text/css' href='/style.css'>\n");
  html->concat("<script language='javascript' type='text/javascript' src='/parameter.js'></script>\n");
  html->concat("<script language='javascript' type='text/javascript' src='/javascript.js'></script>\n");
  html->concat("<title>Bewässerungssteuerung</title></head>\n");
  html->concat("<body>\n");
  html->concat("<table>\n");
  html->concat("  <tr>\n");
  html->concat("   <td colspan='13'>\n");
  html->concat("     <h2>Konfiguration</h2>\n");
  html->concat("   </td>\n");
  html->concat(" </tr>\n");
  html->concat(" <tr>\n");
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==ROOT)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/BaseConfig'>Basis Config</a></td>\n", (pageactive==BASECONFIG)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/SensorConfig'>Sensor Config</a></td>\n", (pageactive==SENSOR)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/VentilConfig'>Ventil Config</a></td>\n", (pageactive==VENTILE)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/Relations'>Relations</a></td>\n", (pageactive==RELATIONS)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  html->concat("   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/ESP8266_PumpControl/wiki' target='_blank'>Wiki</a></td>\n");
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  html->concat(" </tr>\n");
  html->concat("  <tr>\n");
  html->concat("   <td colspan='11'>\n");
  html->concat("   <p />\n");
}

void WebServer::getPageFooter(String* html) {
  /*html->concat("  </td></tr>\n");
  html->concat("</table>\n");
  html->concat("</body>\n");
*/
  html->concat("</html>\n");
}

void WebServer::getPage_Status(String* html) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  uint8_t count = 0;

  html->concat("<table class='editorDemoTable'>\n");
  html->concat("<thead>\n");
  html->concat("<tr>\n");
  html->concat("<td style='width: 250px;'>Name</td>\n");
  html->concat("<td style='width: 200px;'>Wert</td>\n");
  html->concat("</tr>\n");
  html->concat("</thead>\n");
  html->concat("<tbody>\n");

  html->concat("<tr>\n");
  html->concat("<td>IP-Adresse:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.localIP().toString().c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>WiFi Name:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.SSID().c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>i2c Bus:</td>\n");
  html->concat("<td>");
  html->concat(I2Cdetect->i2cGetAddresses());
  html->concat("</td>\n");
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>MAC:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.macAddress().c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>WiFi RSSI:</td>\n");
  sprintf(buffer, "<td>%d</td>\n", WiFi.RSSI());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Uptime:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", "---"); //UpTime->getFormatUptime());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Free Heap Memory:</td>\n");
  sprintf(buffer, "<td>%d</td>\n", ESP.getFreeHeap()); //https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/heap_debug.html
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>aktuell geöffnete Ventile</td>\n");
  html->concat("<td>\n");

  count = VStruct->CountActiveThreads();
  if (count > 0) {
    sprintf(buffer, "Es sind %d Ventile offen", count);
    html->concat(buffer);
  } else { html->concat("alle Ventile geschlossen\n"); }
  html->concat("</td></tr>\n");

  if (LevelSensor->GetType() != NONE && LevelSensor->GetType() != EXTERN) {  
    html->concat("<tr>\n");
    html->concat("<td>Sensor RAW Value:</td>\n");
    sprintf(buffer, "<td>%d %%</td>\n", LevelSensor->GetRaw());
    html->concat(buffer);
    html->concat("</tr>\n");
  }
  if (LevelSensor->GetType() != NONE) {  
    html->concat("<tr>\n");
    html->concat("<td>Füllstand in %:</td>\n");
    sprintf(buffer, "<td>%d %%</td>\n", LevelSensor->GetLvl());
    html->concat(buffer);
    html->concat("</tr>\n");
  }

  html->concat("<tr>\n");
  html->concat("<td>Firmware Update</td>\n");
  html->concat("<td><form action='update'><input class='button' type='submit' value='Update' /></form></td>\n");
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Device Reboot</td>\n");
  html->concat("<td><form action='reboot'><input class='button' type='submit' value='Reboot' /></form></td>\n");
  html->concat("</tr>\n");

  html->concat("</tbody>\n");
  html->concat("</table>\n");     
}

