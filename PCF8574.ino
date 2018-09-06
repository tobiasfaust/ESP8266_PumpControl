/*
 * PCF8574 GPIO Port Expand
 * http://nopnop2002.webcrow.jp/WeMos/WeMos-25.html
 *
 * PCF8574    ----- WeMos
 * A0         ----- GRD
 * A1         ----- GRD
 * A2         ----- GRD
 * VSS        ----- GRD
 * VDD        ----- 5V/3.3V
 * SDA        ----- GPIO_4(PullUp)
 * SCL        ----- GPIO_5(PullUp)
 *
 * P0     ----------------- LED0
 * P1     ----------------- LED1
 * P2     ----------------- LED2
 * P3     ----------------- LED3
 * P4     ----------------- LED4
 * P5     ----------------- LED5
 * P6     ----------------- LED6
 * P7     ----------------- LED7
 *
 */


// https://github.com/xreef/PCF8574_library 

typedef struct {
    uint8_t address;
} pcf8547Host;

typedef struct {
  PCF8574* pcf8574;
  boolean enabled;
  unsigned long startmillis;
  unsigned int lengthmillis;
  unsigned int port;
} pcf8574Device;

pcf8574Device* pcf8574dev;
PCF8574* pcf8574;

//uint8_t sda = 4;
//uint8_t scl = 0;
//PCF8574 pcf8574_test(0x38, sda, scl);

void PCF8574_setup()
{
  
  pcf8574 = new PCF8574(0x38, pin_sda, pin_scl);
  pcf8574dev = new pcf8574Device[8];
  
  Serial.print("Started PCF8574: PCF8574(");
  Serial.print(i2caddress_pfc8574);
  Serial.print(", ");Serial.print(pin_sda);
  Serial.print(", ");Serial.print(pin_scl);
  Serial.println(")");
 
  // Set pinMode to OUTPUT
  for (unsigned int i=0; i<8; i++) {
    pcf8574->pinMode(i, OUTPUT);
    //pcf8574_test.pinMode(i, OUTPUT);
    //TODO: setzen nach Config
    pcf8574dev[i].port = i;
    pcf8574dev[i].enabled = true;
    pcf8574dev[i].startmillis = 0;
    pcf8574dev[i].pcf8574 = pcf8574;
  }
  pcf8574->begin();
  //pcf8574_test.begin();

  for(int i=0;i<8;i++) {
    //pcf8574_test.digitalWrite(i, HIGH);
    pcf8574->digitalWrite(i, HIGH);
  }
  
  pcf8574->digitalWrite(0, LOW);
  delay(2000);
  pcf8574->digitalWrite(0, HIGH);
}

/*
 * wird aus dem MQTT Callback aufgerufen
 */
void PCF8574_onfortimer(char* msg, int port) {
  Serial.print("on-for-timer: Pin");Serial.println(port);
  for (unsigned int i=0; i<8; i++) {
    if (atoi(msg) && atoi(msg)>0 && pcf8574dev[i].port == port) {
      Serial.print("aktiviere Pin ");Serial.println(i);
      pcf8574dev[i].pcf8574->digitalWrite(pcf8574dev[i].port , LOW); 
      pcf8574dev[i].startmillis = millis();
      pcf8574dev[i].lengthmillis = atoi(msg)*1000;

      //pcf8574_test.digitalWrite(i, LOW);
    }
  }
}

void PCF8574_loop(){
  for (unsigned int i=0; i<8; i++) {
      if (pcf8574dev[i].enabled && pcf8574dev[i].startmillis != 0 && (millis() - pcf8574dev[i].startmillis > pcf8574dev[i].lengthmillis)) {
      Serial.print("on-for-timer abgelaufen: Pin");Serial.println(i);
      pcf8574dev[i].pcf8574->digitalWrite(pcf8574dev[i].port, HIGH);
      pcf8574dev[i].startmillis = 0;

      //pcf8574_test.digitalWrite(i, HIGH);
    }
  }
}

