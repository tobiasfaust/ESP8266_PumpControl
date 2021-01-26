Release 2.4.5:
  - Feature: deletion of WiFi credentials now possible
  - Feature: ESP Hostname now the configured Devicename
  - Bug: WIFI Mode forces to STATION-Mode, some devices has been ran in unsecured STA+AP Mode
  - Bug: security issue: dont show debug output of WiFi Connection (password has been shown)
  - Feature: valve reverse mode: enable if your valve act on LOW instead of ON
  - Feature: AutoOff: possibility to setup a security AutoOff 
  - Bug: count of Threads now push out if an on-for-timer has been expired
  
Release 2.4.4:
  - Feature: Issue #9: MQTT Client ID now configurable
  - MQTT now reconnect after DeviceName has been changed
  - MQTT LastWillTopic as device status configured by topic "/state [Offline|Online]"
  - Publish Release and Version after MQTT Connect by topic "/version"
  - Bugfix: Nullpointer to Hardwaredevice if multiple hardware devices are defined

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
