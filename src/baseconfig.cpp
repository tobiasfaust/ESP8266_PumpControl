#include "baseconfig.h"

BaseConfig::BaseConfig(): 
  mqtt_server ("test.mosquitto.org"),
  mqtt_port(1883),
  mqtt_root("PumpControl"),
  mqtt_basepath("home/"),
  mqtt_UseRandomClientID(true),
  keepalive(0),
  debuglevel(3),
  enable_3wege(false),
  ventil3wege_port(0),
  max_parallel(0),
  useETH(0)
  {
  
  #ifdef ESP8266
    this->pin_sda = 5;
    this->pin_scl = 4;
  #elif ESP32
    this->pin_sda = 21;
    this->pin_scl = 22,
  #endif
  
  LoadJsonConfig();
}

void BaseConfig::LoadJsonConfig() {
  if (LittleFS.exists("/baseconfig.json")) {
    //file exists, reading and loading
    dbg.println(F("reading baseconfig.json file"));
    File configFile = LittleFS.open("/baseconfig.json", "r");
    if (configFile) {
      if (this->GetDebugLevel() >=3) dbg.println(F("baseconfig.json is now open"));
      ReadBufferingStream stream{configFile, 64};
      stream.find("\"data\":[");
      do {

        JsonDocument elem;
        DeserializationError error = deserializeJson(elem, stream); 
        if (error) {
          if (this->GetDebugLevel() >=1) {
            dbg.printf("Failed to parse baseconfig.json data: %s, load default config\n", error.c_str()); 
          } 
        } else {
          // Print the result
          if (this->GetDebugLevel() >=5) {dbg.println(F("parsing partial JSON of baseconfig.json ok")); }
          if (this->GetDebugLevel() >=5) {serializeJsonPretty(elem, dbg);} 
          
          if (elem.containsKey("mqttroot"))         { this->mqtt_root = elem["mqttroot"].as<String>();}
          if (elem.containsKey("mqttserver"))       { this->mqtt_server = elem["mqttserver"].as<String>();}
          if (elem.containsKey("mqttport"))         { this->mqtt_port = elem["mqttport"].as<int>();}
          if (elem.containsKey("mqttuser"))         { this->mqtt_username = elem["mqttuser"].as<String>();}
          if (elem.containsKey("mqttpass"))         { this->mqtt_password = elem["mqttpass"].as<String>();}
          if (elem.containsKey("mqttbasepath"))     { this->mqtt_basepath = elem["mqttbasepath"].as<String>();}
          if (elem.containsKey("sel_UseRandomClientID")){ if (strcmp(elem["sel_UseRandomClientID"], "none")==0) { this->mqtt_UseRandomClientID=false;} else {this->mqtt_UseRandomClientID=true;}}
          if (elem.containsKey("keepalive"))        { if (elem["keepalive"].as<int>() == 0) { this->keepalive = 0;} else { this->keepalive = _max(elem["keepalive"].as<int>(), 10);}}
          if (elem.containsKey("debuglevel"))       { this->debuglevel = _max(elem["debuglevel"].as<int>(), 0);}
          if (elem.containsKey("pinsda"))           { this->pin_sda = (elem["pinsda"].as<int>()) - 200;}
          if (elem.containsKey("pinscl"))           { this->pin_scl = (elem["pinscl"].as<int>()) - 200;}
          if (elem.containsKey("sel_3wege"))        { if (strcmp(elem["sel_3wege"], "none")==0) { this->enable_3wege=false;} else {this->enable_3wege=true;}}
          if (elem.containsKey("sel_update"))       { if (strcmp(elem["sel_update"], "manu")==0) { this->enable_autoupdate=false;} else {this->enable_autoupdate=true;}}
          if (elem.containsKey("autoupdate_url"))   { this->autoupdate_url = elem["autoupdate_url"].as<String>(); }                   
          if (elem.containsKey("ventil3wege_port")) { this->ventil3wege_port = elem["ventil3wege_port"].as<int>();}
        }
      } while (stream.findUntil(",","]"));
    } else {
      dbg.println("cannot open existing baseconfig.json config File, load default BaseConfig"); // -> constructor
    }
  } else {
    dbg.println("baseconfig.json config File not exists, load default BaseConfig");
  }

  if (!this->autoupdate_url || this->autoupdate_url.length() < 10 ) {
    this->autoupdate_url = UPDATE_URL;
  }

  // Data Cleaning
  if(this->mqtt_basepath.endsWith("/")) {
    this->mqtt_basepath = this->mqtt_basepath.substring(0, this->mqtt_basepath.length()-1); 
  }
}

const String BaseConfig::GetReleaseName() {
  return Release;
}

void BaseConfig::loop() {  
}


/* https://cpp4arduino.com/2018/11/06/what-is-heap-fragmentation.html*/
size_t BaseConfig::getFragmentation() {
  return 100 - ESP_GetMaxFreeAvailableBlock() * 100 / ESP.getFreeHeap();
}

void BaseConfig::GetInitData(AsyncResponseStream *response) {
  String ret;
  JsonDocument json;
  
  json["data"].to<JsonObject>();
  json["data"]["arch"] = ARCH;
  json["data"]["mqttroot"]    = this->mqtt_root;
  json["data"]["mqttserver"]  = this->mqtt_server;
  json["data"]["mqttport"]    = this->mqtt_port;
  json["data"]["mqttuser"]    = this->mqtt_username;
  json["data"]["mqttpass"]    = this->mqtt_password;
  json["data"]["mqttbasepath"]= this->mqtt_basepath;
  json["data"]["debuglevel"]  = this->debuglevel;
  json["data"]["sel_URCID1"]  = ((this->mqtt_UseRandomClientID)?0:1);
  json["data"]["sel_URCID2"]  = ((this->mqtt_UseRandomClientID)?1:0);
  json["data"]["keepalive"] = this->keepalive;
  
  #ifdef ESP32
    json["data"]["sel_wifi"] = ((this->useETH)?0:1);
    json["data"]["sel_eth"]  = ((this->useETH)?1:0);
  #else 
    json["data"]["tr_LAN"]["className"] = "hide";
    json["data"]["SelectLAN"]["className"] = "hide";
  #endif

  #ifdef USE_I2C
    json["data"]["GpioPin_0"] = this->pin_sda + 200;
    json["data"]["GpioPin_1"] = this->pin_scl + 200;
  #else 
    json["data"]["tr_sda"]["className"] = "hide";
    json["data"]["tr_scl"]["className"] = "hide";
  #endif

  json["data"]["sel_3wege_0"] = ((this->enable_3wege)?0:1);
  json["data"]["sel_3wege_1"] = ((this->enable_3wege)?1:0);
  json["data"]["ConfiguredPort_0"] = this->ventil3wege_port;
  json["js"]["update_url"] = this->autoupdate_url;

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
  serializeJson(json, ret);
  response->print(ret);
}
