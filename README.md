# ESP32-C3\_Sensor

Sensor Board to read two PT100 sensors and a Barometer. Show the measurements on a OLED Display and Broadcast over MQTT.  
With li-ion charger over USB-C.



## Hardware Components

* Two channel PT100
* Li-Ion Charging and protection circuit
* Blue user LED
* User button
* 0.9" OLED
* Li-Ion Battery connector
* Barometer

## Software Modules

* Two channel PT100 measurement with calibration.
* Access point to calibrate and configure the device and to get device information like battery voltage and temperatures.
* Measure battery voltage. (if connected)
* Display temperatures and voltage on OLED.
* Barometer
* MQTT
* Deepsleep

Device:
* Use it as stand-alone device to measure up to two temperatures and show on display. No Wifi needed.
* Use it as stand-alone device to measure up to two temperatures and check over access point.
* Use it as headless device to measure up to two temperatures and broadcast over MQTT.
* Combine the modes.
* Use it as a battery device.
* Use it without battery.
* Use it as you want. Write your own code. Use existing code. Modify existing code.
* Use. Modify. Write. Code.

# Info
All open source!

Makes use of mongoose.wz for HTML UI.