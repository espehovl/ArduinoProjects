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
    byte = 1 << (p->id - 1); // Pump IDs are 1-8

    return ~byte; // Relays are active low
}

void performPump(Pump *p, unsigned int volume, LiquidCrystal_I2C* LCD, Adafruit_NeoPixel *LED,  HX711_ADC* loadCell ){


    unsigned int pourTime = (unsigned long)(1000.0 * volume / ((float)PUMP_RATE)); // [ms]


    // bool dataReady;
    // loadCell->tare();
    // loadCell->update();
    // float baseWeight = loadCell->getData();



    if ( LCD != nullptr ){
        LCD->setCursor(0, 3);
        LCD->print("                    "); // "20 blank spaces"
    }
    int ledsToLight = 0;

    unsigned long T = millis();

    float baseWeight;

    if (loadCell != nullptr)
    {
        if (LED != nullptr){
            for (int i = LED_STRIP_PARTY_LIGHTS; i < LED_STRIP_NUM_LEDS; i++){
                LED->setPixelColor(i, 255, 0, 0);
            }
            LED->show();
        }

        while (!loadCell->update());
        baseWeight = loadCell->getData();

        //loadCell->tare();
    }

    const int pourTimeoutMilliSeconds = pourTime + 10000;
    bool pourComplete = false;

    /* Start pumping */
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, pumpToBin(p));
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    /*
    */
    if (loadCell != nullptr)
    {
        if (LED != nullptr){
            for (int i = LED_STRIP_PARTY_LIGHTS; i < LED_STRIP_NUM_LEDS; i++){
                LED->setPixelColor(i, 255, 0, 120);
            }
            LED->show();
        }
        
        float contentWeightGram = 0.0;
        while (millis() - T < pourTimeoutMilliSeconds){
            while (!loadCell->update());

            contentWeightGram = loadCell->getData() - baseWeight;


            if (contentWeightGram > volume){ // Assumption: 1 g/mL
                pourComplete = true;
                break;
            }
        }
    }
    else
    {
        // Cleaning mode
        //while (millis() - T < pourTime){
           /* if (LCD != nullptr){
                // Print a progress bar
                LCD->print('_');
                LCD->display();
            }*/
            if (LED != nullptr){
                for (int i = LED_STRIP_PARTY_LIGHTS; i < LED_STRIP_NUM_LEDS; i++){
                    LED->setPixelColor(i, 0, 255, 120);
                }
                LED->show();
            }

            delay(pourTime);

 //           ledsToLight += 255 / 20;*/
       // }
    }

    /* Stop pumping */
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~0); // Relays are active low
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);

    if (loadCell != nullptr)
    {
        if (!pourComplete){
            Serial.println("Timed out!");
            LCD->clear();

            LCD->setCursor(0, 0);
            LCD->print("ERROR: Pour failed!");
            LCD->setCursor(0, 2);
            LCD->print("Check bottles and");
            LCD->setCursor(0, 3);
            LCD->print("reboot machine.");
            LCD->display();

            if (LED != nullptr){
                for (int i = 0; i < LED_STRIP_NUM_LEDS; i++){
                    LED->setPixelColor(i, 0);
                }
                
                // Stall the application here
                while (true){
                    for (int i = LED_STRIP_PARTY_LIGHTS + LED_STRIP_NUM_LEDS / 2 - 10; i < LED_STRIP_PARTY_LIGHTS + LED_STRIP_NUM_LEDS / 2 + 10 - LED_STRIP_PARTY_LIGHTS; i++){
                        LED->setPixelColor(i, 255, 255, 0);
                    }
                    LED->show();

                    delay(1000);
                
                    for (int i = LED_STRIP_PARTY_LIGHTS + LED_STRIP_NUM_LEDS / 2 - 10; i < LED_STRIP_PARTY_LIGHTS + LED_STRIP_NUM_LEDS / 2 + 10 - LED_STRIP_PARTY_LIGHTS; i++){
                        LED->setPixelColor(i, 255, 0, 0);
                    }
                    LED->show();
                
                    delay(1000);
                }
            }
        }
    }
    


    if (LED != nullptr){
        for (int i = LED_STRIP_PARTY_LIGHTS; i < LED_STRIP_NUM_LEDS; i++){
            LED->setPixelColor(i, 0, 0, 0);
        }
        LED->show();
    }

    return;
}


void runPump(Pump* p, unsigned long ms){
    
    /* Start pumping */
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, pumpToBin(p));
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);

    delay(ms);

    /* Stop pumping */
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~0); // Relays are active low
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
}

/****************************************************
 * @brief Pour the ingredient from the correct pump
 * @param ing Pointer to Ingredient-object to be poured
 * @param ps  Pointer to array of available Pumps
 * @param LCD Pointer to lcd display object
 ****************************************************/
void pourIngredient(Ingredient *ing, Pump *ps, LiquidCrystal_I2C* LCD, Adafruit_NeoPixel *LED, HX711_ADC *loadCell ){
    unsigned int pumpIdx;
    unsigned long pourTime; // [ms]

    /* Find the pump with the ingredient */
    for (pumpIdx = 0; pumpIdx < NUMBER_OF_PUMPS; pumpIdx++){
        if (strncmp(ps[pumpIdx].drink, ing->beverage, MAX_STRING_LENGTH) == 0){
            break;
        }
    }
    Serial.print("Pouring ");
    Serial.print(ing->beverage);
    Serial.print(" (pump ");
    Serial.print(ps[pumpIdx].id);
    Serial.print(") for ");
    Serial.print(ing->volume / (float)PUMP_RATE);
    Serial.println(" seconds");

    const uint32_t previous_color = LED->getPixelColor(pumpIdx);

    LED->setPixelColor(pump_LED_to_ID[pumpIdx], 255, 255, 255); // Set current bottle to white
    LED->show();
    performPump(&ps[pumpIdx], ing->volume, LCD, LED, loadCell); // Blocking for pourTime milliseconds!
    LED->setPixelColor(pump_LED_to_ID[pumpIdx], previous_color);
    LED->show();
    return;
}

/****************************************************
 * @brief Mix the drink according to the recipe
 * @param rec Pointer to Recipe-object to be mixed
 * @param ps  Pointer to array of available Pumps
 * @param LCD Pointer to lcd display object
 ****************************************************/
void mixDrink(Recipe *rec, Pump *ps, LiquidCrystal_I2C *LCD, Adafruit_NeoPixel *LED, HX711_ADC *loadCell ){
    char buf[LCD_COLS+1];

    Serial.print("\n\rMixing drink: ");
    Serial.println(rec->name);

    /* Store the previous LED status */
    uint32_t previous_led_setting[LED_STRIP_NUM_LEDS];
    for (int i = 0; i < LED_STRIP_NUM_LEDS; i++){
        previous_led_setting[i] = LED->getPixelColor(i < LED_STRIP_BOTTLE_LEDS ? pump_LED_to_ID[i] : i);
    }
    
    for (int i = 0; i < LED_STRIP_BOTTLE_LEDS; i++){
        LED->setPixelColor(i, 0);
    }
    LED->show();

    //LED->clear();
    // for (int i = LED_STRIP_PARTY_LIGHTS; i < LED_STRIP_NUM_LEDS; i++){
    //     LED->setPixelColor(i, 255, 0, 0); // Red light during drink mixing!
    // }
    //LED->show();

    Serial.println("Taring load cell...");
    loadCell->tareNoDelay();

    LCD->clear();
    LCD->setCursor(0, 0);
    snprintf(buf, sizeof(buf), "%s", rec->name);
    LCD->print(buf);

    for (int i = 0; i < rec->num_ingredients; i++){
        LCD->setCursor(0, 1);
        snprintf(buf, sizeof(buf), "Status:          %d/%d", i + 1, rec->num_ingredients);
        LCD->print(buf);
        LCD->setCursor(0, 2);
        snprintf(buf, sizeof(buf), "%-12s%5d mL", rec->ingredients[i].beverage, rec->ingredients[i].volume);
        LCD->print(buf);
        LCD->display();

        pourIngredient(&(rec->ingredients[i]), ps, LCD, LED, loadCell);

        delay(500); // Allow for some time between pumps
    }

    /* Restore the old led lights */
    for (int i = 0; i < LED_STRIP_NUM_LEDS; i++){
        LED->setPixelColor(i < LED_STRIP_BOTTLE_LEDS ? pump_LED_to_ID[i] : i, previous_led_setting[i]);
    }

    for (int i = 0; i < LED_STRIP_BOTTLE_LEDS; i++){
        LED->setPixelColor(i, 255, 255, 255);
    }
    LED->show();


    LCD->clear();
    LCD->setCursor(0, 1);
    LCD->print("Enjoy!");
    LCD->display();
    delay(1000);

    return;
}

/****************************************************************
 * @brief Run the pump to clean the pipes
 * @param ps Pointer to the array containing the pumps
 * @param pumpNo Pump number
 * @param duration The duration (seconds) for each pump to run
 * @param LCD Pointer to lcd display object
 ****************************************************************/
void cleanPump(Pump* p, unsigned int pumpNo, float duration, Adafruit_NeoPixel *LED ){
    /* Store the previous LED status */
    uint32_t previous_led_setting[LED_STRIP_NUM_LEDS];
    for (int i = 0; i < LED_STRIP_NUM_LEDS; i++){
        previous_led_setting[i] = LED->getPixelColor(i < LED_STRIP_BOTTLE_LEDS ? pump_LED_to_ID[i] : i);
    }

    /* Set party lights */
    LED->clear();
    for (int i = LED_STRIP_PARTY_LIGHTS; i < LED_STRIP_NUM_LEDS; i++){
        LED->setPixelColor(i, 0, 255, 0);
    }
    LED->show();

    /*LCD->clear();
    LCD->setCursor(0, 0);
    LCD->print("Cleanup");*/
    LED->setPixelColor(pump_LED_to_ID[pumpNo], 0, 0, 255);
    LED->show();
    //for (int pumpIdx = 0; pumpIdx < NUMBER_OF_PUMPS; pumpIdx++){
    //    LED->setPixelColor(pump_LED_to_ID[pumpIdx], 0, 0, 255); // Current bottle is lit blue
    //    LED->show();

    /*    LCD->setCursor(0, 2);
        LCD->print("Cleaning pump: ");
        LCD->print(p->id);
        LCD->display();*/
    performPump(p, duration * PUMP_RATE, nullptr, LED, nullptr);

    LED->setPixelColor(pump_LED_to_ID[pumpNo], 0);
    //}
    /* Restore the old led lights */
    for (int i = 0; i < LED_STRIP_NUM_LEDS; i++){
        LED->setPixelColor(i < LED_STRIP_BOTTLE_LEDS ? pump_LED_to_ID[i] : i, previous_led_setting[i]);
    }

    LED->show();
    return;
}
