#include "updater.h"

updater::updater(): DoUpdate(true), automode(false), updateError(false), interval(86400) {
  this->releases = new std::vector<release_t>;
  client = new WiFiClient;
  
  this->lastupdate = millis();
  this->LoadJsonConfig();
}

void updater::setIndexJson(String url) {
  this->json_url = url;
}

void updater::setStage(stage_t s) {
  this->stage = s;
}

void updater::setAutoMode(bool a) {
  this->automode = a;  
}

void updater::setInterval(int seconds) {
  this->interval = seconds;  
}

stage_t updater::String2Stage(String s) {
  if (s=="PROD") return (stage_t)PROD;
  if (s=="PRE") return (stage_t)PRE;
  if (s=="DEV") return (stage_t)DEV;
  return (stage_t)UNDEF;
}

String updater::Stage2String(stage_t s) {
  if (s==(stage_t)PROD) return "PROD";
  if (s==(stage_t)PRE) return "PRE";
  if (s==(stage_t)DEV) return "DEV";
  return "UNDEF";
}

String updater::GetUpdateErrorString() {
  return ESPhttpUpdate.getLastErrorString() + "(" + ESPhttpUpdate.getLastError() + ")";
}


release_t* updater::GetCurrentRelease() {
  return &this->currentRelease;
}

String updater::GetReleaseName() {
  return (this->currentRelease.version + " (" + this->Stage2String(this->currentRelease.stage)+ ")");
}

std::vector<release_t>* updater::GetReleases() {
  return this->releases;
}

void updater::RefreshReleases() {
  this->downloadJson();  
}

release_t updater::getLatestRelease() {
  release_t newRelease = this->currentRelease;
  for (uint8_t i=0; i < this->releases->size(); i++) {
    // je nach stage
    if ( (this->stage == (stage_t)PROD and this->releases->at(i).stage == (stage_t)PROD) ||
         (this->stage == (stage_t)PRE and (this->releases->at(i).stage == (stage_t)PROD || this->releases->at(i).stage == (stage_t)PRE)) ||
         (this->stage == (stage_t)DEV)
       ) {
      if (max(this->currentRelease.number, this->releases->at(i).number) > this->currentRelease.number)  { newRelease = this->releases->at(i); }
    }
  }
  return newRelease;
}

void updater::Update() {
  this->downloadJson();
  // check auf neues Release wenn AutoModus und kein Fehlercode gesetzt
  if (this->automode) {
    release_t r = this->getLatestRelease();
    if (r.number > this->currentRelease.number) {
      InstallRelease(r.name);
    }
  } else { Serial.println("No AutoMode"); }
}

/*
String* updater::getURLHostName(String* url) {
  uint8_t start = url->indexOf("//")+2;
  uint8_t end   = url->indexOf("/", start);
  String hostname = url->substring(start, end);
  //Serial.print("Hostname: "); Serial.println(this->json_host.c_str()); 
  //Serial.print("URL: "); Serial.println(this->json_url.c_str()); 
  return &hostname;
}

String* updater::getURLPath(String* url) {
  uint8_t start = url->indexOf("//")+2;
  uint8_t end   = url->indexOf("/", start);
  String PathName = url->substring(end);
  return &PathName;
}
*/

void updater::downloadJson() {
  HTTPClient http;

  if (http.begin(*client, this->json_url)) { 
    int httpCode = http.GET();
    if (httpCode > 0) {
      //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        this->parseJson(&payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.printf("[HTTP] Unable to connect\n");
  }
}

void updater::parseJson(String* json) {
  this->releases->clear();
  
  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray(*json);
  if (root.success()) {
    //root.printTo(Serial);
    for (JsonArray::iterator i=root.begin(); i!=root.end(); ++i) {
      JsonObject& o = *i;
      release_t r;
      if (o.containsKey("name"))           { r.name    = o["name"].as<String>();}
      if (o.containsKey("version"))        { r.version = o["version"].as<String>();}
      if (o.containsKey("number"))         { r.number  = o["number"].as<int>();}
      if (o.containsKey("stage"))          { r.stage   = this->String2Stage(o["stage"].as<String>());}
      if (o.containsKey("download-url"))   { r.downloadURL = o["download-url"].as<String>();}
      this->releases->push_back(r);   
    }
  } else {
    Serial.println("Cannot parse json");
  } 
}

void updater::StoreJsonConfig(release_t* r) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["name"]          = r->name.c_str();
  json["version"]       = r->version.c_str();
  json["number"]        = r->number;
  json["stage"]         = this->Stage2String(r->stage);
  json["download-url"]  = r->downloadURL.c_str();
    
  File configFile = SPIFFS.open("/ESPUpdate.json", "w");
  if (!configFile) {
    Serial.println("failed to open ESPUpdate.json file for writing");
  }

  //json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

void updater::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  if (SPIFFS.exists("/ESPUpdate.json")) {
    //file exists, reading and loading
    Serial.println("reading ESPUpdate config file");
    File configFile = SPIFFS.open("/ESPUpdate.json", "r");
    if (configFile) {
      Serial.println("opened ESPUpdate config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
      if (json.success()) {
        Serial.println("\nparsed json");
        release_t r;
        if (json.containsKey("name"))          { r.name       = json["name"].as<String>();}
        if (json.containsKey("version"))       { r.version    = json["version"].as<String>();}
        if (json.containsKey("number"))        { r.number     = json["number"].as<int>();}
        if (json.containsKey("stage"))         { r.stage      = this->String2Stage(json["stage"].as<String>());}
        if (json.containsKey("download-url"))  { r.downloadURL= json["download-url"].as<String>();}
        this->currentRelease = r;
      } else {
        Serial.println("failed to load ESPUpdate config, load default config");
        loadDefaultConfig = true;
      }
    }
  } else {
    Serial.println("ESPUpdate.json config File not exists, load default config");
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    release_t r;
    r.name = "manual";
    r.version = Release;
    r.number = 0;
    r.stage = (stage_t)PROD;
    r.downloadURL = "";
    this->currentRelease = r;
    loadDefaultConfig = false; //set back
  }
}

void updater::InstallRelease(String ReleaseName) {
  release_t oldRelease = this->currentRelease;
  Serial.printf("Install Release: %s\n", ReleaseName.c_str());
  
  for (uint8_t i=0; i < this->releases->size(); i++) {
    if (this->releases->at(i).name == ReleaseName) {
      this->currentRelease = this->releases->at(i);
      break; 
    }
  }
  this->StoreJsonConfig(&this->currentRelease);
  
  Serial.printf("Install Binary: %s\n", this->currentRelease.downloadURL.c_str());
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  
  t_httpUpdate_return ret = ESPhttpUpdate.update(*client, this->currentRelease.downloadURL);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      this->currentRelease = oldRelease;
      this->StoreJsonConfig(&this->currentRelease);
      //this->automode = false;
      this->updateError = true;
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      this->currentRelease = oldRelease;
      this->StoreJsonConfig(&this->currentRelease);      
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  } 
}
    
void updater::loop() {
  if (WiFi.status() == WL_CONNECTED) { 
    if ((this->DoUpdate && millis()>5000) || ((millis() - this->lastupdate) > this->interval * 1000) || millis() < this->lastupdate)  {
      // nach dem Boot + 5sek oder täglich oder nach millis() restart
      this->DoUpdate = false;
      this->lastupdate = millis();
      this->Update();
    }
  } 
}

