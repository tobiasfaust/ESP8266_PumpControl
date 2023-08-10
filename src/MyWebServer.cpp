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

    this->getPageHeader(buffer, processedRows, currentRow, len, maxLen, ROOT);
    this->getPageStatus(buffer, processedRows, currentRow, len, maxLen);
    this->getPageFooter(buffer, processedRows, currentRow, len, maxLen);
    
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

    this->getPageHeader(buffer, processedRows, currentRow, len, maxLen, BASECONFIG);
    Config->GetWebContent(buffer, processedRows, currentRow, len, maxLen);
    this->getPageFooter(buffer, processedRows, currentRow, len, maxLen);
    
    return len;
	});

  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
	request->send(response);
}

void MyWebServer::handleVentilConfig(AsyncWebServerRequest *request) {
  std::shared_ptr<uint16_t> processedRows = std::make_shared<uint16_t>(0);
  AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [this, processedRows](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
    size_t len = 0;
    size_t currentRow = 0;
    maxLen = maxLen >> 1; // prevent memory issues

    this->getPageHeader(buffer, processedRows, currentRow, len, maxLen, VENTILE);
    VStruct->GetWebContent(buffer, processedRows, currentRow, len, maxLen);
    this->getPageFooter(buffer, processedRows, currentRow, len, maxLen);
    
    return len;
	});
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
	request->send(response);
}

void MyWebServer::handle1WireConfig(AsyncWebServerRequest *request) {
  std::shared_ptr<uint16_t> processedRows = std::make_shared<uint16_t>(0);
  AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [this, processedRows](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
    size_t len = 0;
    size_t currentRow = 0;
    maxLen = maxLen >> 1; // prevent memory issues

    this->getPageHeader(buffer, processedRows, currentRow, len, maxLen, ONEWIRE);
    VStruct->GetWebContent1Wire(buffer, processedRows, currentRow, len, maxLen);
    this->getPageFooter(buffer, processedRows, currentRow, len, maxLen);
    
    return len;
	});
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
	request->send(response);
}

void MyWebServer::handleSensorConfig(AsyncWebServerRequest *request) {
  std::shared_ptr<uint16_t> processedRows = std::make_shared<uint16_t>(0);
  AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [this, processedRows](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
    size_t len = 0;
    size_t currentRow = 0;
    maxLen = maxLen >> 1; // prevent memory issues

    this->getPageHeader(buffer, processedRows, currentRow, len, maxLen, SENSOR);
    LevelSensor->GetWebContent(buffer, processedRows, currentRow, len, maxLen);
    this->getPageFooter(buffer, processedRows, currentRow, len, maxLen);
    
    return len;
	});
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
	request->send(response);
}

void MyWebServer::handleRelations(AsyncWebServerRequest *request) {
  std::shared_ptr<uint16_t> processedRows = std::make_shared<uint16_t>(0);
  AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [this, processedRows](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
    size_t len = 0;
    size_t currentRow = 0;
    maxLen = maxLen >> 1; // prevent memory issues

    this->getPageHeader(buffer, processedRows, currentRow, len, maxLen, RELATIONS);
    ValveRel->GetWebContent(buffer, processedRows, currentRow, len, maxLen);
    this->getPageFooter(buffer, processedRows, currentRow, len, maxLen);
    
    return len;
	});
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
	request->send(response);
}


void MyWebServer::ReceiveJSONConfiguration(AsyncWebServerRequest *request, page_t page) {
Serial.println("ReceiveJSONConfiguration");
Serial.printf("ReceiveJSONConfiguration: 1 HEAP Memory: %d, Fragmentation: %d%% \n", ESP.getFreeHeap(), Config->getFragmentation());
  String json = "{}";

  if(request->hasArg("json")) {
    json = request->arg("json");
  }

  String targetPage = (char*)0;
  targetPage.reserve(20);
  targetPage = "/";

  if (Config->GetDebugLevel() >= 3) {
    Serial.print(F("json empfangen: "));
    Serial.println(FPSTR(json.c_str()));  
  }

  if (page==BASECONFIG)       { Config->StoreJsonConfig(&json);   targetPage = "/BaseConfig"; }
  if (page==VENTILE)          { VStruct->StoreJsonConfig(&json);  targetPage = "/VentilConfig"; }
  if (page==SENSOR)           { LevelSensor->StoreJsonConfig(&json); targetPage = "/SensorConfig"; }
  if (page==RELATIONS)        { ValveRel->StoreJsonConfig(&json); targetPage = "/Relations"; }

  request->redirect(targetPage);
Serial.printf("ReceiveJSONConfiguration: 9 HEAP Memory: %d, Fragmentation: %d%% \n", ESP.getFreeHeap(), Config->getFragmentation());
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

  DynamicJsonDocument jsonGet(512);
  DeserializationError error = deserializeJson(jsonGet, json.c_str());

  DynamicJsonDocument jsonReturn(256);

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

void MyWebServer::getPageHeader(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen, page_t pageactive) {
  WEB("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n");
  WEB("<meta charset='utf-8'>\n");
  WEB("<link rel='stylesheet' type='text/css' href='https://cdn.jsdelivr.net/gh/tobiasfaust/" GIT_REPO "@" GIT_BRANCH "/web/style.css'>\n");
  WEB("<script language='javascript' type='text/javascript' src='https://cdn.jsdelivr.net/gh/tobiasfaust/" GIT_REPO "@" GIT_BRANCH "/web/javascript.js'></script>\n");
  WEB("<script language='javascript' type='text/javascript' src='https://cdn.jsdelivr.net/gh/tobiasfaust/" GIT_REPO "@" GIT_BRANCH "/web/jsajax.js'></script>\n");
  WEB("<script language='javascript' type='text/javascript' src='https://cdn.jsdelivr.net/gh/tobiasfaust/" GIT_REPO "@" GIT_BRANCH "/web/" ESPGPIO "'></script>\n");
  WEB("<script language='javascript' type='text/javascript' src='/parameter.js'></script>\n");
  WEB("<title>Bewässerungssteuerung</title></head>\n");
  WEB("<body>\n");
  WEB("<table>\n");
  WEB("  <tr>\n");
  WEB("   <td colspan='4'>\n");
  WEB("     <h2>Konfiguration</h2>\n");
  WEB("   </td>\n");

  WEB("   <td colspan='4' style='color:#CCCCCC;'>\n");
  WEB("      <i>(%s)</i>\n", Config->GetMqttRoot().c_str());
  WEB("   </td>\n");

  WEB("   <td colspan='5'>\n");
  WEB("     <b>Release: </b><span style='color:orange;'>%s</span><br>of %s %s\n", Config->GetReleaseName().c_str(), __DATE__, __TIME__);
  WEB("   </td>\n");
  WEB(" </tr>\n");

  WEB(" <tr>\n");
  WEB("   <td class='navi' style='width: 50px'></td>\n");
  WEB("   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==ROOT)?"navi_active":"");
  WEB("   <td class='navi' style='width: 50px'></td>\n");
  WEB("   <td class='navi %s' style='width: 100px'><a href='/BaseConfig'>Basis Config</a></td>\n", (pageactive==BASECONFIG)?"navi_active":"");
  WEB("   <td class='navi' style='width: 50px'></td>\n");
  WEB("   <td class='navi %s' style='width: 100px'><a href='/SensorConfig'>Sensor Config</a></td>\n", (pageactive==SENSOR)?"navi_active":"");
  WEB("   <td class='navi' style='width: 50px'></td>\n");
  WEB("   <td class='navi %s' style='width: 100px'><a href='/VentilConfig'>Ventil Config</a></td>\n", (pageactive==VENTILE)?"navi_active":"");
  
  if (Config->Enabled1Wire()) {
      WEB("   <td class='navi' style='width: 50px'></td>\n");
      WEB("   <td class='navi %s' style='width: 100px'><a href='/1WireConfig'>OneWire</a></td>\n", (pageactive==ONEWIRE)?"navi_active":"");
  }
  
  WEB("   <td class='navi' style='width: 50px'></td>\n");
  WEB("   <td class='navi %s' style='width: 100px'><a href='/Relations'>Relations</a></td>\n", (pageactive==RELATIONS)?"navi_active":"");
  WEB("   <td class='navi' style='width: 50px'></td>\n");
  WEB("   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/ESP8266_PumpControl/wiki' target='_blank'>Wiki</a></td>\n");
  WEB("   <td class='navi' style='width: 50px'></td>\n");
  WEB(" </tr>\n");
  WEB(" <tr>\n");
  WEB("   <td colspan='13'>\n");
  WEB("   <p />\n");
}

void MyWebServer::getPageFooter(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen) {
  WEB("   </td>\n");
  WEB(" </tr>\n");
  WEB("</table>\n");
  WEB("<div id='ErrorText' class='errortext'></div>\n");
  WEB("</body>\n");
  WEB("</html>\n");
}

void MyWebServer::getPageStatus(uint8_t* buffer, std::shared_ptr<uint16_t> processedRows, size_t& currentRow, size_t& len, size_t& maxLen) {
  uint8_t count = 0;
  uptime::calculateUptime();
  
  WEB("<table class='editorDemoTable'>\n");
  WEB("<thead>\n");
  WEB("  <tr>\n");
  WEB("    <td style='width: 250px;'>Name</td>\n");
  WEB("    <td style='width: 200px;'>Wert</td>\n");
  WEB("  </tr>\n");
  WEB("</thead>\n");
  WEB("<tbody>\n");

  WEB("<tr>\n");
  WEB("  <td>IP-Adresse:</td>\n");
  WEB("  <td>%s</td>\n", WiFi.localIP().toString().c_str());
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>WiFi Name:</td>\n");
  WEB("  <td>%s</td>\n", WiFi.SSID().c_str());
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>i2c Bus:\n");
    //https://fdossena.com/?p=html5cool/buttons/i.frag
  WEB("  <a href='#' onclick=\"RefreshI2C('showI2C')\" class='ButtonRefresh'>&#8634;</a>\n");
  WEB("  </td>\n");
  WEB("  <td><div id='showI2C'>\n");
  WEB("%s \n", I2Cdetect->i2cGetAddresses().c_str());
  WEB("  </div></td>\n");
  WEB("</tr>\n");

  if (Config->Enabled1Wire()) {
    WEB("<tr>\n");
    WEB("  <td>gefundene 1Wire Controller (Devices):\n");
    WEB("    <a href='#' onclick=\"Refresh1Wire('show1Wire')\" class='ButtonRefresh'>&#8634;</a>\n");
    WEB("  </td>\n");
    WEB("  <td><div id='show1Wire'>");
    WEB("%d (%d)", VStruct->Get1WireCountDevices(), VStruct->Get1WireCountDevices()*8);
    WEB("  </div></td>\n");
    WEB("</tr>\n");
  }
  
  WEB("<tr>\n");
  WEB("  <td>MAC:</td>\n");
  WEB("  <td>%s</td>\n", WiFi.macAddress().c_str());
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>WiFi RSSI:</td>\n");
  WEB("  <td>%d</td>\n", WiFi.RSSI());
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>MQTT Status:</td>\n");
  WEB("  <td>%s</td>\n", (mqtt->GetConnectStatusMqtt()?"Connected":"Not Connected"));
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>Uptime:</td>\n");
  WEB("  <td>%lu Days, %lu Hours, %lu Minutes</td>\n", uptime::getDays(), uptime::getHours(), uptime::getMinutes()); //uptime_formatter::getUptime().c_str()); //UpTime->getFormatUptime());
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>Free Heap Memory:</td>\n");
  WEB("  <td>%d Bytes (%d%% Fragmentation)</td>\n", ESP.getFreeHeap(), Config->getFragmentation()); //https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/heap_debug.html
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>aktuell geöffnete Ventile</td>\n");
  WEB("  <td>\n");

  count = VStruct->CountActiveThreads();
  if (count > 0) {
    WEB("  Es sind %d Ventile offen\n", count);
  } else { 
    WEB("  alle Ventile geschlossen\n"); 
  }
  WEB("  </td>\n");
  WEB("</tr>\n");

  if (LevelSensor->GetType() != NONE && LevelSensor->GetType() != EXTERN) {  
    WEB("<tr>\n");
    WEB("  <td>Sensor RAW Value:</td>\n");
    WEB("  <td>%d</td>\n", LevelSensor->GetRaw());
    WEB("</tr>\n");
  }
  if (LevelSensor->GetType() != NONE) {  
    WEB("<tr>\n");
    WEB("  <td>Füllstand in %:</td>\n");
    WEB("  <td>%d %%</td>\n", LevelSensor->GetLvl());
    WEB("</tr>\n");
  }

  WEB("<tr>\n");
  WEB("  <td>Firmware Update</td>\n");
  WEB("  <td><form action='update'><input class='button' type='submit' value='Update' /></form></td>\n");
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>Device Reboot</td>\n");
  WEB("  <td><form action='reboot'><input class='button' type='submit' value='Reboot' /></form></td>\n");
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>Werkszustand herstellen (ohne WiFi)</td>\n");
  WEB("  <td><form action='reset'><input class='button' type='submit' value='Reset' /></form></td>\n");
  WEB("</tr>\n");

  WEB("<tr>\n");
  WEB("  <td>WiFi Zugangsdaten entfernen</td>\n");
  WEB("  <td><form action='wifireset'><input class='button' type='submit' value='WifiReset' /></form></td>\n");
  WEB("</tr>\n");
  
  WEB("</tbody>\n");
  WEB("</table>\n");   
}

