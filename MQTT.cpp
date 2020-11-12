#include "MQTT.h"

MQTT::MQTT(const char* server, uint16_t port, String root) {  
  this->mqtt_root = root;
  this->subscriptions = new std::vector<subscription_t>{};
  espClient = WiFiClient();
  WiFi.mode(WIFI_STA);
    
  this->mqtt = new PubSubClient();
  
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false); // disable for testing
  wifiManager.setTimeout(300);
  wifi_station_set_hostname(mqtt_root.c_str());
  
  if (!wifiManager.autoConnect(mqtt_root.c_str())) {
    Serial.println("failed to connect and hit timeout");
    if (oled->GetEnabled()) {
      oled->SetWiFiConnected(false);
    }
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  
  Serial.print("WiFi connected with local IP: ");
  
  if (oled && oled->GetEnabled()) {
    oled->SetIP(WiFi.localIP().toString());
    oled->SetRSSI(WiFi.RSSI());
    oled->SetSSID(WiFi.SSID());
    oled->SetWiFiConnected(true);
  }
  Serial.println(WiFi.localIP());
  //WiFi.printDiag(Serial);

  Serial.print("Starting MQTT (");Serial.print(server); Serial.print(":");Serial.print(port);Serial.println(")");
  mqtt->setClient(espClient);
  mqtt->setServer(server, port);
  mqtt->setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
}

void MQTT::reconnect() {
  char topic[50];
  char LWT[50];
  memset(&LWT[0], 0, sizeof(LWT));
  memset(&topic[0], 0, sizeof(topic));
  
  if (Config->UseRandomMQTTClientID()) { 
    snprintf (topic, sizeof(topic), "%s-%s", this->mqtt_root.c_str(), String(random(0xffff)).c_str());
  } else {
    snprintf (topic, sizeof(topic), "%s-%08X", this->mqtt_root.c_str(), ESP.getFlashChipId());
  }
  snprintf(LWT, sizeof(LWT), "%s/state", this->mqtt_root.c_str());
  
  Serial.printf("Attempting MQTT connection as %s \n", topic);
  
  if (mqtt->connect(topic, Config->GetMqttUsername().c_str(), Config->GetMqttPassword().c_str(), LWT, true, false, "Offline")) {
    Serial.println("connected... ");
    oled->SetMqttConnected(true);
    
    // Once connected, publish basics ...
    this->Publish_IP();
    this->Publish_String("version", (char*)Config->GetReleaseName().c_str());
    this->Publish_String("state", "Online"); //LWT reset
        
    // ... and resubscribe
    snprintf (topic, sizeof(topic), "%s/#", mqtt_root.c_str());
    mqtt->subscribe(topic);
    Serial.print(F("MQTT Subscribed to: ")); Serial.println(FPSTR(topic));

    for (uint8_t i=0; i< this->subscriptions->size(); i++) {
      if (this->subscriptions->at(i).active == true) {
        mqtt->subscribe(this->subscriptions->at(i).subscription.c_str()); 
        Serial.print(F("MQTT Subscribed to: ")); Serial.println(FPSTR(this->subscriptions->at(i).subscription.c_str()));
      }
    }
    
  } else {
    Serial.print(F("failed, rc="));
    Serial.print(mqtt->state());
    Serial.println(F(" try again in few seconds"));
    oled->SetMqttConnected(false);
  }
}

void MQTT::disconnect() {
  mqtt->disconnect();
}

void MQTT::callback(char* topic, byte* payload, unsigned int length) {
  if (MyCallback) {
    MyCallback(topic,payload,length);
  }
}

String MQTT::GetRoot() {
  return mqtt_root;
}

void MQTT::Publish_Bool(const char* subtopic, bool b) {
  char* s = "0"; 
  if(b) {s = "1";};
  Publish_String(subtopic, s);
}

void MQTT::Publish_Int(const char* subtopic, int* number ) {
  char buffer[10] = {0};
  memset(&buffer[0], 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%d", number);
  Publish_String(subtopic, buffer);
}

void MQTT::Publish_String(const char* subtopic, char* value ) {
  char topic[50] = {0};
  memset(&topic[0], 0, sizeof(topic));
  snprintf (topic, sizeof(topic), "%s/%s", this->mqtt_root.c_str(), subtopic);
  if (mqtt->connected()) {
    mqtt->publish((const char*)topic, (const char*)value, true);
    Serial.print(F("Publish ")); Serial.print(FPSTR(topic)); Serial.print(F(": ")); Serial.println(value);
  } else { Serial.println(F("Request for MQTT Publish, but not connected to Broker")); }
}

void MQTT::Publish_IP() {
  char buffer[15] = {0};
  memset(&buffer[0], 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s", WiFi.localIP().toString().c_str());
  Publish_String("IP", buffer);
}

void MQTT::setCallback(CALLBACK_FUNCTION) {
    this->MyCallback = MyCallback;
}

void MQTT::Subscribe(String topic, MqttSubscriptionType_t identifier) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  subscription_t sub = {};
  snprintf(buffer, sizeof(buffer), "%s/#", topic.c_str());
  sub.subscription = buffer;
  sub.identifier = identifier;
  sub.active = true;
  this->subscriptions->push_back(sub);
  if (mqtt->connected()) {mqtt->subscribe(sub.subscription.c_str()); Serial.print(F("MQTT Subscribed to: ")); Serial.println(FPSTR(sub.subscription.c_str()));}
}

void MQTT::ClearSubscriptions(MqttSubscriptionType_t identifier) {
  for ( uint8_t i=0; i< this->subscriptions->size(); i++) {
    if (mqtt->connected() && this->subscriptions->at(i).active == true && this->subscriptions->at(i).identifier == identifier) { 
      mqtt->unsubscribe(this->subscriptions->at(i).subscription.c_str()); 
    }
    if (this->subscriptions->at(i).identifier == identifier) { 
      this->subscriptions->at(i).active = false;
    }
  }
}

void MQTT::loop() {
  if (this->mqtt_root != Config->GetMqttRoot()) {
    Serial.printf("MQTT DeviceName has changed via Web Configuration from %s to %s \n", this->mqtt_root.c_str(), Config->GetMqttRoot().c_str());
    Serial.println(F("Initiate Reconnect"));
    this->mqtt_root = Config->GetMqttRoot();
    if (mqtt->connected()) mqtt->disconnect();
  }
  
  if (!mqtt->connected() && WiFi.status() == WL_CONNECTED) { 
      if (millis() - mqttreconnect_lasttry > 10000) {
        espClient = WiFiClient();
        this->reconnect(); 
        this->mqttreconnect_lasttry = millis();
    }
  } else if (WiFi.status() == WL_CONNECTED) { 
    mqtt->loop();
  }

  if (WiFi.status() == WL_CONNECTED) {
    oled->SetIP(WiFi.localIP().toString());
    oled->SetRSSI(WiFi.RSSI());
    oled->SetSSID(WiFi.SSID());
    oled->SetWiFiConnected(true);
  } else {
    oled->SetWiFiConnected(false);
  }
}
