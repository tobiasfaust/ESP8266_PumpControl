#include "valveStructure.h"

extern MQTT* mqtt;
extern valveRelation* ValveRel;
extern i2cdetect* I2Cdetect;

valveStructure::valveStructure(uint8_t sda, uint8_t scl) :
  pin_sda(sda), pin_scl(scl) {
  ValveHW = new valveHardware(sda, scl);
  Valves  = new std::vector<valve>{};
  SPIFFS.begin();
  LoadJsonConfig();
}

void valveStructure::addValve(uint8_t Port, String SubTopic) {
  valve myValve;
  myValve.init(ValveHW, Port, SubTopic);
  Valves->push_back(myValve);
}

void valveStructure::OnForTimer(String SubTopic, int duration) {
  valve* v = this->GetValveItem(SubTopic);
  if (v && v->OnForTimer(duration)) {
    if (mqtt) {mqtt->Publish_Int("Threads", (int*)CountActiveThreads()); }
  }
}

void valveStructure::SetOff(String SubTopic) {
  valve* v = this->GetValveItem(SubTopic);
  if (v) { this->SetOff(GetValveItem(SubTopic)->GetPort1()); }
}

void valveStructure::SetOn(String SubTopic) {
  valve* v = this->GetValveItem(SubTopic);
  if (v) { this->SetOn(GetValveItem(SubTopic)->GetPort1()); }
}

void valveStructure::SetOn(uint8_t Port) {
  valve* v = this->GetValveItem(Port);
  if (v && v->SetOn() && mqtt) { mqtt->Publish_Int("Threads", (int*)CountActiveThreads()); }
}

void valveStructure::SetOff(uint8_t Port) {
  valve* v = this->GetValveItem(Port);
  if (v) { v->SetOff(); }
  if (mqtt) { mqtt->Publish_Int("Threads", (int*)CountActiveThreads()); }
}

bool valveStructure::GetState(uint8_t Port) {
  if (GetValveItem(Port)) { return GetValveItem(Port)->active; }
  return NULL;
}

bool valveStructure::GetEnabled(uint8_t Port) {
  if (GetValveItem(Port)) { return GetValveItem(Port)->enabled;}
  return NULL;
}

void valveStructure::SetEnable(uint8_t Port, bool state) {
  if (GetValveItem(Port)) { GetValveItem(Port)->enabled = state; }
}

void valveStructure::loop() {
  for (uint8_t i=0; i<Valves->size(); i++) {
    Valves->at(i).loop();
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
    if (Valves->at(i).active && (Valves->at(i).GetPort1() != Config->Get3WegePort() || !Config->Enabled3Wege() )) {count++;}
  }
  return count;
}

void valveStructure::StoreJsonConfig(String* json) {
  //https://arduinojson.org/v5/api/jsonobject/begin_end/
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(*json);
    
  if (root.success()) {
    File configFile = SPIFFS.open("/VentilConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open VentilConfig.json file for writing");
    } else {
      root.printTo(Serial);
      root.printTo(configFile);
      configFile.close();
  
      LoadJsonConfig();
    }
  }
}

void valveStructure::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));

  Valves->clear(); // leere den Valve Vector bevor neu befüllt wird
  
  if (SPIFFS.exists("/VentilConfig.json")) {
    File configFile = SPIFFS.open("/VentilConfig.json", "r");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
      if (json.success()) {
        uint8_t count = 0;
        if (json.containsKey("count")) { count = json["count"].as<int>(); }
        if(count == 0) {
          Serial.println("something went wrong with Ventilconfig, load default config");
          loadDefaultConfig = true;
        }
        
        for (uint8_t i=0; i<count; i++) {
          valve myValve;
          
          sprintf(buffer, "pcfport_%d_0", i);
          if (json.containsKey(buffer) && json[buffer].as<int>() > 0) { myValve.AddPort1(ValveHW, json[buffer].as<int>()); }

          sprintf(buffer, "type_%d", i);
          if (json.containsKey(buffer)) {myValve.SetValveType(json[buffer].as<String>()); }
          
          sprintf(buffer, "active_%d", i);
          if (json[buffer] && json[buffer] == 1) {myValve.enabled = true;} else {myValve.enabled = false;}
            
          sprintf(buffer, "mqtttopic_%d", i);
          if (json.containsKey(buffer)) {myValve.subtopic = json[buffer].as<String>();}
 
          sprintf(buffer, "pcfport_%d_1", i);
          if (json.containsKey(buffer) && json[buffer].as<int>() > 0) { myValve.AddPort2(ValveHW, json[buffer].as<int>());}
            
          sprintf(buffer, "imp_%d_0", i); //impulsbreite für Port 1
          if (json.containsKey(buffer)) { myValve.port1ms = max(10, min(json[buffer].as<int>(), 999));}
          
          sprintf(buffer, "imp_%d_1", i); //impulsbreite für Port 2
          if (json.containsKey(buffer)) { myValve.port2ms = max(10, min(json[buffer].as<int>(), 999));}

          sprintf(buffer, "reverse_%d", i);
          if (json[buffer] && json[buffer] == 1) {myValve.reverse = true;} else {myValve.reverse = false;}

          sprintf(buffer, "autooff_%d", i);
          if (json.containsKey(buffer) && json[buffer].as<int>() > 0) { myValve.autooff = json[buffer].as<int>(); }
          
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
    Serial.println("lade Ventile DefaultConfig");
    valve myValve;
    myValve.init(ValveHW, 202, "Valve1");
    Valves->push_back(myValve);
    myValve.init(ValveHW, 203, "Valve2");
    Valves->push_back(myValve);
  }
}

void valveStructure::GetWebContent(ESP8266WebServer* server) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  String html = "";

  html.concat("<p><input type='button' value='&#10010; add new Port' onclick='addrow(\"maintable\")'></p>\n");
  html.concat("<form id='DataForm'>\n");
  html.concat("<table id='maintable' class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("<tr>\n");
  html.concat("<td style='width: 25px;'>Nr</td>\n");
  html.concat("<td style='width: 25px;'>Active</td>\n");
  html.concat("<td style='width: 250px;'>MQTT SubTopic</td>\n");
  html.concat("<td style='width: 210 px;'>Port</td>\n");
  html.concat("<td style='width: 80px;'>Type</td>\n");
  html.concat("<td style='width: 80px;'>Reverse</td>\n");
  html.concat("<td style='width: 80px;'>AutoOff</td>\n");
  html.concat("<td style='width: 25px;'>Delete</td>\n");
  html.concat("<td style='width: 25px;'>Action</td>\n");
  html.concat("</tr>\n");
  html.concat("</thead>\n");
  server->sendContent(html.c_str()); html = "";

  for(uint8_t i=0; i<Valves->size(); i++) {
    html.concat("<tr>\n");
    sprintf(buffer, "  <td>%d</td>\n", i+1);
    html.concat(buffer);
    html.concat("  <td>\n");
    html.concat("    <div class='onoffswitch'>\n");
    sprintf(buffer, "      <input type='checkbox' name='active_%d' class='onoffswitch-checkbox' onclick='ChangeEnabled(this.id)' id='myonoffswitch_%d' %s>\n", i, i, (Valves->at(i).enabled?"checked":""));
    html.concat(buffer);
    sprintf(buffer, "      <label class='onoffswitch-label' for='myonoffswitch_%d'>\n", i);
    html.concat(buffer);
    html.concat("        <span class='onoffswitch-inner'></span>\n");
    html.concat("        <span class='onoffswitch-switch'></span>\n");
    html.concat("      </label>\n");
    html.concat("    </div>\n");
    html.concat("  </td>\n");
    
    sprintf(buffer, "  <td><input size='30' name='mqtttopic_%d' type='text' value='%s'/></td>\n", i, Valves->at(i).subtopic.c_str());
    html.concat(buffer);
    sprintf(buffer, "  <td id='tdport_%d'>\n", i);
    html.concat(buffer);
    
    if (Valves->at(i).GetValveType() == "b") {
      sprintf(buffer, "    <div id='PortA_%d'>\n", i);
      html.concat(buffer);
      html.concat("    <div class='inline'>\n");
      sprintf(buffer, "      <input id='AllePorts_PortA_%d' name='pcfport_%d_0' type='number' min='0' max='220' value='%d'/></div>\n",i, i, Valves->at(i).GetPort1());
      html.concat(buffer);
      html.concat("      <label>for</label>\n");
      sprintf(buffer, "      <input id='imp_%d_0' name='imp_%d_0'  value='%d' type='number' min='10' max='999'/>\n",i, i, Valves->at(i).port1ms);
      html.concat(buffer);
      html.concat("      <label>ms</label>\n");
      html.concat("    </div>\n");
      
      sprintf(buffer, "    <div id='PortB_%d'>\n", i);
      html.concat(buffer);
      html.concat("      <div class='inline'>\n");
      sprintf(buffer, "      <input id='AllePorts_PortB_%d' name='pcfport_%d_1' type='number' min='0' max='220' value='%d'/></div>\n",i, i, Valves->at(i).GetPort2());
      html.concat(buffer);
      html.concat("      <label>for</label>\n");
      sprintf(buffer, "      <input id='imp_%d_1' name='imp_%d_1'  value='%d' type='number' min='10' max='999'/>\n",i, i, Valves->at(i).port2ms);
      html.concat(buffer);
      html.concat("      <label>ms</label>\n");
      html.concat("    </div>\n");
    } else if (Valves->at(i).GetValveType() == "n") {
      sprintf(buffer, "    <div id='Port_%d'>\n", i);
      html.concat(buffer);
      sprintf(buffer, "      <input id='AllePorts_%d' name='pcfport_%d_0' type='number' min='0' max='220' value='%d'/>\n",i, i, Valves->at(i).GetPort1());
      html.concat(buffer);
      html.concat("    </div>\n");
    } 
    html.concat("  </td>\n");
    html.concat("  <td>\n");
    sprintf(buffer, "    <div class='inline'><input type='radio' id='type_%d_0' name='type_%d' value='n' %s onclick='chg_type(this.id)' /><label for='type_%d_0'>normal</label></div>\n",i, i, (Valves->at(i).GetValveType()=="n"?"checked":""),i);
    html.concat(buffer);
    sprintf(buffer, "    <div class='inline'><input type='radio' id='type_%d_1' name='type_%d' value='b' %s onclick='chg_type(this.id)' /><label for='type_%d_1'>bistabil</label></div>\n",i, i, (Valves->at(i).GetValveType()=="b"?"checked":""),i);
    html.concat(buffer);
    html.concat("  </td>\n");

    html.concat("  <td>\n");
    html.concat("    <div class='onoffswitch'>\n");
    sprintf(buffer, "      <input type='checkbox' name='reverse_%d' class='onoffswitch-checkbox' id='myreverseswitch_%d' %s>\n", i, i, (Valves->at(i).reverse?"checked":""));
    html.concat(buffer);
    sprintf(buffer, "      <label class='onoffswitch-label' for='myreverseswitch_%d'>\n", i);
    html.concat(buffer);
    html.concat("        <span class='onoffswitch-inner'></span>\n");
    html.concat("        <span class='onoffswitch-switch'></span>\n");
    html.concat("      </label>\n");
    html.concat("    </div>\n");
    html.concat("  </td>\n");

    html.concat("  <td>\n");
    sprintf(buffer, "      <input id='autooff_%d' name='autooff_%d' type='number' min='0' max='65000' value='%d'/>\n",i, i, Valves->at(i).autooff);
    html.concat(buffer);
    html.concat("  </td>\n");
    
    html.concat("  <td><input type='button' value='&#10008;' onclick='delrow(this)'></td>\n");

    html.concat("  <td>\n");
    sprintf(buffer, "    <input type='button' id='action_%d' value='Set %s' onClick='ChangeValve(this.id)'/>\n", i, (!Valves->at(i).active?"On":"Off"));
    html.concat(buffer);
    html.concat("  </td>\n");

    html.concat("</tr>\n");
    server->sendContent(html.c_str()); html = "";
  }
  html.concat("</tbody>\n");
  html.concat("</table>\n");
  html.concat("</form>\n\n<br />\n");
  html.concat("<form id='jsonform' action='StoreVentilConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html.concat("  <input type='text' id='json' name='json' />\n");
  html.concat("  <input type='submit' value='Speichern' />\n");
  html.concat("</form>\n\n");
  html.concat("<div id='ErrorText' class='errortext'></div>\n");
  server->sendContent(html.c_str()); html = "";
}

void valveStructure::getWebJsParameter(String* html) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  // bereits belegte Ports, können nicht ausgewählt werden (zb.i2c-ports)
  // const gpio_disabled = Array(0,4);
  sprintf(buffer, "const gpio_disabled = [%d,%d];\n", Config->GetPinSDA() + 200, Config->GetPinSCL() + 200);
  html->concat(buffer);

  // anhand gefundener pcf Devices die verfügbaren Ports bereit stellen
  //const pcf_found = [65,72];
  html->concat("const availablePorts = [");
  uint8_t count=0;
  for (uint8_t p=1; p<=254; p++) {
    if (ValveHW->IsValidPort(p) && I2Cdetect->i2cIsPresent(ValveHW->GetI2CAddress(p)) && (!Config->EnabledOled() || Config->GetI2cOLED()!=ValveHW->GetI2CAddress(p))) {
      sprintf(buffer, "%s%d", (count>0?",":"") , p);
      html->concat(buffer);
      count++;
    }
  }
  html->concat("];\n");

  //konfigurierte Ports / Namen
  //const configuredPorts = [ {port:65, name:"Ventil1"}, {port:67, name:"Ventil2"}]
  html->concat("const configuredPorts = [");
  for(int i=0; i < Valves->size(); i++) {
    sprintf(buffer, "{port:%d, name:'%s'}%s", Valves->at(i).GetPort1() ,Valves->at(i).subtopic.c_str(), (i<Valves->size()-1?",":""));
    html->concat(buffer);
  }
  html->concat("];\n");
}
