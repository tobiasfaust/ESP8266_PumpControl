#include "oled.h"

OLED::OLED() : enabled(false) {};

void OLED::init(uint8_t sda, uint8_t scl, uint8_t i2cAddress) {
  
  this->SensorLevel = 0;
  this->pin_sda = sda;
  this->pin_scl = scl;
  this->i2cAddress = i2cAddress;
  
  ssd = new SSD1306Wire(this->i2cAddress, this->pin_sda, this->pin_scl);
  
  Serial.print("Starting OLED 1306: (");
  Serial.print(this->i2cAddress, HEX);
  Serial.print(", ");Serial.print(pin_sda);
  Serial.print(", ");Serial.print(pin_scl);
  Serial.println(")");
  
  ssd->init();
  ssd->flipScreenVertically();
  Serial.println("OLED Ready");
  this->enabled = false;
}

void OLED::Enable(bool e) {
  Serial.println((e?"OLED enabled":"OLED disabled"));
  this->enabled = e;
  if (e) { UpdateAll();}
}

void OLED::SetIP(String ip) {
  if(this->ip != ip) {
    this->ip = ip;
    UpdateAll();
  }
}

void OLED::SetSSID(String ssid) {
  if(this->ssid != ssid) {
    this->ssid = ssid;
    UpdateAll();
  }
}

void OLED::SetRSSI(int rssi) {
  if(ssd && this->enabled && (abs(rssi) < abs(this->rssi)-3 || abs(rssi) > abs(this->rssi)+3)) { 
    this->rssi = rssi;
    display_wifibars();
    ssd->display(); 
  }
}

void OLED::SetWiFiConnected(bool c) {
  if(this->WiFiConnected != c) {
    this->WiFiConnected = c;
    UpdateAll(); 
    Serial.print(F("OLED: Change WiFi Connect Status to ")); Serial.println(c);
  }
}

void OLED::SetMqttConnected(bool c) {
  if(this->MqttConnected != c) {
    this->MqttConnected = c;
    if (this->enabled) { display_MqttConnectInfo(); }
    Serial.print(F("OLED: Change MQTT Connect Status to ")); Serial.println(c);
  }
}

void OLED::UpdateAll() {
  if (ssd && this->enabled) {
    Serial.println("OLED Update All");
    ssd->clear();
    display_header();
    display_wifibars();
    display_MqttConnectInfo();
    SetLevel(this->SensorLevel, true);
    ssd->display(); 
  }
}

void OLED::SetLevel(uint8_t Level) {
  this->SetLevel(Level, false);
}

void OLED::SetLevel(uint8_t Level, bool force) {
  if(ssd && this->enabled && (this->SensorLevel != Level || force)) {
    this->SensorLevel = Level;
    ssd->setColor(BLACK);
    ssd->fillRect(1, 32, 127, 63); // 
    ssd->setColor(WHITE);
    ssd->setTextAlignment(TEXT_ALIGN_CENTER);
    ssd->setFont(ArialMT_Plain_24);
    ssd->drawString(64, 35, String(this->SensorLevel) + "%");
    ssd->display();
  }
}

void OLED::SetDeviceName(String s) {
  if(this->DeviceName != s) {
    this->DeviceName = s;
    UpdateAll();
  }
}

void OLED::display_header() {
  String title,subtitle = "";
  if (this->WiFiConnected) {
    title = this->ssid;
    subtitle = this->ip;
    title.trim();
    subtitle.trim();
  } else {
    title = this->DeviceName;
    subtitle = F("Try to connect...");
    title.trim();
    subtitle.trim();
  }
  
  ssd->setTextAlignment(TEXT_ALIGN_CENTER);
  ssd->setFont(ArialMT_Plain_10);
  ssd->setColor(WHITE);
  ssd->drawString(56, 1, title);
  ssd->drawString(56, 15, subtitle);
}

void OLED::display_MqttConnectInfo() {
  ssd->setFont(ArialMT_Plain_10);
  ssd->setTextAlignment(TEXT_ALIGN_LEFT);
  int x = 100;
  int y = 15;
  String text("MQTT");
  int width = ssd->getStringWidth(text);
  
  ssd->setColor(BLACK);
  ssd->fillRect(x, y, x+width, y+10);
  ssd->setColor(WHITE);
  ssd->drawString(x, y, text);
  if(!this->MqttConnected) {
    ssd->drawHorizontalLine(x, y+6, width);
  }
  ssd->display();
}

void OLED::display_wifibars() {
  // https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P036_FrameOLED.ino
  const int nbars_filled = (this->rssi + 100) / 8;
  
  int x = 110;
  int y = 3; //0
  int size_x = 15;
  int size_y = 10; //10
  int nbars = 5;
  int16_t width = (size_x / nbars);
  size_x = width * nbars - 1; // Correct for round errors.

  //  x,y are the x,y locations
  //  sizex,sizey are the sizes (should be a multiple of the number of bars)
  //  nbars is the number of bars and nbars_filled is the number of filled bars.

  //  We leave a 1 pixel gap between bars
  ssd->setColor(BLACK);
  ssd->fillRect(x , y, size_x, size_y);
  ssd->setColor(WHITE);
  if (this->WiFiConnected) {
    for (byte ibar = 0; ibar < nbars; ibar++) {
      int16_t height = size_y * (ibar + 1) / nbars;
      int16_t xpos = x + ibar * width;
      int16_t ypos = y + size_y - height;
      if (ibar <= nbars_filled) {
        // Fill complete bar
        ssd->fillRect(xpos, ypos, width - 1, height);
      } else {
        // Only draw top and bottom.
        ssd->fillRect(xpos, ypos, width - 1, 1);
        ssd->fillRect(xpos, y + size_y - 1, width - 1, 1);
      }
    }
  } else {
    // Draw a not connected sign.
  }
}
