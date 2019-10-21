#include "MQTT.h"

MQTT::MQTT(const char* server, uint16_t port, String root) {  
  this->mqtt_root = root;
  /*WiFiManager wifiManager;
  wifiManager.setTimeout(300);
  if (!wifiManager.autoConnect(mqtt_root.c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  Serial.print("WiFi connected with local IP: ");
  Serial.println(WiFi.localIP());
  */
  mqtt.setClient(espClient);
  mqtt.setServer(server, port);
  mqtt.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
}

void MQTT::reconnect() {
  char topic[50];
  memset(&topic[0], 0, sizeof(topic));
  Serial.println("Attempting MQTT connection...");
  snprintf (topic, sizeof(topic), "%s-%s", mqtt_root.c_str(), String(random(0xffff)).c_str());
  if (mqtt.connect(mqtt_root.c_str())) {
    Serial.print("connected, subscribe to: ");
    // Once connected, publish an announcement...
    //client.publish("outTopic", "hello world");
    // ... and resubscribe
    snprintf (topic, sizeof(topic), "%s/#", mqtt_root.c_str());
    Serial.println(topic);
    mqtt.subscribe(topic);
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqtt.state());
    Serial.println(" try again in 5 seconds");
  }
}

void MQTT::callback(char* topic, byte* payload, unsigned int length) {
  if (MyCallback) {
    MyCallback(topic,payload,length);
  }
}

String MQTT::GetRoot() {
  return mqtt_root;
}

void MQTT::Publish(const char* subtopic, bool b) {
  char b1[2];
  memset(&b1[0], 0, sizeof(b1));
  if(b) {strcpy(b1, "1");} else {strcpy(b1, "0");}
  Publish(subtopic, b1);
}

void MQTT::Publish(const char* subtopic, int* number ) {
  char buffer[10];
  memset(&buffer[0], 0, sizeof(buffer));
  itoa(*number, buffer, 10);
  Publish(subtopic, buffer);
}

void MQTT::Publish(const char* subtopic, char* value ) {
  char topic[50];
  memset(&topic[0], 0, sizeof(topic));
  snprintf (topic, sizeof(topic), "%s/%s", this->mqtt_root.c_str(), subtopic);
  mqtt.publish(topic, value);
  Serial.print("Publish "); Serial.print(topic); Serial.print(": "); Serial.println(value);
}

void MQTT::setCallback(CALLBACK_FUNCTION) {
    this->MyCallback = MyCallback;
}

void MQTT::loop() {
  if (!mqtt.connected() && WiFi.status() == WL_CONNECTED) { 
      if (millis() - mqttreconnect_lasttry > 10000) {
        this->reconnect(); 
        this->mqttreconnect_lasttry = millis();
    }
  } else if (WiFi.status() == WL_CONNECTED) { 
    mqtt.loop();
  }
}


