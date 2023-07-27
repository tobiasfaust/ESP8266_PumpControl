#include "valveStructure.h"

valveStructure::valveStructure(uint8_t sda, uint8_t scl) :
  pin_sda(sda), pin_scl(scl) {
  this->ValveHW = new valveHardware(sda, scl);
  if (Config->Enabled1Wire()) { this->ValveHW->add1WireDevice(Config->GetPin1Wire());}
  Valves  = new std::vector<valve>{};
  LoadJsonConfig();
}

/*void valveStructure::addValve(uint8_t Port, String SubTopic) {
  valve myValve;
  myValve.init(ValveHW, Port, SubTopic);
  this->Valves->push_back(myValve);
}*/

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

  if (Config->Enabled1Wire() && /*this->ValveHW->Get1WireActive() &&*/ Config->GetPin1Wire() != this->ValveHW->GetPin1wire()) {
    Serial.println("Der 1Wire hat sich geändert, initiiere den 1Wire Bus neu.....");
    ValveHW->add1WireDevice(Config->GetPin1Wire());
  }
}

void valveStructure::ReceiveMQTT(String topic, int value) {
  char buffer[50] = {0};
  memset(buffer, 0, sizeof(buffer));
  String SubTopic(topic); // nur das konfigurierte Subtopic, zb. "valve1"
  SubTopic = SubTopic.substring(SubTopic.lastIndexOf("/", SubTopic.lastIndexOf("/")-1)+1, SubTopic.lastIndexOf("/"));
  if (topic == "/test/on-for-timer") { Valves->at(0).OnForTimer(value); }
  if (topic.startsWith(mqtt->GetRoot()) && topic.endsWith("on-for-timer")) { this->OnForTimer(SubTopic, value); }
  if (topic.startsWith(mqtt->GetRoot()) && topic.endsWith("setstate") && value==1) { this->SetOn(SubTopic); }
  if (topic.startsWith(mqtt->GetRoot()) && topic.endsWith("setstate") && value==0) { this->SetOff(SubTopic); }
  if (topic.startsWith(mqtt->GetRoot()) && topic.endsWith("state") && value==0) { this->SetOff(SubTopic); }
  if (topic.endsWith("state")) { this->handleDeps(topic, value); } 
}

void valveStructure::handleDeps(String topic, int value) {
  // topic: PumpControlDev/Valve1/state
  // Check auf Ventile, die auf Relationen ansprechen sollen
  String BaseTopic(topic); // das komplette topic ohne Kommando, zb. "PumpControlDev/Valve1"
  BaseTopic = BaseTopic.substring(0, BaseTopic.lastIndexOf("/"));
  
  std::vector<uint8_t> Ports;
  ValveRel->GetPortDependencies(&Ports, BaseTopic);
  for (uint8_t i=0; i<Ports.size(); i++) {
    if (value == 1 && topic.endsWith("state")) {
      if (!ValveRel->CheckEnabledByBypass(Ports.at(i), BaseTopic) || !Config->Enabled3Wege() || 
         (ValveRel->CheckEnabledByBypass(Ports.at(i), BaseTopic) && Config->Enabled3Wege() && this->GetState(Config->Get3WegePort()))) { 
        this->SetOn(Ports.at(i));
        ValveRel->AddSubscriber(Ports.at(i), BaseTopic);
      }
    } else if (value == 0 && topic.endsWith("state")) {
      ValveRel->DelSubscriber(BaseTopic);
      if(ValveRel->CountActiveSubscribers(Ports.at(i)) == 0) { this->SetOff(Ports.at(i)); }
    }
  }
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

uint8_t valveStructure::Get1WireCountDevices() {
  return this->ValveHW->Get1WireCountDevices();
}

uint8_t valveStructure::Refresh1WireDevices() {
  return this->ValveHW->Refresh1WireDevices();
}

void valveStructure::StoreJsonConfig(String* json) {
  StaticJsonDocument<512> doc;
  deserializeJson(doc, *json);
  JsonObject root = doc.as<JsonObject>();

  if (!root.isNull()) {
    File configFile = LittleFS.open("/VentilConfig.json", "w");
    if (!configFile) {
      if (Config->GetDebugLevel() >=0) {Serial.println("failed to open VentilConfig.json file for writing");}
    } else {  
      if (Config->GetDebugLevel() >= 3) {
        serializeJsonPretty(doc, Serial); 
        Serial.println();
      }

      if (serializeJson(doc, configFile) == 0) {
        if (Config->GetDebugLevel() >=0) {Serial.println(F("Failed to write to file"));}
      }
      configFile.close();
  
      LoadJsonConfig();
    }
  }
}

void valveStructure::LoadJsonConfig() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  bool loadDefaultConfig = false;
  
  if (LittleFS.exists("/VentilConfig.json")) {
    //file exists, reading and loading
    Serial.println("reading config file");
    File configFile = LittleFS.open("/VentilConfig.json", "r");
    if (configFile) {
      Serial.println("opened config file");
      //size_t size = configFile.size();

      StaticJsonDocument<512> json; // TODO Use computed size??
      DeserializationError error = deserializeJson(json, configFile);
      
      if (!error) {
        if (Config->GetDebugLevel() >=3 ) {
          serializeJsonPretty(json, Serial); 
          Serial.println();
        }
        uint8_t count = 0;
        if (json.containsKey("count")) { count = json["count"].as<int>(); }
        if(count == 0) {
          if (Config->GetDebugLevel() >=2) { Serial.println("something went wrong with Ventilconfig, load default config"); }
          loadDefaultConfig = true;
        }
        
        for (uint8_t i=0; i<count; i++) {
          valve myValve;
          
          sprintf(buffer, "pcfport_%d_0", i);
          if (json.containsKey(buffer) && json[buffer].as<int>() > 0) { myValve.AddPort1(this->ValveHW, json[buffer].as<int>()); }

          sprintf(buffer, "type_%d", i);
          if (json.containsKey(buffer)) {myValve.SetValveType(json[buffer].as<String>()); }
          
          sprintf(buffer, "active_%d", i);
          if (json[buffer] && json[buffer] == 1) {myValve.SetActive(true);} else {myValve.SetActive(false);}
            
          sprintf(buffer, "mqtttopic_%d", i);
          if (json.containsKey(buffer)) {myValve.subtopic = json[buffer].as<String>();}
 
          sprintf(buffer, "pcfport_%d_1", i);
          if (json.containsKey(buffer) && json[buffer].as<int>() > 0) { myValve.AddPort2(ValveHW, json[buffer].as<int>());}
            
          sprintf(buffer, "imp_%d_0", i); //impulsbreite für Port 1
          if (json.containsKey(buffer)) { myValve.port1ms = max(10, min(json[buffer].as<int>(), 999));}
          
          sprintf(buffer, "imp_%d_1", i); //impulsbreite für Port 2
          if (json.containsKey(buffer)) { myValve.port2ms = max(10, min(json[buffer].as<int>(), 999));}

          sprintf(buffer, "reverse_%d", i);
          if (json[buffer] && json[buffer] == 1) {myValve.SetReverse(true);} else {myValve.SetReverse(false);}

          sprintf(buffer, "autooff_%d", i);
          if (json.containsKey(buffer) && json[buffer].as<int>() > 0) { myValve.SetAutoOff(json[buffer].as<int>()); }
          
          Valves->push_back(myValve);
        }
        
      } else {
        loadDefaultConfig = true;
      }
    } else {
      loadDefaultConfig = true;
    }
  } else {
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    if (Config->GetDebugLevel() >=3) { Serial.println("lade Ventile DefaultConfig"); }
    valve myValve;
    myValve.init(ValveHW, 202, "Valve1");
    Valves->push_back(myValve);
    myValve.init(ValveHW, 203, "Valve2");
    Valves->push_back(myValve);
  }
}

void valveStructure::GetWebContent1Wire(AsyncResponseStream *response) {
   if (Config->Enabled1Wire()) { ValveHW->GetWebContent1Wire(response); }
}

void valveStructure::GetWebContent(AsyncResponseStream *response) {
  response->println("<p><input type='button' value='&#10010; add new Port' onclick='addrow(\"maintable\")'></p>\n");
  response->println("<form id='DataForm'>\n");
  response->println("<table id='maintable' class='editorDemoTable'>\n");
  response->println("<thead>\n");
  response->println("<tr>\n");
  response->println("<td style='width: 25px;'>Nr</td>\n");
  response->println("<td style='width: 25px;'>Active</td>\n");
  response->println("<td style='width: 250px;'>MQTT SubTopic</td>\n");
  response->println("<td style='width: 210 px;'>Port</td>\n");
  response->println("<td style='width: 80px;'>Type</td>\n");
  response->println("<td style='width: 80px;'>Reverse</td>\n");
  response->println("<td style='width: 80px;'>AutoOff</td>\n");
  response->println("<td style='width: 25px;'>Delete</td>\n");
  response->println("<td style='width: 25px;'>Action</td>\n");
  response->println("</tr>\n");
  response->println("</thead>\n");

  for(uint8_t i=0; i<Valves->size(); i++) {
    response->println("<tr>\n");
    response->printf("  <td>%d</td>\n", i+1);
    response->println("  <td>\n");
    response->println("    <div class='onoffswitch'>\n");
    response->printf("      <input type='checkbox' name='active_%d' class='onoffswitch-checkbox' onclick='ChangeEnabled(this.id)' id='myonoffswitch_%d' %s>\n", i, i, (Valves->at(i).GetEnabled()?"checked":""));
    response->printf("      <label class='onoffswitch-label' for='myonoffswitch_%d'>\n", i);
    response->println("        <span class='onoffswitch-inner'></span>\n");
    response->println("        <span class='onoffswitch-switch'></span>\n");
    response->println("      </label>\n");
    response->println("    </div>\n");
    response->println("  </td>\n");
    
    response->printf("  <td><input size='30' name='mqtttopic_%d' type='text' value='%s'/></td>\n", i, Valves->at(i).subtopic.c_str());
    response->printf("  <td id='tdport_%d'>\n", i);
    
    if (Valves->at(i).GetValveType() == "b") {
      response->printf("    <div id='PortA_%d'>\n", i);
      response->println("    <div class='inline'>\n");
      response->printf("      <input id='AllePorts_PortA_%d' name='pcfport_%d_0' type='number' min='0' max='220' value='%d'/></div>\n",i, i, Valves->at(i).GetPort1());
      response->println("      <label>for</label>\n");
      response->printf("      <input id='imp_%d_0' name='imp_%d_0'  value='%d' type='number' min='10' max='999'/>\n",i, i, Valves->at(i).port1ms);
      response->println("      <label>ms</label>\n");
      response->println("    </div>\n");
      
      response->printf("    <div id='PortB_%d'>\n", i);
      response->println("      <div class='inline'>\n");
      response->printf("      <input id='AllePorts_PortB_%d' name='pcfport_%d_1' type='number' min='0' max='220' value='%d'/></div>\n",i, i, Valves->at(i).GetPort2());
      response->println("      <label>for</label>\n");
      response->printf("      <input id='imp_%d_1' name='imp_%d_1'  value='%d' type='number' min='10' max='999'/>\n",i, i, Valves->at(i).port2ms);
      response->println("      <label>ms</label>\n");
      response->println("    </div>\n");
    } else if (Valves->at(i).GetValveType() == "n") {
      response->printf("    <div id='Port_%d'>\n", i);
      response->printf("      <input id='AllePorts_%d' name='pcfport_%d_0' type='number' min='0' max='220' value='%d'/>\n",i, i, Valves->at(i).GetPort1());
      response->println("    </div>\n");
    } 
    response->println("  </td>\n");
    response->println("  <td>\n");
    response->printf("    <div class='inline'><input type='radio' id='type_%d_0' name='type_%d' value='n' %s onclick='chg_type(this.id)' /><label for='type_%d_0'>normal</label></div>\n",i, i, (Valves->at(i).GetValveType()=="n"?"checked":""),i);
    response->printf("    <div class='inline'><input type='radio' id='type_%d_1' name='type_%d' value='b' %s onclick='chg_type(this.id)' /><label for='type_%d_1'>bistabil</label></div>\n",i, i, (Valves->at(i).GetValveType()=="b"?"checked":""),i);
    response->println("  </td>\n");

    response->println("  <td>\n");
    response->println("    <div class='onoffswitch'>\n");
    response->printf("      <input type='checkbox' name='reverse_%d' class='onoffswitch-checkbox' id='myreverseswitch_%d' %s>\n", i, i, (Valves->at(i).GetReverse()?"checked":""));
    response->printf("      <label class='onoffswitch-label' for='myreverseswitch_%d'>\n", i);
    response->println("        <span class='onoffswitch-inner'></span>\n");
    response->println("        <span class='onoffswitch-switch'></span>\n");
    response->println("      </label>\n");
    response->println("    </div>\n");
    response->println("  </td>\n");

    response->println("  <td>\n");
    response->printf("      <input id='autooff_%d' name='autooff_%d' type='number' min='0' max='65000' value='%d'/>\n",i, i, Valves->at(i).GetAutoOff());
    response->println("  </td>\n");
    
    response->println("  <td><input type='button' value='&#10008;' onclick='delrow(this)'></td>\n");

    response->println("  <td>\n");
    response->printf("    <input type='button' id='action_%d' value='Set %s' onClick='ChangeValve(this.id)'/>\n", i, (!Valves->at(i).GetActive()?"On":"Off"));
    response->println("  </td>\n");

    response->println("</tr>\n");
  }
  response->println("</tbody>\n");
  response->println("</table>\n");
  response->println("</form>\n\n<br />\n");
  response->println("<form id='jsonform' action='StoreVentilConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  response->println("  <input type='text' id='json' name='json' />\n");
  response->println("  <input type='submit' value='Speichern' />\n");
  response->println("</form>\n\n");
}

void valveStructure::getWebJsParameter(AsyncResponseStream *response) {
  
  // bereits belegte Ports, können nicht ausgewählt werden (zb.i2c-ports)
  // const gpio_disabled = Array(0,4);
  response->printf("const gpio_disabled = [%d,%d,%d];\n", Config->GetPinSDA() + 200, Config->GetPinSCL() + 200, (Config->Enabled1Wire()?Config->GetPin1Wire() + 200:0));

  // anhand gefundener pcf Devices die verfügbaren Ports bereit stellen
  //const pcf_found = [65,72];
  response->println("const availablePorts = [");
  uint8_t count=0;
  for (uint8_t p=1; p<=254; p++) {
    if (ValveHW->IsValidPort(p) && (I2Cdetect->i2cIsPresent(ValveHW->GetI2CAddress(p)) || ValveHW->GetI2CAddress(p) == 0x01) && (!Config->EnabledOled() || Config->GetI2cOLED()!=ValveHW->GetI2CAddress(p))) {
      // i2cDetect muss den ic2Port finden oder es ist 0x01 OneWire 
      //ohne die OLED i2c Adresse
      response->printf("%s%d", (count>0?",":"") , p);
      count++;
    }
  }
  response->println("];\n");

  //konfigurierte Ports / Namen
  //const configuredPorts = [ {port:65, name:"Ventil1"}, {port:67, name:"Ventil2"}]
  response->println("const configuredPorts = [");
  for(int i=0; i < Valves->size(); i++) {
    response->printf("{port:%d, name:'%s'}%s", Valves->at(i).GetPort1() ,Valves->at(i).subtopic.c_str(), (i<Valves->size()-1?",":""));
  }
  response->println("];\n");
}
