/***********************************************************
 * Arduino Drink Mixer
 * 
 * Author: Espen Hovland, Steinar Valbø
 * 
 * Storage issues!!! <- will be solved when Mega arrives
 * 
 ***********************************************************/
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711_ADC.h>

#include "DrinkMixer.h"
#include "Recipes.h"
#include "pumps.h"



// HX711_ADC loadCell{LOADCELL_DATA_PIN, LOADCELL_SCLK_PIN};
// LiquidCrystal_I2C lcd{LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS};


static Pump     pumps[NUMBER_OF_PUMPS];
static Recipe   recipes[NUMBER_OF_RECIPES];
File     SDCard;

static int  numRecipes;


int  initSDCard();
void fetchRecipes(int &nRecipes);

/**********************************************************************/

void setup()
{
    Serial.begin(9600);
    //Wire.begin();

    /* Init SD card and collect recipes */
    if (initSDCard() != 0){
        Serial.println("Could not initialize SD card!");
        while(1);
    }
    fetchRecipes(numRecipes);
    Serial.print(numRecipes);
    Serial.println(" recipes found in SD card!");

    /* Init LCD display */
    // lcd.init();
    // lcd.backlight();
    // lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print("Please weight...");
    // lcd.display();

    /* Init load cell */
    // loadCell.begin();
    // loadCell.start(1000);
    // loadCell.setCalFactor(1); // TODO: Set this value properly when calibrating sensor
    // loadCell.tare();

    /* Init shift register pins */
    pinMode(SR_RELAY_CLOCK_PIN, OUTPUT);
    pinMode(SR_RELAY_LATCH_PIN, OUTPUT);
    pinMode(SR_RELAY_DATA_PIN,  OUTPUT);
    // TODO: Add support for shift register number 2

    // for (uint8_t i = 0; i < 255; i++){
    //     digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    //     shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~i); // Relays are active low
    //     digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    //     delay(100);
    // }

    // lcd.clear();

    mixDrink(&recipes[0], pumps);
}

void loop()
{
    // TODO: Write smart logic for the main operation of the drink mixer here.
    // loadCell.update();
    // float val = loadCell.getData();
    // lcd.clear();
    // lcd.setCursor(0, 1);
    // lcd.print(val);
    // lcd.display();
    // for (int i = 0; i < NUMBER_OF_PUMPS; i++){
    //     digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    //     shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, pumpToBin(&pumps[i]));
    //     digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    //     delay(1000);
    // }
}


/***********************************************************************************/

/**************************************************
 * @brief  Initialize the SD card
 * @return 0 if OK, otherwise if an error occurred
 **************************************************/
int initSDCard(){
    Serial.print("Initializing SD card...");
    if (!SD.begin(SPI_CS_PIN)){
        Serial.println("failed!");
        return 1;
    }
    Serial.println("done!");

    Serial.print("Finding file...");
    if (!SD.exists(FILENAME)){
        Serial.println("failed!");
        return 2;
    }
    Serial.println("OK!");

    return 0;
}


/**************************************************************
 * @brief  Fetch drink recipes from SD card
 * @param  nRecipes Variable to hold number of recipes found
 *************************************************************/
void fetchRecipes( int &nRecipes ){
    // Serial.print("Initializing SD card...");
    // if (!SD.begin(SPI_CS_PIN)){
    //     Serial.println("failed!");
    //     while(1);
    // }
    // Serial.println("done!");

    // Serial.print("Finding file...");
    // if (!SD.exists(FILENAME)){
    //     Serial.println("failed!");
    //     while(1);
    // }
    // Serial.println("OK!");

    SDCard = SD.open(FILENAME);
    if (SDCard){
        bool readingRecipes = false;

        int ctr = 0;

        while(SDCard.available()){
            String buf = SDCard.readStringUntil('\r'); // Get one line of file
            // Index 0 is '\r', so check 1 instead
            if (buf.charAt(1) == '#'){
                // Comment only, ignore this line
                continue;
            }
            else if (buf.charAt(1) == '='){
                /* Get the number of recipes */
                nRecipes = buf.substring(2).toInt();
                // Serial.println(nRecipes);
                continue;
            }
            else {
                if (!readingRecipes && buf.charAt(1) == '<'){ 
                    /* From now on we are reading recipes */
                    readingRecipes = true;
                    Serial.println("Pump beverages loaded.");
                }
                if (!readingRecipes){
                    /* We are reading pump id and contents */
                    int id = buf.toInt(); // Cheap way of getting the id of pump
                    if (id > 0 && id <= NUMBER_OF_PUMPS){
                        String b = buf.substring(2);
                        b.trim();
                        pumps[id - 1].id = id;
                        pumps[id - 1].drink = b.c_str();
                    }
                }
                else {
                    /* We are reading recipes */
                    //TODO: Read and store recipes in a useful way
                    String  name;
                    String  ing;
                    int   amount;
                    int   numIngs;

                    if ( buf.charAt(1) == '<'){
                        /* New recipe, get name and number of ingredients */
                        buf = SDCard.readStringUntil('\r');
                        int idx = buf.indexOf(','); // Get index of delimiter
                        name = buf.substring(1, idx);
                        numIngs = buf.substring(idx + 1).toInt();

                        recipes[ctr].name = name;
                        recipes[ctr].num_ingredients = numIngs;

                        for (int i = 0; i < min(numIngs, MAX_INGREDIENTS); i++){
                            /* Get an ingredient */
                            buf = SDCard.readStringUntil('\r');
                            idx = buf.indexOf(',');
                            recipes[ctr].ingredients[i].beverage = buf.substring(1, idx);
                            recipes[ctr].ingredients[i].volume   = buf.substring(idx + 1).toInt();
                        }

                        ctr++;
                    }

                }
            }
        }
        SDCard.close();
        Serial.println("Closed file.");
    }
    else {
        /* No recipes found */
        Serial.println("Could not open SD card.");
        nRecipes = 0;
        return;
    }

    /* Print for debugging purposes */
    // for (int i = 0; i < NUMBER_OF_PUMPS; i++){
    //     Serial.print(pumps[i].id);
    //     Serial.print(" ");
    //     Serial.println(pumps[i].drink);
    // }

    // for (int i = 0; i < nRecipes; i++)
    // {
    //     Serial.print(recipes[i].name);
    //     Serial.print(" ");
    //     Serial.println(recipes[i].num_ingredients);
    //     for (int j = 0; j < recipes[i].num_ingredients; j++)
    //     {
    //         Serial.print(recipes[i].ingredients[j].beverage);
    //         Serial.print(" ");
    //         Serial.println(recipes[i].ingredients[j].volume);
    //     }
    //     Serial.println();
    // }

    //return nRecipes;
}

