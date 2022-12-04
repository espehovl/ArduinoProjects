#ifndef _PUMPS_H_
#define _PUMPS_H_

#include "Recipes.h"

#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include <HX711_ADC.h>

// Rate when pump voltage is 12 V : 25
// Rate when pump voltage is  7 V : 15
#define PUMP_RATE (  15 ) // Pump rate, [mL/s]

uint8_t pumpToBin       ( Pump *p );
void    performPump     ( Pump *p,           unsigned int volume,   LiquidCrystal_I2C *LCD, Adafruit_NeoPixel *LED, HX711_ADC *loadCell );
void    pourIngredient  ( Ingredient *ing,   Pump *ps,              LiquidCrystal_I2C *LCD, Adafruit_NeoPixel *LED, HX711_ADC *loadCell );
void    mixDrink        ( Recipe *rec,       Pump *ps,              LiquidCrystal_I2C *LCD, Adafruit_NeoPixel *LED, HX711_ADC *loadCell );
void    cleanPump       ( Pump *p, unsigned int pumpNo, float duration, Adafruit_NeoPixel *LED);
void    runPump         ( Pump *p,           unsigned long ms );

#endif
