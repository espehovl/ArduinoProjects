# MonitorAccentLight

_This will be updated later on with more info, pictures, schematics, etc._

Application for monitor ambience lighting. Consists of a **Wemos D1 Mini** connected to 10 LED long **NeoPixel** strip. In addition, a generic breakout **light sensor** is added in order to be able to dim the lights down when the room is dark.

### Operation
The program provides two modes of operation:

- Mode 0: A light sensor will turn on/off the strip according to the ambient brightness. 
- Mode 1: The brightness of the LED strip will automatically dim down to a lower brightness at the specified time. In the morning, the light will increase the brightness to the value it was before the "night mode" was enabled.

In both modes, the strip color and brightness can always be controlled via the Blynk app. See the screenshot below for how I set up my canvas.


## Circuit diagram according to Arduino sketch

![Schematic](Schematic.png?raw=true "Schematic")

![BlynkCanvas](BlynkCanvas.PNG?raw=true "Blynk Canvas")
