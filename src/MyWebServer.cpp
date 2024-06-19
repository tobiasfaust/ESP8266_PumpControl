#include "MyWebServer.h" 

MyWebServer::MyWebServer(AsyncWebServer *server, DNSServer* dns): server(server), dns(dns), DoReboot(false) {
  
  fsfiles = new handleFiles(server);

  server->begin();

  server->onNotFound(std::bind(&MyWebServer::handleNotFound, this, std::placeholders::_1));
  server->on("/", HTTP_GET, std::bind(&MyWebServer::handleRoot, this, std::placeholders::_1));
  server->on("/reboot", HTTP_GET, std::bind(&MyWebServer::handleReboot, this, std::placeholders::_1));
  server->on("/reset", HTTP_GET, std::bind(&MyWebServer::handleReset, this, std::placeholders::_1));
  server->on("/wifireset", HTTP_GET, std::bind(&MyWebServer::handleWiFiReset, this, std::placeholders::_1));

  server->on("/parameter.js", HTTP_GET, std::bind(&MyWebServer::handleJSParam, this, std::placeholders::_1));
  server->on("/ajax", HTTP_POST, std::bind(&MyWebServer::handleAjax, this, std::placeholders::_1));
  server->on("/update",                 HTTP_GET, std::bind(&MyWebServer::handle_update_page, this, std::placeholders::_1));
  server->on("/update",                 HTTP_POST, std::bind(&MyWebServer::handle_update_response, this, std::placeholders::_1),
                                                   std::bind(&MyWebServer::handle_update_progress, this, std::placeholders::_1, 
                                                          std::placeholders::_2,
                                                          std::placeholders::_3,
                                                          std::placeholders::_4,
                                                          std::placeholders::_5,
                                                          std::placeholders::_6));

  server->on("^/(.+).(css|js|html|json)$", HTTP_GET, std::bind(&MyWebServer::handleRequestFiles, this, std::placeholders::_1));
  
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

      /*
      if (filename == "filesystem") {
        if(!Update.begin(LittleFS.totalBytes(), U_SPIFFS)) {
          Update.printError(Serial);
        }
      } else {
      */
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
          Update.printError(Serial);
      }
      //}
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
  request->redirect("/web/index.html");
}

void MyWebServer::handleRequestFiles(AsyncWebServerRequest *request) {
  if (Config->GetDebugLevel() >=3) {
    Serial.printf("Request file %s", ("/" + request->pathArg(0) + "." + request->pathArg(1)).c_str()); Serial.println();
  }  
  
  File f = LittleFS.open("/" + request->pathArg(0) + "." + request->pathArg(1), "r");
  
  if (!f) {
    if (Config->GetDebugLevel() >=0) {Serial.printf("failed to open requested file: %s.%s", request->pathArg(0).c_str(), request->pathArg(1).c_str());}
    request->send(404, "text/plain", "404: Not found"); 
    return;
  }

  f.close();

  if (request->pathArg(1) == "css") {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/css");
  } else if (request->pathArg(1) == "js") {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/javascript");
  } else if (request->pathArg(1) == "html") {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/html");
  } else if (request->pathArg(1) == "json") {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/json");
  } else {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/plain");
  }

}

void MyWebServer::handleReboot(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML_UPDATERESPONSE);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response);

  this->DoReboot = true;
}

void MyWebServer::handleReset(AsyncWebServerRequest *request) {
  if (Config->GetDebugLevel() >= 3) { Serial.println("deletion of all config files was requested ...."); }
  //LittleFS.format(); // Werkszustand -> nur die config dateien loeschen, die register dateien muessen erhalten bleiben
  File root = LittleFS.open("/", "r");
  File file = root.openNextFile();
  while(file){
    String path("/"); path.concat(file.name());
    if (path.indexOf(".json") == -1) {Serial.println("Continue"); file = root.openNextFile(); continue;}
    file.close();
    bool rm = LittleFS.remove(path);
    if (Config->GetDebugLevel() >= 3) {
      Serial.printf("deletion of configuration file '%s' %s\n", file.name(), (rm?"was successful":"has failed"));;
    }
    file = root.openNextFile();
  }
  root.close();

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

void MyWebServer::handleJSParam(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/javascript");
  response->addHeader("Server","ESP Async Web Server");

  VStruct->getWebJsParameter(response);
  request->send(response);
}

void MyWebServer::handleAjax(AsyncWebServerRequest *request) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  String ret = (char*)0;
  bool RaiseError = false;
  String action, subaction, newState; 
  String json = "{}";
  uint8_t port = 0;

  AsyncResponseStream *response = request->beginResponseStream("text/json");
  response->addHeader("Server","ESP Async Web Server");

  if(request->hasArg("json")) {
    json = request->arg("json");
  }

  JsonDocument jsonGet; 
  DeserializationError error = deserializeJson(jsonGet, json.c_str());

  JsonDocument jsonReturn;
  jsonReturn["response"].to<JsonObject>();

  if (Config->GetDebugLevel() >=4) { Serial.print("Ajax Json Empfangen: "); }
  if (!error) {
    if (Config->GetDebugLevel() >=4) { serializeJsonPretty(jsonGet, Serial); Serial.println(); }

    if (jsonGet.containsKey("action"))   {action    = jsonGet["action"].as<String>();}
    if (jsonGet.containsKey("subaction")){subaction = jsonGet["subaction"].as<String>();}
    if (jsonGet.containsKey("newState")) { newState = jsonGet["newState"].as<String>(); }
    if (jsonGet.containsKey("port"))     { port = jsonGet["port"].as<int>(); }
  
  } else { 
    snprintf(buffer, sizeof(buffer), "Ajax Json Command not parseable: %s -> %s", json.c_str(), error.c_str());
    RaiseError = true; 
  }

  if (RaiseError) {
    jsonReturn["response"]["status"] = 0;
    jsonReturn["response"]["text"] = buffer;
    serializeJson(jsonReturn, ret);
    response->print(ret);

    if (Config->GetDebugLevel() >=2) {
      Serial.println(FPSTR(buffer));
    }

    return;
    
  } else if(action && action == "GetInitData")  {
    if (subaction && subaction == "status") {
      this->GetInitDataStatus(response);
    } else if (subaction && subaction == "navi") {
      this->GetInitDataNavi(response);
    } else if (subaction && subaction == "baseconfig") {
      Config->GetInitData(response);
    } else if (subaction && subaction == "valveconfig") {
      VStruct->GetInitData(response);
    } else if (subaction && subaction == "1wireconfig") {
      VStruct->GetInitData1Wire(response);
    }else if (subaction && subaction == "sensorconfig") {
      LevelSensor->GetInitData(response);
    } else if (subaction && subaction == "relations") {
      ValveRel->GetInitData(response);
    }
  
  } else if(action && action == "ReloadConfig")  {
    if (subaction && subaction == "baseconfig") {
      Config->LoadJsonConfig();
    } else if (subaction && subaction == "valveconfig") {
      VStruct->LoadJsonConfig();
    } else if (subaction && subaction == "sensorconfig") {
      LevelSensor->LoadJsonConfig();
    } else if (subaction && subaction == "relations") {
      ValveRel->LoadJsonConfig();
    }
  
    jsonReturn["response"]["status"] = 1;
    jsonReturn["response"]["text"] = "new config reloaded sucessfully";
    serializeJson(jsonReturn, ret); 
    response->print(ret);
  
  } else if(action && action == "handlefiles") {
    fsfiles->HandleAjaxRequest(jsonGet, response);

  } else if (action && action == "SetValve") {
      if (newState && port && port > 0 && !VStruct->GetEnabled(port)) { 
        jsonReturn["response"]["status"] = 0; 
        jsonReturn["response"]["text"] = "Requested Port not enabled. Please enable first!";
        serializeJson(jsonReturn, ret);
        response->print(ret);
      }
      else if (newState && port && port > 0 )  { 
        if (newState == "On") {
          VStruct->SetOn(port); 
        }
        if (newState == "Off") { 
          VStruct->SetOff(port); 
        }

        jsonReturn["response"]["status"] = 1;
        jsonReturn["response"]["text"] =(VStruct->GetState(port)?"Valve is now: ON":"Valve is now: OFF");
        jsonReturn["data"][subaction] = (VStruct->GetState(port)?"Set Off":"Set On"); // subaction = button.id
        serializeJson(jsonReturn, ret);
        response->print(ret);
      }

  
  } else if (action && newState && action == "EnableValve") {
      if (port && port > 0 && newState) {
        if (strcmp(newState.c_str(),"true")==0) VStruct->SetEnable(port, true);
        if (strcmp(newState.c_str(),"false")==0) VStruct->SetEnable(port, false);
        jsonReturn["response"]["status"] = 1;
        jsonReturn["response"]["text"] = (VStruct->GetEnabled(port)?"valve now enabled":"valve now disabled");
        serializeJson(jsonReturn, ret);
        response->print(ret);
      }
  
#ifdef USE_I2C
  } else if (action && action == "RefreshI2C") {
      I2Cdetect->i2cScan();  
      
      jsonReturn["data"].to<JsonObject>();
      jsonReturn["data"]["showI2C"] = I2Cdetect->i2cGetAddresses();
      jsonReturn["response"]["status"] = 1;
      jsonReturn["response"]["text"] = "successful";
      serializeJson(jsonReturn, ret);
      response->print(ret);
#endif

#ifdef USE_ONEWIRE
  } else if (action && action == "Refresh1Wire") {
      uint8_t ow = VStruct->Refresh1WireDevices();  
      snprintf(buffer, sizeof(buffer), "%d (%d)", ow, ow * 8);
      
      jsonReturn["data"].to<JsonObject>();
      jsonReturn["data"]["show1Wire"] = buffer;
      jsonReturn["response"]["status"] = 1;
      jsonReturn["response"]["text"] = "successful";
      serializeJson(jsonReturn, ret);
      response->print(ret);
  #endif

  } else {
    snprintf(buffer, sizeof(buffer), "Ajax Command unknown: %s - %s", action.c_str(), subaction.c_str());
    jsonReturn["response"]["status"] = 0;
    jsonReturn["response"]["text"] = buffer;
    serializeJson(jsonReturn, ret);
    response->print(ret);

    if (Config->GetDebugLevel() >=1) {
      Serial.println(buffer);
    }
  }
  
  if (Config->GetDebugLevel() >=4) { Serial.print("Ajax Json Antwort: "); Serial.println(ret); }
  
  request->send(response);
}

void MyWebServer::GetInitDataNavi(AsyncResponseStream *response){
  String ret;
  JsonDocument json;
  json["data"].to<JsonObject>();
  json["data"]["hostname"] = Config->GetMqttRoot();
  json["data"]["releasename"] = Config->GetReleaseName();
  json["data"]["releasedate"] = __DATE__;
  json["data"]["releasetime"] = __TIME__;

  #ifdef USE_ONEWIRE
    if (VStruct->Get1WireCountDevices()==0) { 
      json["data"]["td_1wire_0"]["className"] = "hide";
      json["data"]["td_1wire_1"]["className"] = "hide";
    }
  #else
    json["data"]["td_1wire_0"]["className"] = "hide";
    json["data"]["td_1wire_1"]["className"] = "hide";
  #endif

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
  serializeJson(json, ret);
  response->print(ret);
}

void MyWebServer::GetInitDataStatus(AsyncResponseStream *response) {
  String ret;
  JsonDocument json;
  
  json["data"].to<JsonObject>();
  json["data"]["ipaddress"] = mqtt->GetIPAddress().toString();
  json["data"]["wifiname"] = (Config->GetUseETH()?"LAN":WiFi.SSID());
  json["data"]["macaddress"] = WiFi.macAddress();
  json["data"]["mqtt_status"] = (mqtt->GetConnectStatusMqtt()?"Connected":"Not Connected");
  json["data"]["uptime"] = uptime_formatter::getUptime();
  json["data"]["freeheapmem"] = ESP.getFreeHeap();
  json["data"]["ValvesCount"] = VStruct->CountActiveThreads();
  
  #ifdef USE_I2C
    json["data"]["showI2C"] = I2Cdetect->i2cGetAddresses();
  #else
    json["data"]["tr_i2c"]["className"] = "hide";
  #endif

  #ifdef USE_ONEWIRE
    if (Config->Enabled1Wire()) {
      json["data"]["show1Wire"] = VStruct->Get1WireCountDevices()*8;
    } else { 
      json["data"]["tr_1wire"]["className"] = "hide";
    }
  #else
    json["data"]["tr_1wire"]["className"] = "hide";
  #endif

  if (LevelSensor->GetType() != NONE && LevelSensor->GetType() != EXTERN) { 
    json["data"]["SensorRawValue"] = LevelSensor->GetRaw();
  } else {
    json["data"]["tr_sensRaw"]["className"] = "hide";
  }

  if (LevelSensor->GetType() != NONE) {
    json["data"]["SensorLevel"] = LevelSensor->GetLvl();
  } else {
    json["data"]["tr_sensLvl"]["className"] = "hide";
  }

  #ifdef ESP32
    json["data"]["rssi"] = (Config->GetUseETH()?ETH.linkSpeed():WiFi.RSSI()), (Config->GetUseETH()?"Mbps":"");
  #else
    json["data"]["rssi"] = WiFi.RSSI();
  #endif

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";

  serializeJson(json, ret);
  response->print(ret);
}

