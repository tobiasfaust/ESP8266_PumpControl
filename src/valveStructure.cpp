#include "valveStructure.h"

valveStructure::valveStructure(uint8_t sda, uint8_t scl) :
  pin_sda(sda), pin_scl(scl) {
  this->ValveHW = new valveHardware(sda, scl);
  
  this->Valves = std::make_shared<std::vector<valve>>(); 
  
  // loading twice, 1st valve is corrupted after 1st load, has to be investigate
  // TODO
  LoadJsonConfig();
  LoadJsonConfig();
}

void valveStructure::OnForTimer(String SubTopic, int duration) {
  valve* v = this->GetValveItem(SubTopic);
  if (v && v->OnForTimer(duration)) {
    if (mqtt) {mqtt->Publish_Int("Threads", (int)this->CountActiveThreads(), false); }
  }
}

void valveStructure::SetOff(String SubTopic) {
  valve* v = this->GetValveItem(SubTopic);
  if (v) { this->SetOff(GetValveItem(SubTopic)->GetPort1()); }
}

void valveStructure::SetOn(String SubTopic) {
  valve* v = this->GetValveItem(SubTopic);
  if (v) {this->SetOn(GetValveItem(SubTopic)->GetPort1()); }
}

void valveStructure::SetOn(uint8_t Port) {
  valve* v = this->GetValveItem(Port);
  if (v && v->SetOn() && mqtt) { mqtt->Publish_Int("Threads", (int)this->CountActiveThreads(), false); }
}

void valveStructure::SetOff(uint8_t Port) {
  valve* v = this->GetValveItem(Port);
  if (v) { v->SetOff(); }
  if (mqtt) { mqtt->Publish_Int("Threads", (int)this->CountActiveThreads(), false); }
}

bool valveStructure::GetState(uint8_t Port) {
  if (GetValveItem(Port)) { return GetValveItem(Port)->GetActive(); }
  return NULL;
}

bool valveStructure::GetEnabled(uint8_t Port) {
  if (GetValveItem(Port)) { return GetValveItem(Port)->GetEnabled();}
  return NULL;
}

void valveStructure::SetEnable(uint8_t Port, bool state) {
  if (GetValveItem(Port)) { GetValveItem(Port)->SetActive(state); }
}

void valveStructure::loop() {
  for (uint8_t i=0; i<Valves->size(); i++) {
    Valves->at(i).loop();
  }
}

void valveStructure::ReceiveMQTT(String topic, int value) {
  String SubTopic(topic); // nur das konfigurierte Subtopic, zb. "valve1"
  SubTopic = SubTopic.substring(SubTopic.lastIndexOf("/", SubTopic.lastIndexOf("/")-1)+1, SubTopic.lastIndexOf("/"));
  if (topic == "/test/on-for-timer") { Valves->at(0).OnForTimer(value); }

  if (topic.startsWith(mqtt->getTopic("", false)) && topic.endsWith("on-for-timer")) { this->OnForTimer(SubTopic, value); }
  if (topic.startsWith(mqtt->getTopic("", false)) && topic.endsWith("setstate") && value==1) { this->SetOn(SubTopic); }
  if (topic.startsWith(mqtt->getTopic("", false)) && topic.endsWith("setstate") && value==0) { this->SetOff(SubTopic); }
  if (topic.startsWith(mqtt->getTopic("", false)) && topic.endsWith("state") && value==0) { this->SetOff(SubTopic); } 
}

valve* valveStructure::GetValveItem(uint8_t Port) {
  for (uint8_t i=0; i<Valves->size(); i++) {
    if (Valves->at(i).GetPort1() == Port) {return &Valves->at(i);}
  }
  return NULL;
}

valve* valveStructure::GetValveItem(String SubTopic) {
  for (uint8_t i=0; i<Valves->size(); i++) {
    if (Valves->at(i).subtopic == SubTopic) { return &Valves->at(i);}
  }
  return NULL;
}

uint8_t valveStructure::CountActiveThreads() {
  uint8_t count = 0;
  for (uint8_t i=0; i<Valves->size(); i++) {
    if (Valves->at(i).GetActive() && (Valves->at(i).GetPort1() != Config->Get3WegePort() || !Config->Enabled3Wege() )) {count++;}
  }
  return count;
}

/* load json config from littlefs */
void valveStructure::LoadJsonConfig() {
  bool loadDefaultConfig = false;

  if (!Valves->empty()) { 
    Valves->erase(Valves->begin(), Valves->end());
  }

  if (LittleFS.exists("/valveconfig.json")) {
    //file exists, reading and loading
    if (Config->GetDebugLevel() >=3) dbg.println("reading valveconfig.json file....");
    File configFile = LittleFS.open("/valveconfig.json", "r");
    if (configFile) {
      if (Config->GetDebugLevel() >=3) dbg.println("valveconfig.json is now open");

      ReadBufferingStream stream{configFile, 64};
      stream.find("\"data\":[");
      do {
        JsonDocument elem;
        DeserializationError error = deserializeJson(elem, stream); 

        if (error) {
          loadDefaultConfig = true;
          if (Config->GetDebugLevel() >=1) {
            dbg.printf("Failed to parse valveconfig.json data: %s, load default config\n", error.c_str()); 
          } 
        } else {
          // Print the result
          if (Config->GetDebugLevel() >=4) {dbg.println("parsing JSON ok"); }
          if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, dbg);} 

          valve myValve;
            
          String type = GetJsonKeyMatch(&elem, "type");
          if (elem.containsKey("port_a") && elem["port_a"].as<int>() > 0) { myValve.AddPort1(this->ValveHW, elem["port_a"].as<int>()); }
          if (elem.containsKey(type)) {myValve.SetValveType(elem[type].as<String>()); }
          if (elem["active"] && elem["active"] == 1) {myValve.SetActive(true);} else {myValve.SetActive(false);}
          if (elem.containsKey("mqtttopic")) {myValve.subtopic = elem["mqtttopic"].as<String>();}
          if (elem.containsKey("port_b") && elem["port_b"].as<int>() > 0) { myValve.AddPort2(ValveHW, elem["port_b"].as<int>());}
          if (elem.containsKey("imp_a")) { myValve.port1ms = _max(10, _min(elem["imp_a"].as<int>(), 999));}
          if (elem.containsKey("imp_b")) { myValve.port2ms = _max(10, _min(elem["imp_b"].as<int>(), 999));}
          if (elem["reverse"] && elem["reverse"] == 1) {myValve.SetReverse(true);} else {myValve.SetReverse(false);}
          if (elem.containsKey("autooff") && elem["autooff"].as<int>() > 0) { myValve.SetAutoOff(elem["autooff"].as<int>()); }
            
          Valves->push_back(myValve);
        }

      } while (stream.findUntil(",","]"));
    } else {
      loadDefaultConfig = true;
      if (Config->GetDebugLevel() >=1) {dbg.println("failed to load valveconfig.json, load default config");}
    }
  } else {
    loadDefaultConfig = true;
    if (Config->GetDebugLevel() >=3) {dbg.println("valveconfig.json File not exists, load default config");}
  }
  
  if (loadDefaultConfig) {
    if (Config->GetDebugLevel() >=3) { dbg.println("lade Ventile DefaultConfig"); }
    valve myValve;
    
    myValve.init(this->ValveHW, 203, "Valve1");
    this->Valves->push_back(myValve);
    
    myValve.init(this->ValveHW, 204, "Valve2");
    this->Valves->push_back(myValve);
  }
  if (Config->GetDebugLevel() >=3) {
    dbg.printf("%d valves are now loaded \n", Valves->size());
  }
}

/**************************************
 lookup with a pattern for a key
 returns the first matched key 
***************************************/
String valveStructure::GetJsonKeyMatch(JsonDocument* doc, String key) {
  for (JsonPair kv : doc->as<JsonObject>()) {
    if (strstr(kv.key().c_str(), key.c_str())) { 
      return (String)kv.key().c_str();
    }
  }
  return "";
}

void valveStructure::GetInitData(AsyncResponseStream* response) {
  String ret;
  JsonDocument json;
  
  json["data"].to<JsonObject>();
  JsonArray row = json["data"]["rows"].to<JsonArray>();
  
  for(uint8_t i=0; i<Valves->size(); i++) {
    row[i]["active"] = (Valves->at(i).GetEnabled()?1:0);
    row[i]["mqtttopic"] = Valves->at(i).subtopic;

    if (Valves->at(i).GetValveType() == "b") {
      row[i]["typ_n"]["className"] = "hide";
    } else if (Valves->at(i).GetValveType() == "n") {
      row[i]["typ_b"]["className"] = "hide";
    }
      
    row[i]["AllePorts_PortA"] = Valves->at(i).GetPort1();
    row[i]["imp_a"] = Valves->at(i).port1ms;
    row[i]["AllePorts_PortB"] = Valves->at(i).GetPort2();
    row[i]["imp_b"] = Valves->at(i).port2ms;
    row[i]["AllePorts"] = Valves->at(i).GetPort1(); 

    String type_name("type_"); type_name.concat(i);
    row[i]["SelType_n"]["checked"] = (Valves->at(i).GetValveType()=="n"?1:0);
    row[i]["SelType_n"]["name"] = type_name; 
    row[i]["SelType_b"]["checked"] = (Valves->at(i).GetValveType()=="b"?1:0);
    row[i]["SelType_b"]["name"] = type_name;
    row[i]["reverse"] = (Valves->at(i).GetReverse()?1:0);
    row[i]["autooff"] = Valves->at(i).GetAutoOff();
    row[i]["action"] = (Valves->at(i).GetActive()?"Set Off":"Set On");
  }

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";

  serializeJson(json, ret);
  response->print(ret);
}

void valveStructure::getWebJsParameter(AsyncResponseStream *response) {
  
  // bereits belegte Ports, können nicht ausgewählt werden (zb.i2c-ports)
  // const gpio_disabled = Array(0,4);
  response->printf("const gpio_disabled = [%d,%d];\n", Config->GetPinSDA() + 200, Config->GetPinSCL() + 200);

  // anhand gefundener I2C Devices die verfügbaren Ports bereit stellen
  //const availablePorts = [65,72];
  response->println("const availablePorts = [");
#ifdef USE_I2C
  uint8_t count=0;
  for (uint8_t p=1; p<=254; p++) {
    if (ValveHW->IsValidPort(p) && (I2Cdetect->i2cIsPresent(ValveHW->GetI2CAddress(p))) ) {
      // i2cDetect muss den ic2Port finden 
      response->printf("%s%d", (count>0?",":"") , p);
      count++;
    }
  }
#endif

  response->println("];\n");

  //konfigurierte Ports / Namen
  //const configuredPorts = [ {port:65, name:"Ventil1"}, {port:67, name:"Ventil2"}]
  response->println("const configuredPorts = [");
  for(uint8_t i=0; i < Valves->size(); i++) {
    response->printf("{port:%d, name:'%s'}%s", Valves->at(i).GetPort1() ,Valves->at(i).subtopic.c_str(), (i<Valves->size()-1?",":""));
  }
  response->println("];\n");
}