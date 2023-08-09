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
  request->send_P(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void MyWebServer::handleRoot(AsyncWebServerRequest *request) {
  // https://github.com/me-no-dev/ESPAsyncWebServer/issues/1185
  std::shared_ptr<uint16_t> processedRows = std::make_shared<uint16_t>(0);
 
  AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [this, processedRows](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
    size_t len = 0;
    size_t currentRow = 0;
    maxLen = maxLen >> 1; // prevent memory issues

    this->getPageHeader2(buffer, processedRows, currentRow, len, maxLen, ROOT);
    this->getPageStatus2(buffer, processedRows, currentRow, len, maxLen);
    this->getPageFooter2(buffer, processedRows, currentRow, len, maxLen);
    
    return len;
	});

  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
	request->send(response);
}

void MyWebServer::handleCSS(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", STYLE_CSS);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response); 
}

void MyWebServer::handleJS(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", JAVASCRIPT);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response); 

  //response->addHeader("Server","ESP Async Web Server");
  //Serial.println("Start handle ESPGPIO");
  //request->send_P(200, "text/html", ESPGPIO);
  //Serial.println("Start handle ESPANALOG");
  //request->send_P(200, "text/html", ESPANALOG);
  //Serial.println("Start handle JAVASCRIPT");
  //request->send_P(200, "text/html", JAVASCRIPT);
  //Serial.println("End Response");
  //request->send(response);
}

void MyWebServer::handleJsAjax(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", JSAJAX);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response); 
}

void MyWebServer::handleJSParam(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/javascript");
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
  LittleFS.format();
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
  std::shared_ptr<uint16_t> processedRows = std::make_shared<uint16_t>(0);
  AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [this, processedRows](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
    size_t len = 0;
    size_t currentRow = 0;
    maxLen = maxLen >> 1; // prevent memory issues

    this->getPageHeader2(buffer, processedRows, currentRow, len, maxLen, BASECONFIG);
    Config->GetWebContent(buffer, processedRows, currentRow, len, maxLen);
    this->getPageFooter2(buffer, processedRows, currentRow, len, maxLen);
    
    return len;
	});

  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
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
  if (Config->GetDebugLevel() >= 3) {
    Serial.print(F("json empfangen: "));
    Serial.println(FPSTR(json.c_str()));  
  }

  if (page==BASECONFIG)       { Config->StoreJsonConfig(&json);   targetPage = "/BaseConfig"; }
  if (page==VENTILE)          { VStruct->StoreJsonConfig(&json);  targetPage = "/VentilConfig"; }
  if (page==SENSOR)           { LevelSensor->StoreJsonConfig(&json); targetPage = "/SensorConfig"; }
  if (page==RELATIONS)        { ValveRel->StoreJsonConfig(&json); targetPage = "/Relations"; }

  request->redirect(targetPage);
}

void MyWebServer::handleAjax(AsyncWebServerRequest *request) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  String ret = (char*)0;
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
    if (jsonGet.containsKey("port"))     { port = jsonGet["port"].as<int>(); }
  
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
      snprintf(buffer, sizeof(buffer), "Ajax Command unknown: %s", action.c_str());
      Serial.println(buffer);
    }
  }
  
  serializeJson(jsonReturn, ret);
  response->print(ret);
  
  if (Config->GetDebugLevel() >=4) { Serial.print("Ajax Json Antwort: "); Serial.println(ret); }
  
  
  request->send(response);
}

size_t MyWebServer::getPageHeader2(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen, page_t pageactive) {
  TT("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n");
  TT("<meta charset='utf-8'>\n");
  TT("<link rel='stylesheet' type='text/css' href='https://cdn.jsdelivr.net/gh/tobiasfaust/" GIT_REPO "@" GIT_BRANCH "/web/style.css'>\n");
  TT("<script language='javascript' type='text/javascript' src='https://cdn.jsdelivr.net/gh/tobiasfaust/" GIT_REPO "@" GIT_BRANCH "/web/javascript.js'></script>\n");
  TT("<script language='javascript' type='text/javascript' src='https://cdn.jsdelivr.net/gh/tobiasfaust/" GIT_REPO "@" GIT_BRANCH "/web/jsajax.js'></script>\n");
  TT("<script language='javascript' type='text/javascript' src='https://cdn.jsdelivr.net/gh/tobiasfaust/" GIT_REPO "@" GIT_BRANCH "/web/" ESPGPIO "'></script>\n");
  TT("<script language='javascript' type='text/javascript' src='/parameter.js'></script>\n");
  TT("<title>Bewässerungssteuerung</title></head>\n");
  TT("<body>\n");
  TT("<table>\n");
  TT("  <tr>\n");
  TT("   <td colspan='4'>\n");
  TT("     <h2>Konfiguration</h2>\n");
  TT("   </td>\n");

  TT("   <td colspan='4' style='color:#CCCCCC;'>\n");
  TT("      <i>(%s)</i>\n", Config->GetMqttRoot().c_str());
  TT("   </td>\n");

  TT("   <td colspan='5'>\n");
  TT("     <b>Release: </b><span style='color:orange;'>%s</span><br>of %s %s\n", Config->GetReleaseName().c_str(), __DATE__, __TIME__);
  TT("   </td>\n");
  TT(" </tr>\n");

  TT(" <tr>\n");
  TT("   <td class='navi' style='width: 50px'></td>\n");
  TT("   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==ROOT)?"navi_active":"");
  TT("   <td class='navi' style='width: 50px'></td>\n");
  TT("   <td class='navi %s' style='width: 100px'><a href='/BaseConfig'>Basis Config</a></td>\n", (pageactive==BASECONFIG)?"navi_active":"");
  TT("   <td class='navi' style='width: 50px'></td>\n");
  TT("   <td class='navi %s' style='width: 100px'><a href='/SensorConfig'>Sensor Config</a></td>\n", (pageactive==SENSOR)?"navi_active":"");
  TT("   <td class='navi' style='width: 50px'></td>\n");
  TT("   <td class='navi %s' style='width: 100px'><a href='/VentilConfig'>Ventil Config</a></td>\n", (pageactive==VENTILE)?"navi_active":"");
  
  if (Config->Enabled1Wire()) {
      TT("   <td class='navi' style='width: 50px'></td>\n");
      TT("   <td class='navi %s' style='width: 100px'><a href='/1WireConfig'>OneWire</a></td>\n", (pageactive==ONEWIRE)?"navi_active":"");
  }
  
  TT("   <td class='navi' style='width: 50px'></td>\n");
  TT("   <td class='navi %s' style='width: 100px'><a href='/Relations'>Relations</a></td>\n", (pageactive==RELATIONS)?"navi_active":"");
  TT("   <td class='navi' style='width: 50px'></td>\n");
  TT("   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/ESP8266_PumpControl/wiki' target='_blank'>Wiki</a></td>\n");
  TT("   <td class='navi' style='width: 50px'></td>\n");
  TT(" </tr>\n");
  TT(" <tr>\n");
  TT("   <td colspan='13'>\n");
  TT("   <p />\n");
  
  return len;
}

size_t MyWebServer::getPageFooter2(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen) {
  TT("   </td>\n");
  TT(" </tr>\n");
  TT("</table>\n");
  TT("<div id='ErrorText' class='errortext'></div>\n");
  TT("</body>\n");
  TT("</html>\n");
  return len;
}

void MyWebServer::getPageHeader(AsyncResponseStream *response, page_t pageactive) {
  response->println("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>");
  response->println("<meta charset='utf-8'>");
  //response->println("<link rel='stylesheet' type='text/css' href='/style.css'>");
  //response->println("<script language='javascript' type='text/javascript' src='/parameter.js'></script>");
  response->println("<script language='javascript' type='text/javascript' src='/javascript.js'></script>");
  //response->println("<script language='javascript' type='text/javascript' src='/jsajax.js'></script>");
  response->println("<title>Bewässerungssteuerung</title></head>");
  response->println("<body>");
  response->println("<table>");
  response->println("  <tr>");
  response->println("   <td colspan='4'>");
  response->println("     <h2>Konfiguration</h2>");
  response->println("   </td>");

  response->println("   <td colspan='4' style='color:#CCCCCC;'>");
  response->printf("      <i>(%s)</i>\n", Config->GetMqttRoot().c_str());
  response->println("   </td>");

  response->println("   <td colspan='5'>");
  response->printf ("     <b>Release: </b><span style='color:orange;'>%s</span><br>of %s %s", Config->GetReleaseName().c_str(), __DATE__, __TIME__);
  response->println("   </td>");
  response->println(" </tr>");

  response->println(" <tr>");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf ("   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==ROOT)?"navi_active":"");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf ("   <td class='navi %s' style='width: 100px'><a href='/BaseConfig'>Basis Config</a></td>\n", (pageactive==BASECONFIG)?"navi_active":"");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf ("   <td class='navi %s' style='width: 100px'><a href='/SensorConfig'>Sensor Config</a></td>\n", (pageactive==SENSOR)?"navi_active":"");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf ("   <td class='navi %s' style='width: 100px'><a href='/VentilConfig'>Ventil Config</a></td>\n", (pageactive==VENTILE)?"navi_active":"");
  
  if (Config->Enabled1Wire()) {
      response->println("   <td class='navi' style='width: 50px'></td>");
      response->printf ("   <td class='navi %s' style='width: 100px'><a href='/1WireConfig'>OneWire</a></td>\n", (pageactive==ONEWIRE)?"navi_active":"");
  }
  
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->printf ("   <td class='navi %s' style='width: 100px'><a href='/Relations'>Relations</a></td>\n", (pageactive==RELATIONS)?"navi_active":"");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->println("   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/ESP8266_PumpControl/wiki' target='_blank'>Wiki</a></td>");
  response->println("   <td class='navi' style='width: 50px'></td>");
  response->println(" </tr>");
  response->println(" <tr>");
  response->println("   <td colspan='13'>");
  response->println("   <p />");
}

void MyWebServer::getPageFooter(AsyncResponseStream *response) {
  response->println(" </tr>");
  response->println("</table>");
  response->println("<div id='ErrorText' class='errortext'></div>");
  response->println("</body>");
  response->println("</html>");
}

size_t MyWebServer::getPageStatus2(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen) {
  uint8_t count = 0;
  uptime::calculateUptime();
  
  TT("<table class='editorDemoTable'>\n");
  TT("<thead>\n");
  TT("  <tr>\n");
  TT("    <td style='width: 250px;'>Name</td>\n");
  TT("    <td style='width: 200px;'>Wert</td>\n");
  TT("  </tr>\n");
  TT("</thead>\n");
  TT("<tbody>\n");

  TT("<tr>\n");
  TT("  <td>IP-Adresse:</td>\n");
  TT("  <td>%s</td>\n", WiFi.localIP().toString().c_str());
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>WiFi Name:</td>\n");
  TT("  <td>%s</td>\n", WiFi.SSID().c_str());
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>i2c Bus:\n");
    //https://fdossena.com/?p=html5cool/buttons/i.frag
  TT("  <a href='#' onclick=\"RefreshI2C('showI2C')\" class='ButtonRefresh'>&#8634;</a>\n");
  TT("  </td>\n");
  TT("  <td><div id='showI2C'>\n");
  TT("%s \n", I2Cdetect->i2cGetAddresses().c_str());
  TT("  </div></td>\n");
  TT("</tr>\n");

  if (Config->Enabled1Wire()) {
    TT("<tr>\n");
    TT("  <td>gefundene 1Wire Controller (Devices):\n");
    TT("    <a href='#' onclick=\"Refresh1Wire('show1Wire')\" class='ButtonRefresh'>&#8634;</a>\n");
    TT("  </td>\n");
    TT("  <td><div id='show1Wire'>");
    TT("%d (%d)", VStruct->Get1WireCountDevices(), VStruct->Get1WireCountDevices()*8);
    TT("  </div></td>\n");
    TT("</tr>\n");
  }
  
  TT("<tr>\n");
  TT("  <td>MAC:</td>\n");
  TT("  <td>%s</td>\n", WiFi.macAddress().c_str());
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>WiFi RSSI:</td>\n");
  TT("  <td>%d</td>\n", WiFi.RSSI());
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>MQTT Status:</td>\n");
  TT("  <td>%s</td>\n", (mqtt->GetConnectStatusMqtt()?"Connected":"Not Connected"));
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>Uptime:</td>\n");
  TT("  <td>%lu Days, %lu Hours, %lu Minutes</td>\n", uptime::getDays(), uptime::getHours(), uptime::getMinutes()); //uptime_formatter::getUptime().c_str()); //UpTime->getFormatUptime());
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>Free Heap Memory:</td>\n");
  TT("  <td>%d</td>\n", ESP.getFreeHeap()); //https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/heap_debug.html
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>aktuell geöffnete Ventile</td>\n");
  TT("  <td>\n");

  count = VStruct->CountActiveThreads();
  if (count > 0) {
    TT("  Es sind %d Ventile offen\n", count);
  } else { 
    TT("  alle Ventile geschlossen\n"); 
  }
  TT("  </td>\n");
  TT("</tr>\n");

  if (LevelSensor->GetType() != NONE && LevelSensor->GetType() != EXTERN) {  
    TT("<tr>\n");
    TT("  <td>Sensor RAW Value:</td>\n");
    TT("  <td>%d</td>\n", LevelSensor->GetRaw());
    TT("</tr>\n");
  }
  if (LevelSensor->GetType() != NONE) {  
    TT("<tr>\n");
    TT("  <td>Füllstand in %:</td>\n");
    TT("  <td>%d %%</td>\n", LevelSensor->GetLvl());
    TT("</tr>\n");
  }

  TT("<tr>\n");
  TT("  <td>Firmware Update</td>\n");
  TT("  <td><form action='update'><input class='button' type='submit' value='Update' /></form></td>\n");
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>Device Reboot</td>\n");
  TT("  <td><form action='reboot'><input class='button' type='submit' value='Reboot' /></form></td>\n");
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>Werkszustand herstellen (ohne WiFi)</td>\n");
  TT("  <td><form action='reset'><input class='button' type='submit' value='Reset' /></form></td>\n");
  TT("</tr>\n");

  TT("<tr>\n");
  TT("  <td>WiFi Zugangsdaten entfernen</td>\n");
  TT("  <td><form action='wifireset'><input class='button' type='submit' value='WifiReset' /></form></td>\n");
  TT("</tr>\n");
  
  TT("</tbody>\n");
  TT("</table>\n");   
return len;
}

