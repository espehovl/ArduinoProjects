/***********************************************************
 * Arduino Drink Mixer Application
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
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#include "DrinkMixer.h"
#include "Recipes.h"
#include "pumps.h"




HX711_ADC         loadCell {LOADCELL_DATA_PIN, LOADCELL_SCLK_PIN};
LiquidCrystal_I2C lcd      {LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS};
Adafruit_NeoPixel ledStrip {LED_STRIP_NUM_LEDS, LED_STRIP_DATA_PIN, NEO_GRB + NEO_KHZ800};

DynamicJsonDocument jsonObject(2048);

Pump                pumps[NUMBER_OF_PUMPS];
Recipe              recipes[NUMBER_OF_RECIPES];
File                SDCard;
String              IPAddress;

volatile bool       isButtonPressed           = false;
volatile bool       isWebOrderAvailable       = false;
volatile bool       updateFlag                = false;
volatile bool       isCleaningRequested       = false;
volatile int8_t     rotaryEncoderCounter      = 0;
volatile uint8_t    rotaryEncoderCurrentState = 0;
volatile uint8_t    rotaryEncoderLastState    = 0;

static uint8_t  numRecipes; // Number of actual recipes read from SD card
#define FILENAME "recipes.txt"
#define WIFI_MODULE Serial2

String  getIPAddress();
int     initSDCard();
void    fetchRecipes();
void    placeWebOrder();
Recipe* getRecipeFromName(char *drinkName, int lenDrinkName);

/* Interrupt Service Routines */
void buttonPressedISR();
void incrementCounterISR();
void orderNotifyISR();
void cleanNotifyISR();

/**********************************************************************/

void setup()
{
    /* Init shift register pins */
    pinMode(SR_RELAY_CLOCK_PIN, OUTPUT);
    pinMode(SR_RELAY_LATCH_PIN, OUTPUT);
    pinMode(SR_RELAY_DATA_PIN,  OUTPUT);

    /* Ensure that all relays are inactive upon boot */
    digitalWrite(SR_RELAY_LATCH_PIN, LOW);
    shiftOut(SR_RELAY_DATA_PIN, SR_RELAY_CLOCK_PIN, MSBFIRST, ~0); // Relays are active low
    digitalWrite(SR_RELAY_LATCH_PIN, HIGH);

    Serial.begin(9600);
    WIFI_MODULE.begin(9600); // For communicating with the WiFi module

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
        while (true);
    }

    lcd.setCursor(0, 1);
    lcd.print("Reading SD card...");
    lcd.display();

    /* Get the recipes from the SD card */
    fetchRecipes();
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
    lcd.setCursor(0, 2);
    lcd.print("Scale zeroed!       ");
    lcd.display();

    /* Set up comms with the WiFi-module */
    lcd.setCursor(0, 3);
    lcd.print("Connecting to WiFi..");
    lcd.display();
    while (true){
        IPAddress = getIPAddress();
        if (IPAddress != ""){
            break;
        }
        else {
            delay(50);
        }
    }
    Serial.print("IP: ");
    Serial.println(IPAddress);

    /* Send the recipes to the WiFi-module */
    WIFI_MODULE.write(COMMS_RECIPE_FLAG);
    Serial.println("Sending recipes...");
    WIFI_MODULE.write(numRecipes); // Send the number of recipes read from SD card
    for (int r = 0; r < numRecipes; r++){
        WIFI_MODULE.write((uint8_t*) recipes[r].name, sizeof(recipes[r].name));
        WIFI_MODULE.write((uint8_t) recipes[r].num_ingredients); // Send the number of ingredients
        for (int i = 0; i < recipes[r].num_ingredients; i++){
            WIFI_MODULE.write((uint8_t *)recipes[r].ingredients[i].beverage, sizeof(recipes[r].ingredients[i].beverage));
        }
    }
    /* Verify that all data has been received before proceeding */
    Serial.print("Waiting for reply...");
    while (true)
    {
        if (WIFI_MODULE.available() > 0 && WIFI_MODULE.read() == COMMS_RECIPE_ACK){
            Serial.println("Recipes sent successfully!");
            break;
        }
        else {
            delay(50);
        }
    }

    lcd.setCursor(0, 3);
    lcd.print("WiFi OK!            ");
    lcd.display();

    /* Rotary encoder */
    pinMode(ROT_ENC_CLK_PIN, INPUT);
    pinMode(ROT_ENC_DAT_PIN, INPUT);
    pinMode(ROT_ENC_BTN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ROT_ENC_BTN_PIN), buttonPressedISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(ROT_ENC_DAT_PIN), incrementCounterISR, CHANGE);

    /* Drink order notification from WiFi-module */
    pinMode(ORDER_NOTIFY_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ORDER_NOTIFY_PIN), orderNotifyISR, RISING);

    /* Cleaning program button */
    pinMode(CLEAN_NOTIFY_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CLEAN_NOTIFY_PIN), cleanNotifyISR, RISING);

    /* Finalize */
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Order your drinks at");
    lcd.setCursor(0, 2);
    lcd.print(IPAddress);
    lcd.display();

    /* Let the IP address be visible for some time */
    delay(10000);
}

void loop()
{
    // TODO: Do something cool with the RGB LED strip

    if (updateFlag) {
        lcd.clear();
        for (int i = 0; i < 4; i++){
            /* Draw the drink options on the display */
            lcd.setCursor(1, i);
            char buf[MAX_STRING_LENGTH-1];
            snprintf(buf, sizeof(buf), "%s", recipes[(rotaryEncoderCounter / 4) * 4 + i].name);
            lcd.print(buf);
        }
        lcd.setCursor(0, rotaryEncoderCounter % 4);
        lcd.print(">");
        lcd.display();
        updateFlag = false;
    }

    if (isWebOrderAvailable){
        placeWebOrder();
        isWebOrderAvailable = false;
        updateFlag = true;
    }

    else if (isButtonPressed){
        isButtonPressed = false;

        Serial.println("Drink order coming up!");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Confirm order:");
        lcd.setCursor(0, 1);
        lcd.print(recipes[rotaryEncoderCounter].name);
        lcd.setCursor(0, 3);
        lcd.print("> OK!");
        lcd.display();

        unsigned long time = millis();
        constexpr uint8_t timeout = 5; // seconds
        while (millis() - time < timeout * 1000) {
            if (isButtonPressed){
                mixDrink(&recipes[rotaryEncoderCounter], pumps, &lcd);
                break;
            }
            else {
                lcd.setCursor(15, 3);
                lcd.print("(");
                lcd.print(timeout - (millis() - time) / 1000);
                lcd.print("s)");
                lcd.display();
                delay(100);
            }
        }
        isButtonPressed = false;
        updateFlag = true;
    }

    else if (isCleaningRequested) {
        isCleaningRequested = false;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Press cleanup button");
        lcd.setCursor(0, 1);
        lcd.print("again to confirm.");

        unsigned long time = millis();
        constexpr uint8_t timeout = 3; // seconds
        while (millis() - time < timeout * 1000) {
            if (isCleaningRequested){
                cleanPump(pumps, 10, &lcd);
                break;
            }
            else {
                lcd.setCursor(15, 3);
                lcd.print("(");
                lcd.print(timeout - (millis() - time) / 1000);
                lcd.print("s)");
                lcd.display();
                delay(100);
            }
        }
        isCleaningRequested = false;
        updateFlag = true;
    }
}



/***********************************************************************************/

/**************************************************
 * @brief  Get the IP address of the WiFi module 
 * @return The IP address, if available
 **************************************************/
String getIPAddress(){
    if ( WIFI_MODULE.available() > 0 && WIFI_MODULE.read() == COMMS_IP_FLAG ){
        char buf[20];
        byte data[4];
        String ip;
        data[0] = WIFI_MODULE.read();
        data[1] = WIFI_MODULE.read();
        data[2] = WIFI_MODULE.read();
        data[3] = WIFI_MODULE.read();

        snprintf(buf, sizeof(buf), "%d.%d.%d.%d", data[0], data[1], data[2], data[3]);
        ip = String(buf);
        WIFI_MODULE.write(COMMS_IP_ACK); // We have received the IP address
        return ip;
    }
    else
        return "";
}

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

    Serial.print("Looking for file: ");
    Serial.print(FILENAME);
    Serial.print("...");
    if (!SD.exists(FILENAME)){
        Serial.println("failed!");
        return 2;
    }
    Serial.println("OK!");

    return 0;
}


/**************************************************************
 * @brief  Fetch drink recipes and pump contents from SD card
 *************************************************************/
void fetchRecipes(){
    SDCard = SD.open(FILENAME);
    if (SDCard){
        /* Parse the JSON document into a practical class */
        deserializeJson(jsonObject, SDCard);

        SDCard.close();
        Serial.println("Closed file.");

        /* Get the number of recipes */
        numRecipes = jsonObject["numberOfRecipes"].as<int>();

        /* Get the pump contents */
        for (int i = 0; i < NUMBER_OF_PUMPS; i++){
            pumps[i].id    = i + 1;
            strncpy(pumps[i].drink, jsonObject["pumps"][i].as<char *>(), MAX_STRING_LENGTH);
        }

        /* Get the recipes */
        for (int i = 0; i < numRecipes; i++){
            strncpy(recipes[i].name, jsonObject["recipes"][i]["name"].as<char *>(), MAX_STRING_LENGTH);
            recipes[i].num_ingredients = jsonObject["recipes"][i]["numberOfIngredients"].as<int>();
            for (int j = 0; j < recipes[i].num_ingredients; j++){

                strncpy(recipes[i].ingredients[j].beverage, jsonObject["recipes"][i]["ingredients"][j]["type"].as<char *>(), MAX_STRING_LENGTH);
                recipes[i].ingredients[j].volume   = jsonObject["recipes"][i]["ingredients"][j]["quantity"].as<int>();
            }
        }
    }
    else {
        /* No recipes found */
        Serial.println("Could not open SD card.");
        numRecipes = 0;
    }

    /* Print for debugging purposes */
    // for (int i = 0; i < NUMBER_OF_PUMPS; i++){
    //     Serial.print(pumps[i].id);
    //     Serial.print(" ");
    //     Serial.println(pumps[i].drink);
    // }

    // for (int i = 0; i < numRecipes; i++)
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

    return;
}


/* Interrupt Service Routines */

void buttonPressedISR(){
    isButtonPressed = true;

    return;
}

void incrementCounterISR(){
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();

    /* Simple debouncer */
    if ( interruptTime - lastInterruptTime < 10 )
        return;

    rotaryEncoderCurrentState = digitalRead(ROT_ENC_DAT_PIN);

    if ((rotaryEncoderLastState == LOW) && (rotaryEncoderCurrentState == HIGH)){
        if (digitalRead(ROT_ENC_CLK_PIN) == HIGH){
            rotaryEncoderCounter++;
        }
        else {
            rotaryEncoderCounter--;
        }

        rotaryEncoderCounter = rotaryEncoderCounter < 0          ?   0        : rotaryEncoderCounter;
        rotaryEncoderCounter = rotaryEncoderCounter > numRecipes ? numRecipes : rotaryEncoderCounter;

        updateFlag = true;
    }
    rotaryEncoderLastState = rotaryEncoderCurrentState;
    lastInterruptTime = interruptTime;

    return;
}


void orderNotifyISR(){
    isWebOrderAvailable = true;

    return;
}

void cleanNotifyISR(){
    isCleaningRequested = true;

    return;
}



/***************************************************************************************/

void placeWebOrder(){
    char drinkName[MAX_STRING_LENGTH];
    bool isDataValid = false;
    if (WIFI_MODULE.read() == COMMS_BEGIN_ORDER) {
        WIFI_MODULE.readBytes(drinkName, MAX_STRING_LENGTH);
        if (WIFI_MODULE.read() == COMMS_END_ORDER){
            isDataValid = true;
        }
    }

    if (isDataValid) {
        Recipe* rec = getRecipeFromName(drinkName, MAX_STRING_LENGTH);
        Serial.print("Online drink order found:");
        Serial.println(rec->name);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Web order coming up:");
        lcd.setCursor(0, 2);
        lcd.print(rec->name);
        delay(2000);

        mixDrink(rec, pumps, &lcd);
    }

    return;
}

// Get recipe drink object from drink name
Recipe* getRecipeFromName( char* drinkName, int lenDrinkName ){
    for (int i = 0; i < numRecipes; i++){
        if (strncmp(drinkName, recipes[i].name, MAX_STRING_LENGTH) == 0){
            return &recipes[i];
        }
    }
}