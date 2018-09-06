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
  digitalWrite(pin_hcsr04_trigger, LOW);
}

void hcsr04_loop() {

  // Hold the trigger pin high for at least 10 us
  digitalWrite(pin_hcsr04_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_hcsr04_trigger, LOW);

unsigned long distance;//unsigned, since we don't get negative distances.

distance = round(pulseIn(pin_hcsr04_echo, HIGH, MAX_DIST) /58);//Distance in CM's, use /148 for inches.
  if (distance == 0){//Reached timeout
    Serial.println("Out of range");
  }
  else {
    Serial.print(distance);
    Serial.println(" cm");
  }
}

