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


pcf8574Device* pcf8574dev;
uint8_t pcf8574devCount = 0;
PCF8574* pcf8574;

void PCF8574_setup()
{  
  Serial.println("Starting PCF8574 ...");
  pcf8574 = new PCF8574(i2caddress_pfc8574, pin_sda, pin_scl);
  pcf8574devCount = 16;
  pcf8574dev = new pcf8574Device[pcf8574devCount];
  
  
  Serial.print("Started PCF8574: PCF8574(");
  Serial.print(i2caddress_pfc8574);
  Serial.print(", ");Serial.print(pin_sda);
  Serial.print(", ");Serial.print(pin_scl);
  Serial.println(")");
 
  // Set pinMode to OUTPUT
  for (unsigned int i=0; i < pcf8574devCount; i++) {
    
    pcf8574->pinMode(i, OUTPUT);
    pcf8574dev[i].port = 65+i;
    pcf8574dev[i].enabled = true;
    pcf8574dev[i].startmillis = 0;
    pcf8574dev[i].pcf8574 = pcf8574;
    sprintf(pcf8574dev[i].subtopic, "Ventil%d", i+1);
  }
  pcf8574->begin();

  for(int i=0;i < pcf8574devCount; i++) {
    pcf8574->digitalWrite(i, HIGH);
  }

  // Test
  pcf8574->digitalWrite(3, LOW);
  delay(2000);
  pcf8574->digitalWrite(3, HIGH);
  
}

// wird aus dem MQTT Callback aufgerufen

void PCF8574_onfortimer(int* duration, int port) {
  Serial.print("on-for-timer: Pin ");Serial.print(port);Serial.print(" , duration: ");Serial.println(*duration);
  
  for (unsigned int i=0; i < pcf8574devCount; i++) {
    //Serial.print(i);Serial.print(") Suche Port ");Serial.print(pcf8574dev[i].port);Serial.print("==");Serial.println(port);
    if (duration > 0 && pcf8574dev[i].port == port) {
      Serial.print("aktiviere Pin ");Serial.println(i);
      pcf8574dev[i].pcf8574->digitalWrite(pcf8574dev[i].port , LOW); 
      pcf8574dev[i].startmillis = millis();
      pcf8574dev[i].lengthmillis = *duration * 1000;

      char subtopic[50];
      memset(&subtopic[0], 0, sizeof(subtopic));
      snprintf (subtopic, sizeof(subtopic), "%s/state", pcf8574dev[i].subtopic);
      MQTT_publish(subtopic, true);
    }
  }
}

void PCF8574_loop(){
  for (unsigned int i=0; i < pcf8574devCount; i++) {
      if (pcf8574dev[i].enabled && pcf8574dev[i].startmillis != 0 && (millis() - pcf8574dev[i].startmillis > pcf8574dev[i].lengthmillis)) {
      Serial.print("on-for-timer abgelaufen: Pin");Serial.println(i);
      pcf8574dev[i].pcf8574->digitalWrite(pcf8574dev[i].port, HIGH);
      pcf8574dev[i].startmillis = 0;

      char subtopic[50];
      memset(&subtopic[0], 0, sizeof(subtopic));
      snprintf (subtopic, sizeof(subtopic), "%s/state", pcf8574dev[i].subtopic);
      MQTT_publish(subtopic, false);
    }
  }
}

// see Definition: https://www.letscontrolit.com/wiki/index.php/PCF8574
void GetPCF8574Port (pcf8574Port* t, uint8_t port) {
  if (port >=65 && port <=72) {
    t->i2cAddress=0x38;
    t->port=port-65;
  } else if (port >=73 && port <=80) {
    t->i2cAddress=0x39;
    t->port=port-73;
  } else if (port >=81 && port <=88) {
    t->i2cAddress=0x3A;
    t->port=port-81;
  } else if (port >=89 && port <=96) {
    t->i2cAddress=0x3B;
    t->port=port-89;
  } else if (port >=97 && port <=104) {
    t->i2cAddress=0x3C;
    t->port=port-97;
  } else if (port >=105 && port <=112) {
    t->i2cAddress=0x3D;
    t->port=port-105;
  } else if (port >=113 && port <=112) {
    t->i2cAddress=0x3E;
    t->port=port-113;
  } else if (port >=121 && port <=128) {
    t->i2cAddress=0x3F;
    t->port=port-121;
  }
}


