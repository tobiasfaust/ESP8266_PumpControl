// https://github.com/sparkfun/HC-SR04_UltrasonicSensor/blob/master/Firmware/HC-SR04_UltrasonicSensorExample/HC-SR04_UltrasonicSensorExample.ino
// https://arduino.stackexchange.com/questions/19767/what-if-ultrasonic-sensor-doesnt-detect-object

/**
 * HC-SR04 Demo
 * Demonstration of the HC-SR04 Ultrasonic Sensor
 * Date: August 3, 2016
 * 
 * Description:
 *  Connect the ultrasonic sensor to the Arduino as per the
 *  hardware connections below. Run the sketch and open a serial
 *  monitor. The distance read from the sensor will be displayed
 *  in centimeters and inches.
 * 
 * Hardware Connections:
 *  Arduino | HC-SR04 
 *  -------------------
 *    5V    |   VCC     
 *    7     |   Trig     
 *    8     |   Echo     
 *    GND   |   GND
 *  
 * License:
 *  Public Domain
 */


// Anything over 400 cm (400*58 = 23200 us pulse) is "out of range"
const unsigned int MAX_DIST = 23200;

void hcsr04_setup() {
  Serial.print("Starting HS-SR04 - Trigger/EchoPin: ");Serial.print(pin_hcsr04_trigger);Serial.print("/");Serial.println(pin_hcsr04_echo);
  // The Trigger pin will tell the sensor to range find
  pinMode(pin_hcsr04_trigger, OUTPUT);
  //digitalWrite(pin_hcsr04_trigger, LOW);
  pinMode(pin_hcsr04_echo, INPUT);
  //digitalWrite(pin_hcsr04_echo, LOW);
}

void analog_setup() {
  
}

void hcsr04_loop() {
  digitalWrite(pin_hcsr04_trigger, LOW);
  delayMicroseconds(2);
  
  // Hold the trigger pin high for at least 10 us
  digitalWrite(pin_hcsr04_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_hcsr04_trigger, LOW);

  sensor_level = 0;
  
  sensor_RawValue = pulseIn(pin_hcsr04_echo, HIGH, MAX_DIST); //Distance in CM's, use /148 for inches.
  sensor_RawValue = (sensor_RawValue / 2) / 29.1;

  if (sensor_RawValue == 0){//Reached timeout
    Serial.println("Out of range");
  }
  else {
    //Serial.print(distance);Serial.println(" cm");
    MQTT_publish(MQTT_sensorRawValue, &sensor_RawValue);
    
    if (measureDistMax - measureDistMin > 0) {
      sensor_level = (((measureDistMax - sensor_RawValue)*100)/(measureDistMax - measureDistMin));

      // MQTT 
      MQTT_publish(MQTT_level, &sensor_level);
    
      // Automatik
      auto_3wegeVentil();
    }
  }
}

void analog_loop() {
  
  sensor_RawValue = analogRead(A0);
  
  MQTT_publish(MQTT_sensorRawValue, &sensor_RawValue); //raw-Wert
  
  sensor_level = map(sensor_RawValue, measureDistMin, measureDistMax, 0, 100); // 0-100%
  Serial.print(sensor_level);Serial.println(" %");
  MQTT_publish(MQTT_level, &sensor_level);

  //Automatik
  auto_3wegeVentil();
}

void auto_3wegeVentil() {
  if(enable_3wege && sensor_level <= treshold_min && !pcf8574dev[ventil3wegeDevice].active) {handleSwitch(&pcf8574dev[ventil3wegeDevice], true);}
  if(enable_3wege && sensor_level >= treshold_max &&  pcf8574dev[ventil3wegeDevice].active) {handleSwitch(&pcf8574dev[ventil3wegeDevice], false);}
}

