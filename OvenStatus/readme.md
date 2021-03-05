# Oven Status Indicator and Logger
Keywords:
  Temperature sensor, Thingspeak, LCD display, WiFi

A DS18B20 temperature sensor is attached to my oven, allowing for logging when and for how long the oven has been on during e.g. a day. Additionally, a DHT11 is used to monitor the room temperature as well as the relative humidity in the room. The data is collected and uploaded to my Thingspeak account, where the data is logged continuously.

Furthermore, a 1602 LCD display continuously displays the oven temperature, room temperature and relative humidity, for easy reading in real time.


## Schematic
Note that the temperature sensor (DS18B20) has been connected to an extension wire; the breadboard is not stuck to the oven!

![Schematic](OvenStatus_bb.png?raw=true "Schematic")


