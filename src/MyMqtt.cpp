#include <MyMqtt.h>

MyMQTT::MyMQTT(AsyncWebServer* server, DNSServer *dns, const char* MqttServer, uint16_t MqttPort, String MqttBasepath, String MqttRoot, char* APName, char* APpassword): 
  MQTT(server, dns, MqttServer, MqttPort, MqttBasepath, MqttRoot, APName, APpassword) {
  
  this->subscriptions = new std::vector<subscription_t>{};
}

#ifdef USE_OLED
void MyMQTT::SetOled(OLED* oled) {
  this->oled = oled;  
}
#endif

void MyMQTT::loop() {
  MQTT::loop();

  #ifdef USE_OLED
  if(this->oled) {
    this->oled->SetMqttConnected(MQTT::GetConnectStatusMqtt());
    this->oled->SetWiFiConnected(WiFi.isConnected());
    
    if (WiFi.status() == WL_CONNECTED) {
      this->oled->SetIP(WiFi.localIP().toString());
      this->oled->SetRSSI(WiFi.RSSI());
      this->oled->SetSSID(WiFi.SSID());
    }
  }
  #endif
}

void MyMQTT::reSubscribe() {
  String topic = MQTT::getTopic("#", false);
  MQTT::Subscribe(topic);
  if (Config->GetDebugLevel()>=3) {
    Serial.printf("MyMQTT Subscribed to myself: %s\n", topic.c_str());
  }

  for (uint8_t i=0; i< this->subscriptions->size(); i++) {
    if (this->subscriptions->at(i).active == true) {
      MQTT::Subscribe(this->subscriptions->at(i).subscription.c_str()); 
      if (Config->GetDebugLevel()>=3) {
        Serial.printf("MyMQTT Subscribed to: %s\n", this->subscriptions->at(i).subscription.c_str());
      }
    }
  }
}

/*******************************************************
 * subscribe to a special topic (without /# at end)
*******************************************************/
void MyMQTT::Subscribe(String topic, MqttSubscriptionType_t identifier) {
  //char buffer[100] = {0};
  //memset(buffer, 0, sizeof(buffer));
  subscription_t sub = {};
  //snprintf(buffer, sizeof(buffer), "%s/#", topic.c_str());
  sub.subscription = topic;
  sub.identifier = identifier;
  sub.active = true;
  this->subscriptions->push_back(sub);
  MQTT::Subscribe(sub.subscription.c_str()); 
}

void MyMQTT::ClearSubscriptions(MqttSubscriptionType_t identifier) {
  std::vector<MyMQTT::subscription_t> t;

  for ( uint8_t i=0; i< this->subscriptions->size(); i++) {
    if (this->subscriptions->at(i).identifier != identifier) { 
      t.push_back(this->subscriptions->at(i));
    }
  }

  this->subscriptions->clear();
  for (uint8_t i=0; i<t.size(); i++) {
    this->subscriptions->push_back(t.at(i));
  }
  this->subscriptions->shrink_to_fit();

  MQTT::ClearSubscriptions();
  this->reSubscribe();
}

