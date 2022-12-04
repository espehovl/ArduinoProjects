/***********************************************************
 * Arduino Drink Mixer Application
 * Author: Espen Hovland, Steinar Valbø
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


HX711_ADC           loadCell {LOADCELL_DATA_PIN, LOADCELL_SCLK_PIN};
LiquidCrystal_I2C   lcd      {LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS};
Adafruit_NeoPixel   ledStrip {LED_STRIP_NUM_LEDS, LED_STRIP_DATA_PIN, NEO_GRB + NEO_KHZ800};

DynamicJsonDocument jsonObject(2048);

Pump                pumps[NUMBER_OF_PUMPS];
Recipe              recipes[NUMBER_OF_RECIPES];
File                SDCard;
String              IPAddress;

volatile bool       isButtonPressed           = false;
volatile bool       updateFlag                = true;
volatile int8_t     rotaryEncoderCounter      = 0;
volatile uint8_t    rotaryEncoderCurrentState = 0;
volatile uint8_t    rotaryEncoderLastState    = 0;

static uint8_t      numRecipes;                             // Number of actual recipes read from SD card
static uint8_t      numPumps;                               // Number of actual pumps read from SD card
static char         *SSID                     = nullptr;
static bool         loadCellDataReady         = false;
static float        previousWeightGrams       = 0.0;
static float        currentWeightGrams        = 0.0;

#define FILENAME            "recipes.txt"
#define WIFI_ENABLED           0
#define WIFI_MODULE         Serial2
#define WIFI_WAIT_TIMEOUT   ( 10 ) // Seconds

void    cleaningLoop();

int     initSDCard();
void    fetchWiFiDetails();
void    fetchDataFromSD();
void    placeWebOrder();
Recipe* getRecipeFromName(char *drinkName, int lenDrinkName);



/* Interrupt Service Routines */
void buttonPressedISR();
void incrementCounterISR();

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
#if WIFI_ENABLED
    WIFI_MODULE.begin(9600); // For communicating with the WiFi module
#endif
    Wire.begin(); // I2C comms

    /* Init RGB LEDs */
    ledStrip.begin();
    ledStrip.setBrightness(LED_STRIP_BRIGHTNESS);
    ledStrip.clear();
    ledStrip.show();

    /* Init LCD display */
    lcd.init();
    lcd.backlight();
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
    lcd.display();

    /* Init SD card and collect recipes */
    pinMode(SPI_CS_PIN, OUTPUT);
    digitalWrite(SPI_CS_PIN, HIGH);

    if (initSDCard() != 0){
        Serial.println("Could not initialize SD card!");
        lcd.setCursor(0, 1);
        lcd.print("SD card failed.");
        lcd.display();
        while (true);
    }

    lcd.setCursor(0, 1);
    lcd.print("Reading SD card...");
    lcd.display();

    /* Get the recipes from the SD card */
    fetchDataFromSD();
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
    loadCell.begin();
    loadCell.start(2000, true);
    loadCell.setCalFactor(-943.33);
    lcd.setCursor(0, 2);
    lcd.print("Scale zeroed!       ");
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

    /* Turn on the LED lights to indicate ready-state */
    for (int i = 0; i < LED_STRIP_BOTTLE_LEDS; i++){
        for (int brightness = 0; brightness < 256; brightness += 10){
            ledStrip.setPixelColor(pump_LED_to_ID[i], brightness, brightness, brightness);
            ledStrip.show();
            delay(10);
        }
        ledStrip.show();
        delay(50);
    }


    for (int i = LED_STRIP_PARTY_LIGHTS; i < LED_STRIP_NUM_LEDS; i++)
        for (int j = 0; j < 255; j += 20){
        {
            ledStrip.setPixelColor(i, j, 0, 0); // Idle color
            ledStrip.show();
            delay(1);
        }
    }

    /* Finalize */

    loadCell.tareNoDelay();
    lcd.clear();

    // Enter main menu
    lcd.setCursor(0,0);
    lcd.print("  - BarBot  1000 -  ");
    lcd.setCursor(0, 1);
    lcd.print(" Get to drinking!");
    lcd.setCursor(0, 2);
    lcd.print(" Pump control");

    unsigned long T = millis();
    constexpr int timeout = 10;

    while (millis() - T < timeout * 1000) {
        if (updateFlag){

            rotaryEncoderCounter = rotaryEncoderCounter >= 1 ? 1 : 0;
            switch (rotaryEncoderCounter){
                case 0:
                    lcd.setCursor(0, 1);
                    lcd.print(">");
                    lcd.setCursor(0, 2);
                    lcd.print(" ");
                    break;
                case 1:
                    lcd.setCursor(0, 2);
                    lcd.print(">");
                    lcd.setCursor(0, 1);
                    lcd.print(" ");
                    break;
                default:
                    break;
            }
            lcd.display();
            updateFlag = false;
        }

        if (isButtonPressed){
            isButtonPressed = false;
            if (rotaryEncoderCounter == 0){
                // Go to the main loop and let the party begin!
                break;
            }
            else {
                // Enter the cleaning menu. No return from this point!
                delay(100);
                cleaningLoop();
            }
        }
        else {
            char buf[MAX_STRING_LENGTH];
            snprintf(buf, MAX_STRING_LENGTH, "(%2ds)", timeout - (millis() - T) / 1000);
            lcd.setCursor(15, 3);
            lcd.print(buf);
            lcd.display();
            delay(50);
        }

    }

    // Reset relevant values before start
    rotaryEncoderCounter    = 0;
    isButtonPressed         = false;
    updateFlag              = true;
    loadCellDataReady       = true;

    lcd.clear();
    delay(100);
}

void loop()
{
    if (loadCell.update()) {
        currentWeightGrams = loadCell.getData();
        if (abs(currentWeightGrams - previousWeightGrams) > 1.0){
            if (currentWeightGrams < 0.5 && currentWeightGrams > -0.5){
                currentWeightGrams = 0.0;
            }
            previousWeightGrams = currentWeightGrams;
            loadCellDataReady   = true;
        }
    }

    if (loadCellDataReady){
        char buf[MAX_STRING_LENGTH - 1];
        char val[6];
        dtostrf(currentWeightGrams, 6, 0, val);
        snprintf(buf, sizeof(buf), " Load:     %s g", val);
        lcd.setCursor(0, 3);
        lcd.print(buf);
        loadCellDataReady = false;
        lcd.display();
        Serial.println(currentWeightGrams);
    }

    if (updateFlag) {
        for (int i = 0; i < 3; i++){
            /* Draw the drink options on the display */
            lcd.setCursor(0, i);
            char buf[MAX_STRING_LENGTH];
            snprintf(buf, sizeof(buf), " %-20s", recipes[(rotaryEncoderCounter / 3) * 3 + i].name);
            lcd.print(buf);
        }
        lcd.setCursor(0, rotaryEncoderCounter % 3);
        lcd.print(">");

        lcd.display();

        updateFlag = false;
    }

    if (isButtonPressed){
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
                mixDrink(&recipes[rotaryEncoderCounter], pumps, &lcd, &ledStrip, &loadCell);
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

        loadCell.tareNoDelay();

        isButtonPressed = false;
        updateFlag = true;
        loadCellDataReady = true;
        lcd.clear();
    }
}

/***********************************************************************************/

/**************************************************
 * @brief  Eternal loop for doing cleaning functions
 **************************************************/
void cleaningLoop(){
    updateFlag = true;
    rotaryEncoderCounter = 7;
    isButtonPressed = false;
    uint8_t currentCursorIndex;
    uint8_t currentRotEncValue;
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Select pump");
    lcd.setCursor(0, 1);
    lcd.print("  8 7 6 5 4 3 2 1  ");
    lcd.setCursor(0, 3);
    lcd.print("Push to start/stop");
    while (true){
        if (updateFlag){
            currentRotEncValue = 7 - (rotaryEncoderCounter > 7 ? 7 : rotaryEncoderCounter);
            currentCursorIndex = 16 - currentRotEncValue * 2;

            lcd.setCursor(0, 2);
            lcd.print("                    ");
            lcd.setCursor(currentCursorIndex, 2);
            lcd.print("^");
            lcd.display();

            updateFlag = false;
        }
        if (isButtonPressed){
            lcd.setCursor(currentCursorIndex, 2);
            lcd.print("X");
            lcd.display();

            isButtonPressed = false;
            while (!isButtonPressed){
                cleanPump(pumps + currentRotEncValue, currentRotEncValue,  1.0, &ledStrip);
            }
            isButtonPressed = false;
            lcd.setCursor(currentCursorIndex, 2);
            lcd.print("^");
            lcd.display();
        }
    }
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
 * @brief  Fetch drink recipes
 *         and pump contents from SD card
 *************************************************************/
void fetchDataFromSD(){
    SDCard = SD.open(FILENAME);
    if (SDCard){
        /* Parse the JSON document into a practical class */
        deserializeJson(jsonObject, SDCard);

        SDCard.close();
        Serial.println("Closed file.");

        /* Get the number of recipes and pumps */
        numRecipes = jsonObject["numberOfRecipes"].as<int>();
        numPumps   = jsonObject["numberOfPumps"].as<int>();

        /* Get the pump contents */
        for (int i = 0; i < numPumps; i++){
            pumps[i].id = i + 1;
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
        numPumps   = 0;
    }

    return;
}


/* Interrupt Service Routines */

void buttonPressedISR(){
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();

    /* Simple debouncer */
    if ( interruptTime - lastInterruptTime < 50 )
        return;

    isButtonPressed = true;

    lastInterruptTime = interruptTime;

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


/***************************************************************************************/

// Get recipe drink object from drink name
Recipe* getRecipeFromName( char* drinkName, int lenDrinkName ){
    for (int i = 0; i < numRecipes; i++){
        if (strncmp(drinkName, recipes[i].name, MAX_STRING_LENGTH) == 0){
            return &recipes[i];
        }
    }
}
