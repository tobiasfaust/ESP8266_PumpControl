#include "MyWebServer.h" 

MyWebServer::MyWebServer(AsyncWebServer *server, DNSServer* dns): server(server), dns(dns), DoReboot(false) {
  server->begin();

  server->onNotFound(std::bind(&MyWebServer::handleNotFound, this, std::placeholders::_1));
  server->on("/", HTTP_GET, std::bind(&MyWebServer::handleRoot, this, std::placeholders::_1));
  server->on("/BaseConfig", HTTP_GET, std::bind(&MyWebServer::handleBaseConfig, this, std::placeholders::_1));
  server->on("/SensorConfig", HTTP_GET, std::bind(&MyWebServer::handleSensorConfig, this, std::placeholders::_1));
  server->on("/VentilConfig", HTTP_GET, std::bind(&MyWebServer::handleVentilConfig, this, std::placeholders::_1));
  server->on("/1WireConfig", HTTP_GET, std::bind(&MyWebServer::handle1WireConfig, this, std::placeholders::_1));
  server->on("/Relations", HTTP_GET, std::bind(&MyWebServer::handleRelations, this, std::placeholders::_1));
  
  server->on("/style.css", HTTP_GET, std::bind(&MyWebServer::handleCSS, this, std::placeholders::_1));
  server->on("/javascript.js", HTTP_GET, std::bind(&MyWebServer::handleJS, this, std::placeholders::_1));
  server->on("/jsajax.js", HTTP_GET, std::bind(&MyWebServer::handleJsAjax, this, std::placeholders::_1));
  server->on("/parameter.js", HTTP_GET, std::bind(&MyWebServer::handleJSParam, this, std::placeholders::_1));
  
  server->on("/StoreBaseConfig", HTTP_POST, std::bind(&MyWebServer::ReceiveJSONConfiguration, this, std::placeholders::_1, BASECONFIG));
  server->on("/StoreSensorConfig", HTTP_POST, std::bind(&MyWebServer::ReceiveJSONConfiguration, this, std::placeholders::_1, SENSOR ));
  server->on("/StoreVentilConfig", HTTP_POST, std::bind(&MyWebServer::ReceiveJSONConfiguration, this, std::placeholders::_1, VENTILE));
  server->on("/StoreRelations", HTTP_POST, std::bind(&MyWebServer::ReceiveJSONConfiguration, this, std::placeholders::_1, RELATIONS));
  server->on("/reboot", HTTP_GET, std::bind(&MyWebServer::handleReboot, this, std::placeholders::_1));
  server->on("/reset", HTTP_GET, std::bind(&MyWebServer::handleReset, this, std::placeholders::_1));
  server->on("/wifireset", HTTP_GET, std::bind(&MyWebServer::handleWiFiReset, this, std::placeholders::_1));

  
  server->on("/ajax", HTTP_POST, std::bind(&MyWebServer::handleAjax, this, std::placeholders::_1));
  
  Serial.println(F("WebServer started..."));
}

void MyWebServer::handle_update_page(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML_UPDATEPAGE);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response); 
}

void MyWebServer::handle_update_response(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML_UPDATERESPONSE);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response); 
}

void MyWebServer::handle_update_progress(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
      //Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
  }
  if(!Update.hasError()){
    if(Update.write(data, len) != len){
        Update.printError(Serial);
    }
  }
  if(final){
    if(Update.end(true)){
      Serial.printf("Update Success: %uB\n", index+len);
      this->DoReboot = true;//Set flag so main loop can issue restart call
    } else {
      Update.printError(Serial);
    }
  }
}

void MyWebServer::loop() {
  //delay(1); // slow response Issue: https://github.com/espressif/arduino-esp32/issues/4348#issuecomment-695115885
  if (this->DoReboot) {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
}

void MyWebServer::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void MyWebServer::handleRoot(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  
  this->getPageHeader(response, ROOT);
  this->getPage_Status(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleCSS(AsyncWebServerRequest *request) {
  request->send_P(200, "text/css", STYLE_CSS);
}

void MyWebServer::handleJS(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", JAVASCRIPT);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response); 
}

void MyWebServer::handleJsAjax(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", JSAJAX);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response); 
}

void MyWebServer::handleJSParam(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  VStruct->getWebJsParameter(response);
  request->send(response);
}

void MyWebServer::handleReboot(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML_UPDATERESPONSE);
  response->addHeader("Connection", "close");
  request->send(response);
  
  this->DoReboot = true;
}

void MyWebServer::handleReset(AsyncWebServerRequest *request) {
  SPIFFS.format();
  this->handleReboot(request);
}

void MyWebServer::handleWiFiReset(AsyncWebServerRequest *request) {
  #ifdef ESP32
    WiFi.disconnect(true,true);
  #elif ESP8266  
    ESP.eraseConfig();
  #endif
  
  this->handleReboot(request);
}


void MyWebServer::handleBaseConfig(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  this->getPageHeader(response, BASECONFIG);
  Config->GetWebContent(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleVentilConfig(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  this->getPageHeader(response, VENTILE);
  VStruct->GetWebContent(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handle1WireConfig(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  this->getPageHeader(response, ONEWIRE);
  VStruct->GetWebContent1Wire(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleSensorConfig(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  this->getPageHeader(response, SENSOR);
  LevelSensor->GetWebContent(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleRelations(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  this->getPageHeader(response, RELATIONS);
  ValveRel->GetWebContent(response);
  this->getPageFooter(response);
  request->send(response);
}


void MyWebServer::ReceiveJSONConfiguration(AsyncWebServerRequest *request, page_t page) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");
  String json = "{}";

  if(request->hasArg("json")) {
    json = request->arg("json");
  }

  String targetPage = "/";
  Serial.print(F("json empfangen: "));
  Serial.println(FPSTR(json.c_str()));  
  
  if (page==BASECONFIG)       { Config->StoreJsonConfig(&json);   targetPage = "/BaseConfig"; }
  if (page==VENTILE)          { VStruct->StoreJsonConfig(&json);  targetPage = "/VentilConfig"; }
  if (page==SENSOR)           { LevelSensor->StoreJsonConfig(&json); targetPage = "/SensorConfig"; }
  if (page==RELATIONS)        { ValveRel->StoreJsonConfig(&json); targetPage = "/Relations"; }

  request->redirect(targetPage);
}

void MyWebServer::handleAjax(AsyncWebServerRequest *request) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  String ret;
  bool RaiseError = false;
  String action, newState; 
  String json = "{}";
  uint8_t port = 0;

  AsyncResponseStream *response = request->beginResponseStream("text/json");
  response->addHeader("Server","ESP Async Web Server");

  if(request->hasArg("json")) {
    json = request->arg("json");
  }

  StaticJsonDocument<512> jsonGet; // TODO Use computed size??
  DeserializationError error = deserializeJson(jsonGet, json.c_str());

  StaticJsonDocument<256> jsonReturn;

  if (Config->GetDebugLevel() >=4) { Serial.print("Ajax Json Empfangen: "); }
  if (!error) {
    if (Config->GetDebugLevel() >=4) { serializeJsonPretty(jsonGet, Serial); Serial.println(); }

    if (jsonGet.containsKey("action"))   {action = jsonGet["action"].as<String>();}
    if (jsonGet.containsKey("newState")) { newState = jsonGet["newState"].as<String>(); }
    if (jsonGet.containsKey("port"))     { port = atoi(jsonGet["port"]); }
  
  } else { RaiseError = true; }

  if (RaiseError) {
    jsonReturn["error"] = buffer;
    serializeJson(jsonReturn, ret);
    response->print(ret);

    if (Config->GetDebugLevel() >=2) {
      snprintf(buffer, sizeof(buffer), "Ajax Json Command not parseable: %s -> %s", json.c_str(), error.c_str());
      Serial.println(FPSTR(buffer));
    }
    
  } else if (action && strcmp(action.c_str(), "SetValve")==0) {
      if (newState && port && port > 0 && !VStruct->GetEnabled(port)) { jsonReturn["accepted"] = 0; jsonReturn["error"] = "Requested Port not enabled. Please enable first!";}
      else if (newState && port && port > 0 && strcmp(newState.c_str(),"On")==0)  { VStruct->SetOn(port); jsonReturn["accepted"] = 1;}
      else if (newState && port && port > 0 && strcmp(newState.c_str(),"Off")==0) { VStruct->SetOff(port); jsonReturn["accepted"] = 1;}
      else { RaiseError = true; }

      if (port && port > 0) {jsonReturn["NewState"] = (VStruct->GetState(port)?"On":"Off");}
  } else if (action && strcmp(action.c_str(), "EnableValve")==0) {
      if (port && port > 0 && newState) {
        if (strcmp(newState.c_str(),"true")==0) VStruct->SetEnable(port, true);
        if (strcmp(newState.c_str(),"false")==0) VStruct->SetEnable(port, false);
        jsonReturn["NewState"] = (VStruct->GetEnabled(port)?"true":"false");
        jsonReturn["accepted"] = 1;
      }
  } else if (action && newState && strcmp(action.c_str(), "InstallRelease")==0) {
      Config->InstallRelease(atoi(newState.c_str()));  
      jsonReturn["accepted"] = 1;  
  } else if (action && strcmp(action.c_str(), "RefreshReleases")==0) {
      Config->RefreshReleases();  
      jsonReturn["accepted"] = 1;  
  } else if (action && strcmp(action.c_str(), "RefreshI2C")==0) {
      I2Cdetect->i2cScan();  
      jsonReturn["NewState"] = I2Cdetect->i2cGetAddresses();
      jsonReturn["accepted"] = 1;  
  } else if (action && strcmp(action.c_str(), "Refresh1Wire")==0) {
      uint8_t ret = VStruct->Refresh1WireDevices();  
      snprintf(buffer, sizeof(buffer), "%d (%d)", ret, ret * 8);
      jsonReturn["NewState"] = buffer;
      jsonReturn["accepted"] = 1;  
  } else {
    if (Config->GetDebugLevel() >=1) {
      snprintf(buffer, sizeof(buffer), "Ajax Command unknown: %s", action);
      Serial.println(buffer);
  }
    serializeJson(jsonReturn, ret);
    response->print(ret);
  }

  if (Config->GetDebugLevel() >=4) { Serial.print("Ajax Json Antwort: "); Serial.println(ret); }
  
  
  request->send(response);
}

void MyWebServer::getPageHeader(AsyncResponseStream *response, page_t pageactive) {
  response->println("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>");
  response->println("<meta charset='utf-8'>");
  response->println("<link rel='stylesheet' type='text/css' href='/style.css'>");
  response->println("<script language='javascript' type='text/javascript' src='/parameter.js'></script>");
  response->println("<script language='javascript' type='text/javascript' src='/javascript.js'></script>");
  response->println("<script language='javascript' type='text/javascript' src='/jsajax.js'></script>");
  response->println("<title>Bewässerungssteuerung</title></head>");
  response->println("<body>");
  response->println("<table>");
  response->println("  <tr>");
  response->println("   <td colspan='4'>");
  response->println("     <h2>Konfiguration</h2>");
  response->println("   </td>");

  response->println("   <td colspan='4' style='color:#CCCCCC;'>");
  response->printf("   <i>(%s)</i>\n", Config->GetMqttRoot().c_str());
  response->println("   </td>");

  response->println("   <td colspan='5'>");
  response->printf("     <b>Release: </b><span style='color:orange;'>%s</span><br>of %s %s", Config->GetReleaseName().c_str(), __DATE__, __TIME__);
  response->println("   </td>");
  response->println(" </tr>");

  response->println(" <tr>");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==ROOT)?"navi_active":"");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/BaseConfig'>Basis Config</a></td>\n", (pageactive==BASECONFIG)?"navi_active":"");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/SensorConfig'>Sensor Config</a></td>\n", (pageactive==SENSOR)?"navi_active":"");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/VentilConfig'>Ventil Config</a></td>\n", (pageactive==VENTILE)?"navi_active":"");
  
  if (Config->Enabled1Wire()) {
      response->println("   <td class='navi' style='width: 50px'></td>");
      response->printf("   <td class='navi %s' style='width: 100px'><a href='/1WireConfig'>OneWire</a></td>\n", (pageactive==ONEWIRE)?"navi_active":"");
  }
  
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/Relations'>Relations</a></td>\n", (pageactive==RELATIONS)?"navi_active":"");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->println("   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/ESP8266_PumpControl/wiki' target='_blank'>Wiki</a></td>");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->println(" </tr>");
  response->println("  <tr>");
  response->println("   <td colspan='13'>");
  response->println("   <p />");
}

void MyWebServer::getPageFooter(AsyncResponseStream *response) {
  response->println("</table>");
  response->println("<div id='ErrorText' class='errortext'></div>");
  response->println("</body>");
  response->println("</html>");
}

void MyWebServer::getPage_Status(AsyncResponseStream *response) {
  uint8_t count = 0;
  uptime::calculateUptime();
  
  response->println("<table class='editorDemoTable'>");
  response->println("<thead>");
  response->println("<tr>");
  response->println("<td style='width: 250px;'>Name</td>");
  response->println("<td style='width: 200px;'>Wert</td>");
  response->println("</tr>");
  response->println("</thead>");
  response->println("<tbody>");

  response->println("<tr>");
  response->println("<td>IP-Adresse:</td>");
  response->printf("<td>%s</td>\n", WiFi.localIP().toString().c_str());
  response->println("</tr>");

  response->println("<tr>");
  response->println("<td>WiFi Name:</td>");
  response->printf("<td>%s</td>\n", WiFi.SSID().c_str());
  response->println("</tr>");

  response->println("<tr>");
  response->println("<td>i2c Bus:");
    //https://fdossena.com/?p=html5cool/buttons/i.frag
  response->println("<a href='#' onclick=\"RefreshI2C('showI2C')\" class='ButtonRefresh'>&#8634;</a>");
  response->println("</td>");
  response->println("<td><div id='showI2C'>");
  response->printf("%s \n", I2Cdetect->i2cGetAddresses().c_str());
  response->println("</div></td>");
  response->println("</tr>");

  if (Config->Enabled1Wire()) {
    response->println("<tr>");
    response->println("<td>gefundene 1Wire Controller (Devices):");
    response->println("<a href='#' onclick=\"Refresh1Wire('show1Wire')\" class='ButtonRefresh'>&#8634;</a>");
    response->println("</td>");
    response->println("<td><div id='show1Wire'>");
    response->printf("%d (%d)", VStruct->Get1WireCountDevices(), VStruct->Get1WireCountDevices()*8);
    response->println("</div></td>");
    response->println("</tr>");
  }
  
  response->println("<tr>");
  response->println("<td>MAC:</td>");
  response->printf("<td>%s</td>\n", WiFi.macAddress().c_str());
  response->println("</tr>");

  response->println("<tr>");
  response->println("<td>WiFi RSSI:</td>");
  response->printf("<td>%d</td>\n", WiFi.RSSI());
  response->println("</tr>");

  response->println("<tr>");
  response->println("<td>MQTT Status:</td>");
  response->printf("<td>%s</td>\n", (mqtt->GetConnectStatusMqtt()?"Connected":"Not Connected"));
  response->println("</tr>");

  response->println("<tr>");
  response->println("<td>Uptime:</td>");
  response->printf("<td>%lu Days, %lu Hours, %lu Minutes</td>\n", uptime::getDays(), uptime::getHours(), uptime::getMinutes()); //uptime_formatter::getUptime().c_str()); //UpTime->getFormatUptime());
  response->println("</tr>");

  response->println("<tr>");
  response->println("<td>Free Heap Memory:</td>");
  response->printf("<td>%d</td>\n", ESP.getFreeHeap()); //https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/heap_debug.html
  response->println("</tr>");

  response->println("<tr>");
  response->println("<td>aktuell geöffnete Ventile</td>");
  response->println("<td>");

  count = VStruct->CountActiveThreads();
  if (count > 0) {
    response->printf("Es sind %d Ventile offen", count);
  } else { response->println("alle Ventile geschlossen"); }
  response->println("</td></tr>");

  if (LevelSensor->GetType() != NONE && LevelSensor->GetType() != EXTERN) {  
    response->println("<tr>");
    response->println("<td>Sensor RAW Value:</td>");
    response->printf("<td>%d</td>\n", LevelSensor->GetRaw());
    response->println("</tr>");
  }
  if (LevelSensor->GetType() != NONE) {  
    response->println("<tr>");
    response->println("<td>Füllstand in %:</td>");
    response->printf("<td>%d %%</td>\n", LevelSensor->GetLvl());
    response->println("</tr>");
  }

  response->println("<tr>");
  response->println("  <td>Firmware Update</td>");
  response->println("  <td><form action='update'><input class='button' type='submit' value='Update' /></form></td>");
  response->println("</tr>");

  response->println("<tr>");
  response->println("  <td>Device Reboot</td>");
  response->println("  <td><form action='reboot'><input class='button' type='submit' value='Reboot' /></form></td>");
  response->println("</tr>");

  response->println("<tr>");
  response->println("  <td>Werkszustand herstellen (ohne WiFi)</td>");
  response->println("  <td><form action='reset'><input class='button' type='submit' value='Reset' /></form></td>");
  response->println("</tr>");

  response->println("<tr>");
  response->println("  <td>WiFi Zugangsdaten entfernen</td>");
  response->println("  <td><form action='wifireset'><input class='button' type='submit' value='WifiReset' /></form></td>");
  response->println("</tr>");
  
  response->println("</tbody>");
  response->println("</table>");     
}
