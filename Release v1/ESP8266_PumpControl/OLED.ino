// https://github.com/ThingPulse/esp8266-oled-ssd1306

//#include <Adafruit_GFX.h>
#include "SSD1306Wire.h"

SSD1306Wire* ssd = NULL;

void oled_setup() {
  // Read parameters previously from SPIFFS via getParameters procedure
  Serial.print("Starting OLED 1306: (");
  Serial.print(i2caddress_oled);
  Serial.print(", ");Serial.print(pin_sda);
  Serial.print(", ");Serial.print(pin_scl);
  Serial.println(")");

  ssd = new SSD1306Wire(i2caddress_oled, pin_sda, pin_scl);
  ssd->init();
  ssd->flipScreenVertically();
  ssd->clear();
  display_header();
  ssd->display();
}

void oled_loop() {
  // clear the display
  ssd->clear();
  // draw the percentage as String
  ssd->setTextAlignment(TEXT_ALIGN_CENTER);
  ssd->setFont(ArialMT_Plain_24);
  ssd->drawString(64, 35, String(hcsr04_level) + "%"); 

  /*long rssi = WiFi.RSSI();
  int bars = getBarsSignal(rssi);
  for (int b=0; b <= bars; b++) {
    // display.fillRect(59 + (b*5),33 - (b*5),3,b*5,WHITE); 
    ssd->fillRect(10 + (b*5),48 - (b*5),3,b*5); 
  }
  */
  display_header();
    
  // write the buffer to the display
  ssd->display();
}


void display_header() {
  if (WiFi.status() == WL_CONNECTED) {
    String title = WiFi.SSID();
    String subtitle = WiFi.localIP().toString();
    title.trim();
    subtitle.trim();
    display_title(title, subtitle);
  } else {
    String title = mqtt_root;
    String subtitle = F("connecting...");
    title.trim();
    subtitle.trim();
    display_title(title, subtitle);
  }
  display_wifibars();
}

void display_title(String& title, String& subtitle) {
  ssd->setTextAlignment(TEXT_ALIGN_CENTER);
  ssd->setFont(ArialMT_Plain_10);
  //ssd->setColor(BLACK);
  //ssd->fillRect(0, 0, 128, 26); // Underscores use a extra lines, clear also.
  ssd->setColor(WHITE);
  ssd->drawString(64, 0, title);
  ssd->drawString(64, 13, subtitle);
}


void display_wifibars() {
  // https://github.com/letscontrolit/ESPEasy/blob/mega/src/_P036_FrameOLED.ino
  const int nbars_filled = (WiFi.RSSI() + 100) / 8;
  
  int x = 105;
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
  if (WiFi.status() == WL_CONNECTED) {
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

/*
int getBarsSignal(long rssi){
  // 5. High quality: 90% ~= -55db
  // 4. Good quality: 75% ~= -65db
  // 3. Medium quality: 50% ~= -75db
  // 2. Low quality: 30% ~= -85db
  // 1. Unusable quality: 8% ~= -96db
  // 0. No signal
  int bars;
  
  if (rssi > -55) { 
    bars = 5;
  } else if (rssi < -55 & rssi > -65) {
    bars = 4;
  } else if (rssi < -65 & rssi > -75) {
    bars = 3;
  } else if (rssi < -75 & rssi > -85) {
    bars = 2;
  } else if (rssi < -85 & rssi > -96) {
    bars = 1;
  } else {
    bars = 0;
  }
  return bars;
}
*/
