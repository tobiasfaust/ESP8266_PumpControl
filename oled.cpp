#include "oled.h"

OLED::OLED() : enabled(false) {};

void OLED::init(uint8_t sda, uint8_t scl, uint8_t i2cAddress) {
  
  this->SensorLevel = 0;
  this->pin_sda = sda;
  this->pin_scl = scl;
  this->i2cAddress = i2cAddress;
  this->type = Config->GetOledType();

  if (Config->GetDebugLevel() >=4) Serial.printf("Starting OLED: I2c: 0x%02X, SDA: %d, SCL: %d, Type: %d )\n", this->i2cAddress, this->pin_sda, pin_scl, this->type);
  
  ssd = new OLEDWrapper(this->pin_sda, this->pin_scl, this->i2cAddress);
  
  this->ssd->init();
  this->ssd->flipScreenVertically();
  if (Config->GetDebugLevel() >=3) Serial.println("OLED Ready");
  this->enabled = false;
}

void OLED::Enable(bool e) {
  if (Config->GetDebugLevel() >=3) Serial.println((e?"OLED enabled":"OLED disabled"));
  this->enabled = e;
}

void OLED::loop() {
  if (Config->EnabledOled() != this->GetEnabled()) {
    if (Config->EnabledOled()) this->init(Config->GetPinSDA(), Config->GetPinSCL(), Config->GetI2cOLED());
    this->Enable(Config->EnabledOled());
  }

  if (this->type != Config->GetOledType()) {
     this->init(Config->GetPinSDA(), Config->GetPinSCL(), Config->GetI2cOLED());
     this->type = Config->GetOledType();
  }
  
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
    if (Config->GetDebugLevel() >=3) { Serial.print(F("OLED: Change WiFi Connect Status to ")); Serial.println(c);}
  }
}

void OLED::SetMqttConnected(bool c) {
  if(this->MqttConnected != c) {
    this->MqttConnected = c;
    if (this->enabled) { display_MqttConnectInfo(); }
    if (Config->GetDebugLevel() >=3) { Serial.print(F("OLED: Change MQTT Connect Status to ")); Serial.println(c);}
  }
}

void OLED::UpdateAll() {
  if (ssd && this->enabled) {
    if (Config->GetDebugLevel() >=4) Serial.println("OLED Update All");
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

//##########################################################

OLEDWrapper::OLEDWrapper(uint8_t sda, uint8_t scl, uint8_t i2cAddress) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = new SSD1306Wire(i2cAddress, sda, scl);
    this->oled = ssd;
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd= new SH1106(i2cAddress, sda, scl);
    this->oled = ssd;
  }  
}

bool OLEDWrapper::init() {
  bool ret = false;
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ret = ssd->init();    
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ret = ssd->init();
  }
  return ret;
}

void OLEDWrapper::flipScreenVertically() {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->flipScreenVertically();
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->flipScreenVertically();
  }
}

void OLEDWrapper::fillRect(int16_t x, int16_t y, int16_t width, int16_t height) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->fillRect(x, y, width, height);
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->fillRect(x, y, width, height);
  }
}

void OLEDWrapper::setColor(OLEDDISPLAY_COLOR color) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->setColor(color);
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->setColor(color);
  }
}

void OLEDWrapper::setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->setTextAlignment(textAlignment);
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->setTextAlignment(textAlignment);
  }
}

void OLEDWrapper::setFont(const uint8_t *fontData) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->setFont(fontData);
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->setFont(fontData);
  }
}

void OLEDWrapper::drawString(int16_t x, int16_t y, String text) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->drawString(x, y, text);
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->drawString(x, y, text);
  }
}

void OLEDWrapper::drawHorizontalLine(int16_t x, int16_t y, int16_t length) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->drawHorizontalLine(x, y, length);
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->drawHorizontalLine(x, y, length);
  }
}

uint16_t OLEDWrapper::getStringWidth(String text) {
  uint16_t ret = 0;
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ret = ssd->getStringWidth(text);
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ret = ssd->getStringWidth(text);
  }
  return ret;
}

void OLEDWrapper::clear(void) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->clear();
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->clear();
  }
}

void OLEDWrapper::display(void) {
  if (Config->GetOledType() == 0) {
    SSD1306Wire* ssd = static_cast<SSD1306Wire*>(this->oled);
    ssd->display();
  } else if (Config->GetOledType() == 1) {
    SH1106* ssd = static_cast<SH1106*>(this->oled);
    ssd->display();
  }
}
