Release 2.4.4:  -
  - Feature: Issue #9: MQTT Client ID now configurable
  - MQTT now reconnect after DeviceName has been changed
  - MQTT LastWillTopic as device status configured: state [Offline|Online]

Release 2.4.3:
  - Bugfixing Automatische Releaseverteilung
  - Überarbeitung Github Workflow mit automatischer Releaseerstellung 

Release 2.4.2:
  - Bugfixing des TB6612 Handlings

Release 2.4.1:
  - Added TB6612 Support
  - Added automatic Release Update

Release 2.3:
  - solved some bugfixes
  - MQTT Commands setstate [on|off] now available

Release 2.2:
  - some bugs resolved

Release 2.1:
  Final Release! complete redesign, Its now easier to understand and add more functionality.
  Wiki is now up-to-date based on Release 2.1

  New functionality:
    - i2c Motordriver support for bistable valves
      - ESP8266 motordriverboard for bistable valves
      - Relations now added for complex garden
      - external and analog sensor support
      - changing valve status by Web-UI added

Release 2.0:
  1st Pre Release with completely new refactored code by completely class based.
  Tested with valves at PCF8575 and GPIO, LevelSensor HCSR04 and OLED 1306

Release 1.0:
  this is the finale on first release. Works with optional OLED, optional LevelSensor.
  Supports valves at OnBoard GPIO Pins and PCF8574 Extender i2c-Shield
