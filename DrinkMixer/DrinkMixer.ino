/***********************************************************
 * Arduino Drink Mixer
 * 
 * Author: Espen Hovland, Steinar Valbø
 * 
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




HX711_ADC loadCell{LOADCELL_DATA_PIN, LOADCELL_SCLK_PIN};
LiquidCrystal_I2C lcd{LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS};

static Pump     pumps[NUMBER_OF_PUMPS];
static Recipe   recipes[NUMBER_OF_RECIPES];
File            SDCard;
String          IPAddress;

static int  numRecipes; // Number of actual recipes read from SD card
// static char FILENAME[MAX_FILENAME_LENGTH];
#define FILENAME "recipe.txt"
static char files[NUM_DIRECTORY_FILES][MAX_FILENAME_LENGTH];

int   initSDCard();
void  fetchRecipes(int &nRecipes);
void  listDirectory(File dir, int numTabs);
void  selectRecipeFile();

/**********************************************************************/

void setup()
{
    /* Init shift register pins */
    pinMode(SR_RELAY_CLOCK_PIN, OUTPUT);
    pinMode(SR_RELAY_LATCH_PIN, OUTPUT);
    pinMode(SR_RELAY_DATA_PIN,  OUTPUT);
    // TODO: Add support for shift register number 2(?)
    // TODO: Ensure that relays are not active during startup! (manual switch)
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~0); // Relays are active low
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);

    Serial.begin(9600);
    Serial1.begin(9600); // For communicating with the WiFi module

    Wire.begin(); // I2C comms

    /* Init LCD display */
    lcd.init();
    lcd.backlight();
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
    lcd.display();


    /* Init SD card and collect recipes */
    if (initSDCard() != 0){
        Serial.println("Could not initialize SD card!");
        while(1);
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Reading SD card...");
    lcd.display();

    // TODO: Legg inn støtte for å lese tilgjengelige filer på SD-kort, for så å velge den filen man ønsker. Se: https://www.arduino.cc/en/Tutorial/LibraryExamples/Listfiles
    // ...
    fetchRecipes(numRecipes);
    Serial.print(numRecipes);
    Serial.println(" recipes found in SD card!");
    lcd.setCursor(0, 1);
    lcd.print(numRecipes);
    lcd.print(" recipes found!  ");
    lcd.display();

    /* Init load cell */
    lcd.setCursor(0, 2);
    lcd.print("Calibrating scale...");
    lcd.display();
    // loadCell.begin();
    // loadCell.start(1000);
    // loadCell.setCalFactor(1); // TODO: Set this value properly when calibrating sensor
    // loadCell.tare();
    delay(1000); // Load cell simulator
    lcd.setCursor(0, 2);
    lcd.print("Scale zeroed!       ");
    lcd.display();

    // TODO: Set up comms with the WiFi-module
    lcd.setCursor(0, 3);
    lcd.print("Connecting to WiFi..");
    lcd.display();
    /* Get the IP-address of the module */
    while (true){
        if ( Serial1.available() > 0 && Serial1.read() == COMMS_IP_FLAG ){
            char buf[20];
            byte data[4];
            data[0] = Serial1.read();
            data[1] = Serial1.read();
            data[2] = Serial1.read();
            data[3] = Serial1.read();

            snprintf(buf, 20, "%d.%d.%d.%d", data[0], data[1], data[2], data[3]);
            IPAddress = String(buf);
            Serial1.write(COMMS_IP_ACK); // We have received the IP address
            break;
        }
        delay(50);
    }

    Serial.print("IP: ");
    Serial.println(IPAddress);
    //...
    /* Send the recipes to the WiFi-module */
    Serial1.write(COMMS_RECIPE_FLAG);
    Serial1.write(numRecipes);
    for ( int i = 0; i < numRecipes; i++){
        Serial1.write((byte *)&recipes[i], sizeof(recipes[i])); // How do we receive this?

    }


    delay(1000); // Comms setup simulator
    lcd.setCursor(0, 3);
    lcd.print("WiFi OK!            ");
    lcd.display();

    // for (uint8_t i = 0; i < 255; i++){
    //     digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    //     shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~i); // Relays are active low
    //     digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    //     delay(100);
    // }

    // lcd.clear();
    // for (int i = 0; i < numRecipes; i++){
    //     mixDrink(&recipes[i], pumps, &lcd);
    //     delay(100);
    // }

    /* Finalize */
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Order your drinks at");
    lcd.setCursor(0, 2);
    lcd.print(IPAddress);
    lcd.display();
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
    // delay(200);
    // for (int i = 0; i < NUMBER_OF_PUMPS; i++){
    //     digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    //     shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, pumpToBin(&pumps[i]));
    //     digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    //     delay(1000);
    // }
    // digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    // shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~1);
    // digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    // delay(1000);
    // digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    // shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~0);
    // digitalWrite(SR_RELAY_LATCH_PIN, HIGH);
    // delay(1000);
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
                    String  name;
                    String  ing;
                    int   amount;
                    int   numIngs;

                    if ( buf.charAt(1) == '<'){
                        /* New recipe, get name and number of ingredients */
                        buf     = SDCard.readStringUntil('\r');
                        int idx = buf.indexOf(','); // Get index of delimiter
                        name    = buf.substring(1, idx);
                        numIngs = buf.substring(idx + 1).toInt();

                        recipes[ctr].name            = name;
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
}

// TODO: Make this add all files to a list, for the user to pick the file for recipes.
void listDirectory(File dir, int numTabs) {
    static int index = 0;
    while (true){
        File entry = dir.openNextFile();
        if (!entry){
            // no more files
            break;
        }

        for (uint8_t i = 0; i < numTabs; i++){
            Serial.print('\t');
        }

        Serial.print(entry.name());

        if (entry.isDirectory()){
            Serial.println("/");
            listDirectory(entry, numTabs + 1);
        }
        else{
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);

            // Add the file to the array of available files
            snprintf(files[index], min(sizeof(entry.name()), MAX_FILENAME_LENGTH), "%s", entry.name());
        }
        entry.close();
  }
}

// TODO: List the available files and select the recipe file
void selectRecipeFile(){
    // char file[MAX_FILENAME_LENGTH];


    // // TODO: use the rotary encoder to choose the file


    // // Copy the selected filename to the global variable
    // memcpy(FILENAME, file, MAX_FILENAME_LENGTH);
}

// TODO: Set up interrupt routines for the rotary encoder
