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

void performPump(Pump *p, unsigned long ms, LiquidCrystal_I2C* LCD){

    /* Start pumping */
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, pumpToBin(p));
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);

    unsigned long T = millis();

    LCD->setCursor(0, 3);
    LCD->print("                    "); // "20 blank spaces"
    LCD->setCursor(0, 3);
    while( millis() - T < ms){
        // Print a progress bar
        LCD->print('_');
        LCD->display();
        delay(ms / 20);
    }

    /* Stop pumping */
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~0); // Relays are active low
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    return;
}

/****************************************************
 * @brief Pour the ingredient from the correct pump
 * @param ing Pointer to Ingredient-object to be poured
 * @param ps  Pointer to array of available Pumps
 ****************************************************/
void pourIngredient(Ingredient *ing, Pump *ps, LiquidCrystal_I2C* LCD){
    unsigned int pumpIdx;
    unsigned long pourTime; // [ms]

    /* Find the pump with the ingredient */
    for (pumpIdx = 0; pumpIdx < NUMBER_OF_PUMPS; pumpIdx++){
        if (ps[pumpIdx].drink == ing->beverage){
            break;
        }
    }

    // TODO: If amount is 0, fill cup until 350 ml (grams) 
    // Register weight of contents so far, calculate the amount needed for 350 ml total drink, set that as the required volume

    pourTime = (unsigned long)(1000.0 * ing->volume / ((float)PUMP_RATE)); // [ms]

    Serial.print("Pouring ");
    Serial.print(ing->beverage);
    Serial.print(" (pump ");
    Serial.print(ps[pumpIdx].id);
    Serial.print(") for ");
    Serial.print(pourTime/1000.0);
    Serial.println(" seconds");

    performPump(&ps[pumpIdx], pourTime, LCD); // Blocking for pourTime milliseconds!

    return;
}

/****************************************************
 * @brief Mix the drink according to the recipe
 * @param rec Pointer to Recipe-object to be mixed
 * @param ps  Pointer to array of available Pumps
 ****************************************************/
void mixDrink(Recipe *rec, Pump *ps, LiquidCrystal_I2C *LCD){
    char buf[LCD_COLS+1];

    Serial.print("\n\rMixing drink: ");
    Serial.println(rec->name);

    LCD->clear();
    LCD->setCursor(0, 0);
    snprintf(buf, sizeof(buf), "%s", rec->name.c_str());
    LCD->print(buf);

    for (int i = 0; i < rec->num_ingredients; i++){
        LCD->setCursor(0, 1);
        snprintf(buf, sizeof(buf), "Status:          %d/%d", i + 1, rec->num_ingredients);
        LCD->print(buf);
        LCD->setCursor(0, 2);
        snprintf(buf, sizeof(buf), "%-12s%5d mL", rec->ingredients[i].beverage.c_str(), rec->ingredients[i].volume);
        LCD->print(buf);
        LCD->display();

        pourIngredient(&(rec->ingredients[i]), ps, LCD);

        delay(1000); // Is this useful/needed?
    }

    return;
}