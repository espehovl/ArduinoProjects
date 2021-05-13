/***********************************************************
 * Arduino Drink Mixer
 * 
 ***********************************************************/

#include "DrinkMixer.h"

#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "HX711_ADC.h"

HX711_ADC loadCell{LOADCELL_DATA_PIN, LOADCELL_SCLK_PIN};
LiquidCrystal_I2C lcd{LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS};

void setup()
{
    Serial.begin(9600);

    Wire.begin();

    // lcd.init();
    // lcd.backlight();
    // lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print("Please weight...");
    // lcd.display();

    // loadCell.begin();
    // loadCell.start(1000);
    // loadCell.setCalFactor(1); // TODO: Set this value properly
    // loadCell.tare();

    pinMode(SR_RELAY_CLOCK_PIN, OUTPUT);
    pinMode(SR_RELAY_LATCH_PIN, OUTPUT);
    pinMode(SR_RELAY_DATA_PIN,  OUTPUT);

    for (uint8_t i = 0; i <= 255; i++){
        digitalWrite(SR_RELAY_LATCH_PIN, LOW);
        shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~i); // Relays are active low
        digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
        delay(50);
    }

    // lcd.clear();
}

void loop()
{
    // loadCell.update();
    // float val = loadCell.getData();
    // lcd.clear();
    // lcd.setCursor(0, 1);
    // lcd.print(val);
    // lcd.display();
    // delay(100);
}
