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
#include "PumpControl.h"

pcf8574Device* pcf8574dev;
valveRelation* valveRel;
uint8_t pcf8574devCount = 0;
uint8_t valveRelCount = 0;
const uint8_t pcf8574HWCount = 4;
pcf8574HW* pcf8574 = new pcf8574HW[pcf8574HWCount];

void PCF8574_setup() {
  // erwarteter json
  // {"count":16,"mqtttopic_0":"Ventil1","pcfport_0":202,"active_0":1,"mqtttopic_1":"Ventil2","pcfport_1":214,"active_1":1,"mqtttopic_2":"Ventil3","pcfport_2":215,"active_2":1,"mqtttopic_3":"Ventil4","pcfport_3":216,"active_3":0,"mqtttopic_4":"Ventil5","pcfport_4":216,"active_4":0,"mqtttopic_5":"Frischwasserventil","pcfport_5":216,"active_5":0,"mqtttopic_6":"Ventil7","pcfport_6":216,"active_6":0,"mqtttopic_7":"Ventil8","pcfport_7":216,"active_7":0,"mqtttopic_8":"Ventil9","pcfport_8":216,"active_8":0,"mqtttopic_9":"Ventil10","pcfport_9":216,"active_9":0,"mqtttopic_10":"Ventil11","pcfport_10":216,"active_10":0,"mqtttopic_11":"Ventil12","pcfport_11":216,"active_11":0,"mqtttopic_12":"Ventil13","pcfport_12":216,"active_12":0,"mqtttopic_13":"Trinkwasser","pcfport_13":216,"active_13":0,"mqtttopic_14":"3WegeVentil","pcfport_14":205,"active_14":1,"mqtttopic_15":"Ventil16","pcfport_15":216,"active_15":0}
  // {"count":1,"mqtttopic_0":"Ventil1","pcfport_0":202,"active_0":1}
  boolean loadDefaultConfig = false;  
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  
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

          pcf8574devCount = json["count"] | 0;
          uint8_t my_port = 0;
          pcf8574Port my_pcf8574Port;
          boolean i2caddress_found = false;
          uint8_t count = 0;

          // some plausibliy checks
          if(pcf8574devCount==0) {
            Serial.println("something went wrong with json config, load default config");
            pcf8574devCount = 0;
            loadDefaultConfig = true;
          }

          pcf8574dev = new pcf8574Device[pcf8574devCount];
          
          for (unsigned int i=0; i < pcf8574devCount; i++) {
            i2caddress_found = false;
            
            sprintf(buffer, "type_%d", i);
            if (json.containsKey(buffer)) {strlcpy(pcf8574dev[i].type, json[buffer]|"v", 2);}
            else {
              sprintf(buffer, "Die Typedefinition der Ventildefinition '%d' wurde nicht gefunden, lade DefaultConfig", i);
              Serial.println(buffer);
              //loadDefaultConfig = true;
              //break;  
            }
            
            sprintf(buffer, "active_%d", i);
            if (json[buffer] && json[buffer] == 1) {pcf8574dev[i].enabled = true;} else {pcf8574dev[i].enabled = false;}
            
            sprintf(buffer, "mqtttopic_%d", i);
            if (json.containsKey(buffer)) {strlcpy(pcf8574dev[i].subtopic, json[buffer]|"notFound", 20);}

            pcf8574dev[i].startmillis = 0;
            
            sprintf(buffer, "pcfport_%d_0", i);
            if (json.containsKey(buffer) && json[buffer].as<int>() > 0) {
              my_port = json[buffer].as<int>();
              GetPCF8574Port(&my_pcf8574Port, my_port);
              pcf8574dev[i].port = my_port;
              pcf8574dev[i].pcfport = my_pcf8574Port.port;
            }
            
            sprintf(buffer, "pcfport_%d_1", i);
            if (json.containsKey(buffer) && json[buffer].as<int>() > 0) {
              my_port = json[buffer].as<int>();
              GetPCF8574Port(&my_pcf8574Port, my_port);
              pcf8574dev[i].port2 = my_port;
              pcf8574dev[i].pcfport2 = my_pcf8574Port.port;
            }

            sprintf(buffer, "imp_%d_0", i); //impulsbreite für Port 1
            if (json.containsKey(buffer) && json[buffer].as<int>() > 0 && json[buffer].as<int>() < 1000) {
              pcf8574dev[i].portms = json[buffer].as<int>();
            }
            sprintf(buffer, "imp_%d_1", i); //impulsbreite für Port 2
            if (json.containsKey(buffer) && json[buffer].as<int>() > 0 && json[buffer].as<int>() < 1000) {
              pcf8574dev[i].port2ms = json[buffer].as<int>();
            }

            Serial.print("konvertiere Port "); Serial.print(pcf8574dev[i].port);Serial.print(" zu I2C Adresse ");Serial.println(my_pcf8574Port.i2cAddress);
            
            if (pcf8574dev[i].port && pcf8574dev[i].port < 200 && pcf8574dev[i].port > 0) {
              // nur für PCF Ports, alle Ports > 200 sind interne GPIOs
              for (unsigned int j=0; j < pcf8574HWCount; j++) {
                //falls ein vorheriger Port bereits das i2c Device angefordert hat, nutze es!
                if(pcf8574[j].i2cAddress && pcf8574[j].i2cAddress == my_pcf8574Port.i2cAddress) {
                  i2caddress_found = true;
                  pcf8574dev[i].pcf8574 = pcf8574[j].pcf8574;
                }
              }
              if(!i2caddress_found) {
                //der angegebene Port benutzt ein i2c Device welches vorher noch nicht initialisiert wurde
                //check ob Device auch durch i2cdetect gefunden wurde
                if (I2Cdetect->i2cIsPresent(my_pcf8574Port.i2cAddress)) { i2caddress_found = true; }
                if (i2caddress_found) {
                  pcf8574[count].i2cAddress = my_pcf8574Port.i2cAddress;
                  pcf8574[count].pcf8574 = new PCF8574(pcf8574[count].i2cAddress, pin_sda, pin_scl);
                  pcf8574[count].pcf8574->begin();
                  pcf8574dev[i].pcf8574 = pcf8574[count].pcf8574;
                  Serial.print("PCF8574 gefunden und initialisiert auf Adresse: "); Serial.println(my_pcf8574Port.i2cAddress);
                  count++;
                } else {
                  sprintf(buffer, "Die in der Ventildefinition angegebene i2cAdresse (Port: %d , i2c: %02x) wurde nicht gefunden.", my_pcf8574Port.port, my_pcf8574Port.i2cAddress);
                  Serial.print(buffer);
                }
              }
              pcf8574dev[i].pcf8574->pinMode(pcf8574dev[i].pcfport, OUTPUT);
              pcf8574dev[i].pcf8574->digitalWrite(pcf8574dev[i].pcfport, HIGH);
            } else if (pcf8574dev[i].port && pcf8574dev[i].port > 200 && pcf8574dev[i].port < 999) {
              // interne GPIO´s
              if(pcf8574dev[i].pcfport && pcf8574dev[i].pcfport > 0) {
                pinMode(pcf8574dev[i].pcfport, OUTPUT);
                digitalWrite(pcf8574dev[i].pcfport, LOW);
              }

              //beachte Port2 nur für interne Ports
              if(pcf8574dev[i].pcfport2 && pcf8574dev[i].pcfport2 > 0) {
                pinMode(pcf8574dev[i].pcfport2, OUTPUT);
                digitalWrite(pcf8574dev[i].pcfport2, LOW);
              }
            }
          }

          // lade relationen
          loadValveRelations();
          
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
      Serial.println("load DefaultConfig");
      pcf8574devCount = 2;
      pcf8574dev = new pcf8574Device[pcf8574devCount];
      for (unsigned int i=0; i < pcf8574devCount; i++) { 
        pcf8574dev[i].port = 212+i;
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
}
// wird aus dem MQTT Callback aufgerufen

void loadValveRelations() {
    char buffer[100] = {0};
    memset(buffer, 0, sizeof(buffer));
    boolean loadDefaultConfig = false;  
    
    if (SPIFFS.exists("/Ralations.json")) {
      //file exists, reading and loading
      Serial.println("reading Relation config file");
      File configFile = SPIFFS.open("/Relations.json", "r");
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

          valveRelCount = json["count"] | 0;
          valveRel = new valveRelation[valveRelCount];
          for (unsigned int i=0; i < valveRelCount; i++) {
            sprintf(buffer, "active_%d", i);
            if (json[buffer] && json[buffer] == 1) {valveRel[i].enabled = true;} else {valveRel[i].enabled = false;}
            
            for(int j=0; j < pcf8574devCount; j++) {
              sprintf(buffer, "portA_%d_1", i);
              if (json.containsKey(buffer) && strcmp(pcf8574dev[j].subtopic, json[buffer])==0) {
                valveRel[i].portA = &pcf8574dev[j];
              }
              sprintf(buffer, "portB_%d_1", i);
              if (json.containsKey(buffer) && strcmp(pcf8574dev[j].subtopic, json[buffer])==0) {
                valveRel[i].portB = &pcf8574dev[j];
              }
            }
          }
        } else {
          Serial.println("failed to load json config, load default config");
          loadDefaultConfig = true;
        }
      }
    } else {
      Serial.println("RelationConfig.json config File not exists, load default config");
      loadDefaultConfig = true;
    }
        
    if (loadDefaultConfig) {
      // do something
      Serial.println("load DefaultConfig");
      valveRelCount = 1;
      valveRel = new valveRelation[valveRelCount];
      valveRel[0].enabled = false;
      valveRel[0].portA = &pcf8574dev[0];
      valveRel[0].portB = &pcf8574dev[0];

      loadDefaultConfig = false; //set back
    }  
}

void PCF8574_onfortimer(int* duration, pcf8574Device* mydev) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  if (max_parallel == 0 || parallelThreads < max_parallel ) {
    // nur schalten wenn kein virtueller Port und Port ist aktiviert und MaxParrallelThreads noch nicht erreicht 
    if(strcmp(mydev->type, "v")!=0) {
      handleSwitch(mydev, true, duration);
    } 
    
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

  // abhängige Ports müssen abgearbeitet werden
  for (unsigned int i=0; i < valveRelCount; i++) {
    if (strcmp(valveRel[i].portA->subtopic, mydev->subtopic)==0) {
      PCF8574_onfortimer(duration, valveRel[i].portB);
    }
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
  
  sprintf(buffer, "on-for-timer: Dauer %d, Port %d, RequestedState: %s", *duration, mydev->port, (state?"AN":"Aus") );
  Serial.println(buffer);
  
  if (mydev->port < 200 && mydev->pcf8574) {
    mydev->pcf8574->digitalWrite(mydev->pcfport, !state); //schaltet auf LOW
  } else {
    if (strcmp(mydev->type, "b")==0) {
      if (state) {
        // AN schalten
        digitalWrite(mydev->pcfport,  HIGH);
        digitalWrite(mydev->pcfport2, LOW);
        delay(mydev->portms);
        digitalWrite(mydev->pcfport,  LOW);
        digitalWrite(mydev->pcfport2, LOW);
      } else {
        // AUS schalten
        digitalWrite(mydev->pcfport,  LOW);
        digitalWrite(mydev->pcfport2, HIGH);
        delay(mydev->port2ms);
        digitalWrite(mydev->pcfport,  LOW);
        digitalWrite(mydev->pcfport2, LOW);
      }
    } else if (strcmp(mydev->type, "n")==0){
      digitalWrite(mydev->pcfport, state);
    }
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


