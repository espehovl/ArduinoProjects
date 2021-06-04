#ifndef _PUMPS_H_
#define _PUMPS_H_

#include "Recipes.h"

#include <LiquidCrystal_I2C.h>

#define PUMP_RATE (  30 ) // Pump rate, [mL/s] TODO: This need calibration when pumps arrive!

uint8_t pumpToBin(Pump *p);
void    performPump(Pump *p, unsigned long ms, LiquidCrystal_I2C* LCD);
void    pourIngredient(Ingredient *ing, Pump *ps, LiquidCrystal_I2C* LCD);
void    mixDrink(Recipe *rec, Pump *ps, LiquidCrystal_I2C* LCD);

#endif
