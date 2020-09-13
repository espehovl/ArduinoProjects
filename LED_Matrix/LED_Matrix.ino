/* LED matrix application */

#include <PubSubClient.h>
#include <ESP8266WiFi.h>

/* This file requires the Adafruit NeoPixel library! */
#include "driver.h"
#include "color.h"
#include "symbols.h"

/* Defines */


/* Local variables */
static STATE currentState;
static STATE newState;

static const char* ssid = "";
static const char* pass = "";
static const char* mqttServer = "10.0.0.4";
static const int   mqttPort   = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

/* Local functions */
void MQTT_cb( char* topic, uint8_t* payload, unsigned int len );

void countdown( void );

/************************************************************************/

void setup() 
{
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  
  DM_SetUpDisplay();

  // Display he WiFi-status
  DM_DrawBitMap( wifi_bits, COLOR_RED, COLOR_BLACK );
  
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.println("Connecting to WiFi...");
    delay(500);
  }
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
  DM_DrawBitMap( wifi_bits, COLOR_GREEN, COLOR_BLACK );
  delay(500);
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(MQTT_cb);

  DM_DrawBitMap( comms_bits, COLOR_RED, COLOR_BLACK );
  delay(500);
  
  while( !client.connected() ) {
    Serial.println("Connecting to MQTT...");
    if ( client.connect("ESP8266") ){
      Serial.println("MQTT successful!");
      DM_DrawBitMap( comms_bits, COLOR_GREEN, COLOR_BLACK );
      delay(500);
    }
    else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
      while ( true );
    }
  }

  client.publish("esp/display","Display service active");
  client.subscribe("esp/display/plant");

  // Set the main state upon startup
  currentState = STATE_MAIN;
  newState = currentState;
  
  TiminiLogo( COLOR_FJELLBEKK );
  
}

void loop() 
{
  client.loop();

  // Switch on the current state
  switch ( currentState ) {
    case STATE_MAIN:
      // Something
      break;
      
    case STATE_GARDEN:
      // Something else
      break;
      
    case STATE_ERROR:
      // TODO: NOT REALLY FUNCTIONING AS OF NOW
      // Something went wrong, stop the service
      // Indicate the type of error...
      if ( !client.connected() ) {
        DM_DrawBitMap( comms_bits, COLOR_RED, COLOR_BLACK );
      }
      else if ( WiFi.status() != WL_CONNECTED ) {
        DM_DrawBitMap( wifi_bits, COLOR_RED, COLOR_BLACK );
      }
      while ( true );
      
    default:
      break;
  }
}

/************************************************************************/

void MQTT_cb( char* topic, uint8_t* payload, unsigned int len ) 
{
  Serial.print( "Message received!\r\nTopic: " );
  Serial.println( topic );
  Serial.print( "Payload: " );
  Serial.println( (char*)payload );
  countdown();
  return;
}

void countdown()
{
  const unsigned char * nums[] = {  nine_bits, eight_bits, seven_bits, six_bits,
                              five_bits, four_bits, three_bits, two_bits,
                              one_bits, zero_bits };
  delay(1000);
  for (int i = 0; i < 10; i++ ) {
    DM_DrawBitMap( nums[i], COLOR_WHITE, COLOR_BLACK );
    delay(1000);
  }
  for ( int i = 0; i < 5; i++ ) {
    DM_Clear();
    delay(500);
    DM_DrawBitMap( zero_bits, COLOR_WHITE, COLOR_BLACK );
    delay(500);
  }
  DM_Clear();
}
