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
const uint8_t pcf8574HWCount = 4;
pcf8574HW* pcf8574 = new pcf8574HW[pcf8574HWCount];
//gpiopin* mygpiopin = new gpiopin[11];

void PCF8574_setup() {
  boolean loadDefaultConfig = false;  
  char buffer[20] = {0};
  memset(buffer, 0, sizeof(buffer));
  //int pcf8574_addr[pcf8574HWCount] = {0};
  
  Serial.println("Starting PCF8574 ...");
  
    // ############### Ventil CONFIG ##################
    if (SPIFFS.exists("/VentilConfig.json")) {
      //file exists, reading and loading
      Serial.println("reading ventil config file");
      File configFile = SPIFFS.open("/VentilConfig.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          pcf8574devCount = json["count"];
          uint8_t my_port = 0;
          pcf8574Port my_pcf8574Port;
          boolean i2caddress_found = false;
          uint8_t count = 0;

          pcf8574dev = new pcf8574Device[pcf8574devCount];
          
          for (unsigned int i=0; i < pcf8574devCount; i++) {
            i2caddress_found = false;
            sprintf(buffer, "pcfport_%d", i);
            my_port = json[buffer];
            GetPCF8574Port(&my_pcf8574Port, my_port);
            pcf8574dev[i].port = my_port;
            pcf8574dev[i].pcfport = my_pcf8574Port.port;
            Serial.print("konvertiere Port "); Serial.print(my_port);Serial.print(" zu I2C Adresse ");Serial.println(my_pcf8574Port.i2cAddress);
            
            if (pcf8574dev[i].port < 200 ) {
              // nur für PCF Ports, alle Ports > 200 sind interne GPIOs
              for (unsigned int j=0; j < pcf8574HWCount; j++) {
                if(pcf8574[j].i2cAddress && pcf8574[j].i2cAddress == my_pcf8574Port.i2cAddress) {
                  i2caddress_found = true;
                  pcf8574dev[i].pcf8574 = pcf8574[j].pcf8574;
                }
              }
              if(!i2caddress_found) {
                pcf8574[count].i2cAddress = my_pcf8574Port.i2cAddress;
                pcf8574[count].pcf8574 = new PCF8574(pcf8574[count].i2cAddress, pin_sda, pin_scl);
                pcf8574[count].pcf8574->begin();
                pcf8574dev[i].pcf8574 = pcf8574[count].pcf8574;
                Serial.print("PCF8574 gefunden und initialisiert auf Adresse: "); Serial.println(my_pcf8574Port.i2cAddress);
                count++;
              }
              pcf8574dev[i].pcf8574->pinMode(pcf8574dev[i].pcfport, OUTPUT);
              pcf8574dev[i].pcf8574->digitalWrite(pcf8574dev[i].pcfport, HIGH);
            } else {
              // interne GPIO´s
              pinMode(pcf8574dev[i].pcfport, OUTPUT);
              digitalWrite(pcf8574dev[i].pcfport, LOW);
            }
            
            sprintf(buffer, "active_%d", i);
            if (json[buffer] == 1) {pcf8574dev[i].enabled = true;} else {pcf8574dev[i].enabled = false;}
            pcf8574dev[i].startmillis = 0;
            sprintf(buffer, "mqtttopic_%d", i);
            strncpy(pcf8574dev[i].subtopic, json[buffer], 20);
          }
          
        } else {
          Serial.println("failed to load json config, load default config");
          loadDefaultConfig = true;
        }
      }
    } else {
      Serial.println("VentilConfig.json config File not exists, load default config");
      loadDefaultConfig = true;
    }

    if (loadDefaultConfig) {
      // do something
      Serial.println("load DefaultConfig of PCF8574");
      pcf8574devCount = 16;
      pcf8574dev = new pcf8574Device[pcf8574devCount];
      for (unsigned int i=0; i < pcf8574devCount; i++) { 
        pcf8574dev[i].port = 65+i;
        pcf8574dev[i].enabled = false;
        pcf8574dev[i].startmillis = 0;
        sprintf(pcf8574dev[i].subtopic, "Ventil%d", i+1);
      }
      loadDefaultConfig = false; //set back
    }

  // Zuweisen der Automatik VentilObjekte
  for (unsigned int i=0; i < pcf8574devCount; i++) {
    if(syncswitch_port == pcf8574dev[i].port) {
       syncswitchDevice = i;
    }
    if(ventil3wege_port == pcf8574dev[i].port) {
       ventil3wegeDevice = i;
    }
  }

  /*
  // GPIO-Pin Declaration
  mygpiopin[0].pinnumber = 16;
  strncpy(mygpiopin[0].gpioname, "D0", 10);
  mygpiopin[1].pinnumber = 5;
  strncpy(mygpiopin[1].gpioname, "D1", 10);
  mygpiopin[2].pinnumber = 4;
  strncpy(mygpiopin[2].gpioname, "D2/SDA", 10);
  mygpiopin[3].pinnumber = 0;
  strncpy(mygpiopin[3].gpioname, "D3/SCL", 10);
  mygpiopin[4].pinnumber = 2;
  strncpy(mygpiopin[4].gpioname, "D4", 10);
  mygpiopin[5].pinnumber = 14;
  strncpy(mygpiopin[5].gpioname, "D5", 10);
  mygpiopin[6].pinnumber = 12;
  strncpy(mygpiopin[6].gpioname, "D6", 10);
  mygpiopin[7].pinnumber = 13;
  strncpy(mygpiopin[7].gpioname, "D7", 10);
  mygpiopin[8].pinnumber = 15;
  strncpy(mygpiopin[8].gpioname, "D8", 10);
  mygpiopin[9].pinnumber = 1;
  strncpy(mygpiopin[9].gpioname, "TX", 10); 
  mygpiopin[10].pinnumber = 3;
  strncpy(mygpiopin[10].gpioname, "RX", 10);
  
  mygpiopin[0].active = true;
  mygpiopin[1].active = true;
  mygpiopin[2].active = true;
  mygpiopin[3].active = true;
  mygpiopin[4].active = true;
  mygpiopin[5].active = true;
  mygpiopin[6].active = true;
  mygpiopin[7].active = true;
  mygpiopin[8].active = true;
  mygpiopin[9].active = true;
  mygpiopin[10].active = true;
  */
  // Test
  /*Serial.println("make a first test with PCF Device");
  if (pcf8574dev[0].pcf8574) {
    pcf8574dev[0].pcf8574->digitalWrite(3, LOW);
    delay(2000);
    pcf8574dev[0].pcf8574->digitalWrite(3, HIGH);
  } else {
    Serial.println("Sorry, no PCF Device was enabled in WebUI");
  }
  Serial.println("first test done ..");  
  */
}

// wird aus dem MQTT Callback aufgerufen

void PCF8574_onfortimer(int* duration, pcf8574Device* mydev) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  if (max_parallel == 0 || max_parallel <= parallelThreads) {
    // nur schalten wenn deaktiviert oder MaxParrallelThreads noch nicht erreicht 
    handleSwitch(mydev, true, duration);
    
    if(enable_syncswitch && pcf8574dev[ventil3wegeDevice].active) {
      int syncduration = *duration  - 3; //beende 3sek vorher um Druck in der Leitung abzubauen
      if (syncduration > 0 && pcf8574dev[syncswitchDevice].startmillis + pcf8574dev[syncswitchDevice].lengthmillis <  millis() + syncduration) {
        // Neusetzen nur wenn neue Zeit länger dauert als vorher
        handleSwitch(&pcf8574dev[syncswitchDevice], true, &syncduration);
      }
    }
  } else {
    // verwerfe Schaltvorgang
    // TODO: Queue merken
    sprintf(buffer, "on-for-timer: Port %d abgebrochen da MaxParallel Threads erreicht: %d/%d", mydev->port, parallelThreads, max_parallel );
    Serial.println(buffer);
  }
}

void PCF8574_loop(){
  for (unsigned int i=0; i < pcf8574devCount; i++) {
    if (pcf8574dev[i].enabled && pcf8574dev[i].startmillis != 0 && (millis() - pcf8574dev[i].startmillis > pcf8574dev[i].lengthmillis)) {
      Serial.print("on-for-timer abgelaufen: Pin");Serial.println(i);
      handleSwitch(&pcf8574dev[i], false);
    }
  }
}


void handleSwitch (pcf8574Device* mydev, bool state) { 
  int duration = 0;
  handleSwitch(mydev, state, &duration);
}

void handleSwitch (pcf8574Device* mydev, bool state, int* duration) { //true = AN, false = AUS
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  char subtopic[50];
  memset(&subtopic[0], 0, sizeof(subtopic));
  
  sprintf(buffer, "on-for-timer: Port %d, RequestedState: %s", mydev->port, (state?"AN":"Aus") );
  Serial.println(buffer);
  
  if (mydev->port < 200) {
    mydev->pcf8574->digitalWrite(mydev->pcfport, !state); //schaltet auf LOW
  } else {
    digitalWrite(mydev->pcfport, state);
  }
  
  if (state) {
    if (*duration > 0) {
      mydev->startmillis = millis();  
      mydev->lengthmillis = *duration * 1000;
    }
  } else {
    // schalte aus
    mydev->startmillis = 0;
  }
  
  if(mydev->port != pcf8574dev[syncswitchDevice].port && mydev->port != pcf8574dev[ventil3wegeDevice].port) {
    if (mydev->active != state) {
      if (state) {parallelThreads ++;} else {parallelThreads --;}
    }
    MQTT_publish("Threads", &parallelThreads);
  }
  
  mydev->active = state;
  
  snprintf (subtopic, sizeof(subtopic), "%s/state", mydev->subtopic);
  MQTT_publish(subtopic, state);
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
  } else if (port >=200 && port <=216) {
    // interne GPIO
    t->i2cAddress=0x00;
    t->port=port-200;
  }
}


