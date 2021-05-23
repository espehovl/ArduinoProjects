#include "pumps.h"
#include "Recipes.h"
#include "DrinkMixer.h"

#include <Arduino.h>

uint8_t pumpToBin(Pump *p){
    uint8_t byte;
    /*  Relay   -> shift register output
        1       ->  A
        2       ->  B
                ...
        8       ->  H
    */
    byte = 1 << (p->id-1); // Pump IDs are 1-8

    return ~byte; // Relays are active low
}

void performPump(Pump *p, int ms){
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, pumpToBin(p));
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    delay(ms);
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, 0b11111111); // Relays are active low
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    return;
}

/****************************************************
 * @brief Pour the ingredient from the correct pump
 * @param ing pointer to Ingredient-object to be poured
 * @param ps  Pointer to array of available Pumps
 ****************************************************/
void pourIngredient(Ingredient *ing, Pump *ps){
    unsigned int pumpIdx;
    unsigned int pourTime; // [ms]

    /* Find the pump with the ingredient */
    for (pumpIdx = 0; pumpIdx < NUMBER_OF_PUMPS; pumpIdx++){
        if (ps[pumpIdx].drink == ing->beverage){
            //pumpIdx = i;
            break;
        }
    }


    pourTime = (unsigned int)(1000.0 * ing->volume / ((float)PUMP_RATE)); // [ms]

    Serial.println();
    Serial.print("Pouring ");
    Serial.print(ing->beverage);
    Serial.print(" (pump ");
    Serial.print(ps[pumpIdx].id);
    Serial.print(") for ");
    Serial.print(pourTime/1000.0);
    Serial.println(" seconds");

    performPump(&ps[pumpIdx], pourTime); // Blocking for pourTime milliseconds!

    return;
}

/****************************************************
 * @brief Mix the drink according to the recipe
 * @param rec Recipe-object to be mixed
 * @param ps  Pointer to array of available Pumps
 ****************************************************/
void mixDrink(Recipe *rec, Pump *ps){
    Serial.print("Mixing drink: ");
    Serial.println(rec->name);

    for (int i = 0; i < rec->num_ingredients; i++){
        pourIngredient(&(rec->ingredients[i]), ps);
        delay(100); // Is this useful?
    }

    return;
}