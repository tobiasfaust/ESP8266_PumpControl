#include "valveRelation.h"

valveRelation::valveRelation() {
  _relationen  = new std::vector<relation_t>{};
  _subscriber  = new std::vector<subscriber_t>{};
  LoadJsonConfig();
}

void valveRelation::AddRelation(bool enabled, String TriggerTopic, uint8_t Port, bool EnableByBypass) {
  relation_t rel;
  rel.enabled = enabled;
  rel.TriggerTopic = TriggerTopic;
  rel.ActorPort = Port;
  rel.EnableByBypass = EnableByBypass;
  _relationen->push_back(rel);

  if (enabled) { mqtt->Subscribe(TriggerTopic, MQTT::RELATION); }
}

void valveRelation::GetPortDependencies(std::vector<uint8_t>* Ports, String TriggerTopic) {
  for (uint8_t i=0; i<_relationen->size(); i++) {
    if (_relationen->at(i).TriggerTopic == TriggerTopic && _relationen->at(i).enabled) {
      bool stillpresent=false;
      for (uint8_t j=0; j<Ports->size(); j++) {
        if (Ports->at(j) == _relationen->at(i).ActorPort) {stillpresent=true;}
      }
      if (!stillpresent) {Ports->push_back(_relationen->at(i).ActorPort); }
    }
  }
}

/****************************************************************************
* prueft ob die Relation das Bypass Flag gesetzt hat
*****************************************************************************/
bool valveRelation::CheckEnabledByBypass(uint8_t ActorPort, String TriggerTopic) {
  for (uint8_t i=0; i<_relationen->size(); i++) {
    if (_relationen->at(i).TriggerTopic == TriggerTopic && _relationen->at(i).ActorPort == ActorPort && _relationen->at(i).EnableByBypass) {
      return true;
    }
  }
  return false;
}

void valveRelation::AddSubscriber(uint8_t ActorPort, String TriggerTopic) {
  subscriber_t s;
  s.TriggerTopic = TriggerTopic;
  s.ActorPort = ActorPort;
  _subscriber->push_back(s); 
}

void valveRelation::DelSubscriber(String TriggerTopic) {
  //lösche TriggerTopic auf dem ActorPort
// --> hier tritt eine out-of-range exception auf
  std::vector<subscriber_t> t;
  for (uint8_t i=0; i<_subscriber->size(); i++) {
    if (_subscriber->at(i).TriggerTopic != TriggerTopic) { t.push_back(_subscriber->at(i)); }
  }
  _subscriber->clear();
  
  for (uint8_t i=0; i<t.size(); i++) {
    _subscriber->push_back(t.at(i));
  }
}

uint8_t valveRelation::CountActiveSubscribers(uint8_t ActorPort) {
  uint8_t count=0;
  for (uint8_t i=0; i<_subscriber->size(); i++) {
    if (_subscriber->at(i).ActorPort == ActorPort) {count++;}
  }
  return count;
}

void valveRelation::StoreJsonConfig(String* json) {
  StaticJsonDocument<512> doc;
  deserializeJson(doc, *json);
  JsonObject root = doc.as<JsonObject>();

  if (!root.isNull()) {
    File configFile = SPIFFS.open("/Relations.json", "w");
    if (!configFile) {
      if (Config->GetDebugLevel() >=0) {Serial.println("failed to open Relations.json file for writing");}
    } else {  
      serializeJsonPretty(doc, Serial);
      if (serializeJson(doc, configFile) == 0) {
        if (Config->GetDebugLevel() >=0) {Serial.println(F("Failed to write to file"));}
      }
      configFile.close();
  
      LoadJsonConfig();
    }
  }
}

void valveRelation::LoadJsonConfig() {
  _relationen->clear(); // leere den Valve Vector bevor neu befüllt wird
  _subscriber->clear();
  mqtt->ClearSubscriptions(MQTT::RELATION);
  
  bool loadDefaultConfig = false;
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));

  if (SPIFFS.exists("/Relations.json")) {
    //file exists, reading and loading
    Serial.println("reading Relations.json file");
    File configFile = SPIFFS.open("/Relations.json", "r");
    if (configFile) {
      Serial.println("opened Relations.json file");
      //size_t size = configFile.size();

      StaticJsonDocument<512> doc; // TODO Use computed size??
      DeserializationError error = deserializeJson(doc, configFile);
      
      if (!error) {
        serializeJsonPretty(doc, Serial);
        
        uint8_t count = 0;
        if (doc.containsKey("count")) { count = doc["count"].as<int>(); }
        if(count == 0) {
          Serial.println("something went wrong with Relations.json config, load default config");
          loadDefaultConfig = true;
        }
        
        for (uint8_t i=0; i<count; i++) {
          bool enabled = false;
          bool EnableByBypass = false;
          String SubTopic = "";
          uint8_t Port = 0;
          
          sprintf(buffer, "active_%d", i);
          if (doc[buffer] && doc[buffer] == 1) {enabled = true;} else {enabled = false;}
            
          sprintf(buffer, "mqtttopic_%d", i);
          if (doc.containsKey(buffer)) {SubTopic = doc[buffer].as<String>();}
 
          sprintf(buffer, "port_%d", i);
          if (doc.containsKey(buffer) && doc[buffer].as<int>() > 0) { Port = doc[buffer].as<int>();}

          sprintf(buffer, "EnableByBypass_%d", i);
          if (doc[buffer] && doc[buffer] == 1) {EnableByBypass = true;} else {EnableByBypass = false;}

          AddRelation(enabled, SubTopic, Port, EnableByBypass);
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
    Serial.println("lade Relations DefaultConfig");
    AddRelation(false, "testhost/TestValve1", 203, false);
    AddRelation(false, "testhost/TestValve2", 203, false);
  }
}

void valveRelation::GetWebContent(AsyncResponseStream *response) {
  response->println("<p><input type='button' value='&#10010; add new Port' onclick='addrow(\"maintable\")'></p>");
  response->println("<form id='DataForm'>");
  response->println("<table id='maintable' class='editorDemoTable'>");
  response->println("<thead>");
  response->println("<tr>");
  response->println("<td style='width: 25px;'>Nr</td>");
  response->println("<td style='width: 25px;'>Active</td>");
  response->println("<td style='width: 250px;'>Trigger Topic</td>");
  response->println("<td style='width: 250px;'>Port</td>");
  response->println("<td style='width: 25px;'>Enable if Bypass</td>");
  response->println("<td style='width: 25px;'>Delete</td>");

  response->println("</tr>");
  response->println("</thead>");
  response->println("<tbody>");

  for (uint8_t i=0; i< _relationen->size(); i++) {
    response->println("<tr>");
    response->printf("  <td>%d</td>", i+1);
    response->println("  <td>");
    response->println("    <div class='onoffswitch'>");
    response->printf("      <input type='checkbox' name='active_%d' class='onoffswitch-checkbox' id='myonoffswitch_%d' %s>", i, i, (_relationen->at(i).enabled?"checked":""));
    response->printf("      <label class='onoffswitch-label' for='myonoffswitch_%d'>", i);
    response->println("        <span class='onoffswitch-inner'></span>");
    response->println("        <span class='onoffswitch-switch'></span>");
    response->println("      </label>");
    response->println("    </div>");
    response->println("  </td>");

    response->printf("  <td><input id='mqtttopic_%d' name='mqtttopic_%d' type='text' size='30' value='%s'/></td>", i, i, _relationen->at(i).TriggerTopic.c_str());
    response->printf("  <td><input id='ConfiguredPorts_%d' name='port_%d' type='number' min='10' max='999' value='%d'/></td>", i, i, _relationen->at(i).ActorPort);
    
    response->println("  <td>");
    response->println("    <div class='onoffswitch'>");
    response->printf("      <input type='checkbox' name='EnableByBypass_%d' class='onoffswitch-checkbox' id='BypassOnOffSwitch_%d' %s>", i, i, (_relationen->at(i).EnableByBypass?"checked":""));
    response->printf("      <label class='onoffswitch-label' for='BypassOnOffSwitch_%d'>", i);
    response->println("        <span class='onoffswitch-inner'></span>");
    response->println("        <span class='onoffswitch-switch'></span>");
    response->println("      </label>");
    response->println("    </div>");
    response->println("  </td>");
        
    response->println("  <td><input type='button' value='&#10008;' onclick='delrow(this)'></td>");
    response->println("</tr>");
  }

  response->println("</tbody>");
  response->println("</table>");
  response->println("</form><br />");
  response->println("<form id='jsonform' action='StoreRelations' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>");
  response->println("  <input type='text' id='json' name='json' />");
  response->println("  <input type='submit' value='Speichern' />");
  response->println("</form>");
}
