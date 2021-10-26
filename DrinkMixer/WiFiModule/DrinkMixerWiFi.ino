/*******************************************
 * Host the web server and send commands to
 * the drink mixer.
 * 
 ******************************************/

#include <string.h>

#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

#include "DrinkMixerWiFi.h"
#include "site.h"
#include "credentials.h" // SSID and passwords are stored in this file

char ssid[] = CRED_WIFI_SSID;
char pass[] = CRED_WIFI_PASS;

char site[8196];        // The main site html. Let's have some headroom here, shall we?

uint8_t receivedRecipes;

WebRecipe recipes[NUMBER_OF_RECIPES];

ESP8266WebServer server;

void homepage();
void orderDrink();


void setup()
{
    Serial.begin(9600);

    pinMode(NOTIFY_PIN, OUTPUT);
    digitalWrite(NOTIFY_PIN, LOW);

    WiFi.begin(ssid, pass);
    while(WiFi.status() != WL_CONNECTED){
        delay(500);
    }

    // Send the IP address to the Mega, to show on the display
    while (true){
        Serial.write(COMMS_IP_FLAG); // Notify that we are about to send the IP address
        Serial.write(WiFi.localIP()[0]);
        Serial.write(WiFi.localIP()[1]);
        Serial.write(WiFi.localIP()[2]);
        Serial.write(WiFi.localIP()[3]);
        delay(100);
        if (Serial.available() > 0 && Serial.read() == COMMS_IP_ACK){
            // The IP address has been received
            break;
        }
    }

    /* Receive the recipes from the Mega */
    while (true){
        if (Serial.available() > 0 && Serial.read() == COMMS_RECIPE_FLAG){
            receivedRecipes = Serial.read(); // First byte is number of recipes
            int controlCounter = 0;
            for (int r = 0; r < receivedRecipes; r++){
                Serial.readBytes(recipes[r].name, MAX_STRING_LENGTH); // Receive the name of the drink
                Serial.readBytes(&recipes[r].num_ingredients, sizeof(uint8_t)); // Receive the number of ingredients
                for (int i = 0; i < recipes[r].num_ingredients; i++){
                    // Receive the drink ingredient types
                    Serial.readBytes(recipes[r].ingredients[i], MAX_STRING_LENGTH);
                }
                controlCounter++;
            }
            if (controlCounter == receivedRecipes){
                // The recipes have been received, acknowledge
                Serial.write(COMMS_RECIPE_ACK);
                break;
            }
        }
        else
            delay(100);
    }

    char listItem[512];
    char ingredientList[256];
    /* Complete the html page according to the recipes */
    strncat_P(site, top, sizeof(top)); // Copy the top section
    for (int r = 0; r < receivedRecipes; r++){
        memset(listItem, 0, sizeof(listItem));
        memset(ingredientList, 0, sizeof(ingredientList));
        for (int i = 0; i < recipes[r].num_ingredients; i++){
            char beverageBuf[40];
            snprintf(beverageBuf, sizeof(beverageBuf), "<p>%s</p>", recipes[r].ingredients[i]);
            strncat(ingredientList, beverageBuf, sizeof(beverageBuf));
        }
        snprintf(listItem, sizeof(listItem), "<a href=\"/drink?type=%s\"><li>%s %s</li></a>", recipes[r].name, recipes[r].name, ingredientList);
        strncat(site, listItem, sizeof(listItem)); // Add the entries for each drink
    }
    strncat_P(site, tail, sizeof(tail)); // Add the rest of the html

    /* Events */
    server.on("/", homepage);
    server.on("/drink", orderDrink);

    /* Start the server */
    server.begin();
}

void loop()
{
    /* Handle it */
    server.handleClient();
}

/*************************************************************/


void homepage(){
    /* Send the web page */
    server.send(200, "text/html", site);

    return;
}

void orderDrink(){
    /* A drink has been ordered. Fetch it. */
    String drink = server.arg("type");
    /* Transmit the ordered drink to the Mega */
    if (drink != "null" && drink != "Null"){
        Serial.write(COMMS_BEGIN_ORDER);
        Serial.write(drink.c_str(), MAX_STRING_LENGTH);
        Serial.write(COMMS_END_ORDER);

        /* Alert the Mega that an order was received */
        digitalWrite(NOTIFY_PIN, HIGH);
        delay(50);
        digitalWrite(NOTIFY_PIN, LOW);
    }

    /* Send an empty response (204) back */
    server.send(204, "");

    return;
}
