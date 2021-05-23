#ifndef _PUMPS_H_
#define _PUMPS_H_

#include "Recipes.h"

#define PUMP_RATE (  10 ) // Pump rate, [mL/s] TODO: This need calibration when pumps arrive!

uint8_t pumpToBin(Pump *p);
void    performPump(Pump *p, int ms);
void    pourIngredient(Ingredient *ing, Pump *ps);
void    mixDrink(Recipe *rec, Pump *ps);

#endif
