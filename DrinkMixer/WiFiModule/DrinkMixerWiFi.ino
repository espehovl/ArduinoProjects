/*******************************************
 * Host the web server and send commands to
 * the drink mixer.
 * 
 ******************************************/

#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

#include "DrinkMixerWiFi.h"
#include "site.h"

char ssid[] = "edruroam";
char pass[] = "PastaFormaggio42";

int receivedRecipes;

Recipe recipes[NUMBER_OF_RECIPES];

ESP8266WebServer server;

void welcome();
void orderDrink();

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW); // Active low, this one. Light up when comms are okay


    // Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, pass);
    while(WiFi.status() != WL_CONNECTED){
        // Serial.print(".");
        delay(500);
    }
    // Serial.println("success!");
    digitalWrite(LED_BUILTIN, HIGH);


    // Send the IP address to the Mega, to show on the display
    // String ip = WiFi.localIP().toString();
    while (true){
        Serial.write(COMMS_IP_FLAG); // Notify that we are about to send IP address
        // Serial.write(ip.c_str());
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
    
    
    // TODO: Receive the recipes from the SD card
    // Use the same structs as in the Mega code
    // ...
    while (true){
        if (Serial.available() > 0 && Serial.read() == COMMS_RECIPE_FLAG){
            // TODO: Fill in this blank
        }
        delay(50);
    }


    /* Complete the html page according to the recipes */
    char listItem[100];
    snprintf(listItem, 100, "<a href=\"/drink?type=%s\"><li>%s</li></a>", "dummy", "dummier");
    // +++

    /* Events */
    server.on("/", welcome);
    server.on("/drink", orderDrink);

    /* Start the server */
    server.begin();
}

void loop()
{
    /* Handle it */
    server.handleClient();
}

/************************************************************/


void welcome(){
    /* The initial web page */
    server.send_P(200, "text/html", site);

    // TODO: Display the available drinks

    return;
}

void orderDrink(){
    /* A drink has been ordered. Get it. */
    String drink = server.arg("type");

    /* Transmit the ordered drink to the Mega */
    if (drink != "Null" && drink != "null"){
        // Serial.write(drink.c_str()); //... or something along these lines    
        // Serial.print("Just ordered: ");
        // Serial.println(drink);
    }
    else {
        // Serial.println("Invalid drink");
    }

    /* Send an empty response back */
    server.send(204, "");
}