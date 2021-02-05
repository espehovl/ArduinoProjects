/* LED matrix application */

/* KNOWN ISSUES: 
 *  Some sort of stack error(?) occurs whenever a second
 *  bitmap is drawn after an initial one, and the device
 *  reboots. Super annoying, as it reboots every tenth
 *  seconds, due to the flower pot broadcasting its status.
 */
 
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

/* This file requires the Adafruit NeoPixel library! */
#include "driver.h"
#include "color.h"
#include "symbols.h"
#include "images.h"
#include "numbers.h"

/* Defines */
#define STATIC_DISPLAY 1

/* Local variables */
static bool  updateFlag;
static STATE currentState;
static STATE newState;
static byte  stateData[4] = { 0, 0, 0, 0 };

static const char* ssid       = "YOUR WIFI SSID";
static const char* pass       = "YOUR WIFI PASS";
static const char* mqttServer = "YOUR SERVER";
static const int   mqttPort   = 1883; // YOUR PORT

WiFiClient espClient;
PubSubClient client(espClient);

/* Local functions */
void MQTT_cb( char* topic, uint8_t* payload, unsigned int len );

void drawStateGarden( void );

void countdown( void );

/************************************************************************/

void setup() 
{
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  
  DM_SetUpDisplay();

  /* Display the WiFi-status */
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
  updateFlag = true;

  /* Default home screen */
  TiminiLogo( COLOR_FJELLBEKK );

  //DM_DrawColorImage( timini_pride_xpm );

  //delay(30000);
}

void loop() 
{
  client.loop();

  if ( newState != currentState ) {
    /* Raise the update flag */
    updateFlag = true;
    currentState = newState;
  }
  
  // Switch on the current state
  switch ( currentState ) {
    case STATE_MAIN:
      if ( updateFlag ) {
        //TiminiLogo( COLOR_FJELLBEKK );
        DM_DrawColorImage( xpm_30 );
        updateFlag = false;
      }
      break;
      
    case STATE_GARDEN:
      if ( updateFlag ) {
        drawStateGarden();
        updateFlag = false;
      }
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
      else {
        /* Signal a general error */
        DM_DrawBitMap( error_bits, COLOR_BLACK, COLOR_RED );
      }
      while ( true );
      
    default:
      break;
  }

  
}

/************************************************************************/

void MQTT_cb( char* topic, uint8_t* payload, unsigned int len ) 
{

  
  /* Variables for parsing payload */
  const char* delim = " ";
  byte newData[ sizeof(stateData) ];
  
  #if (STATIC_DISPLAY == 1)
    goto returnSection;
  #endif
  
  Serial.print( "Message received!\r\nTopic: " );
  Serial.println( topic );
  Serial.print( "Payload: " );
  Serial.println( (char*)payload );
 
  /* PCheck the topic... */
  if ( strncmp( topic, "esp/display/plant", 17 ) == 0 ) {
    /* If we get a plant message... */
    Serial.println("Message received from plant, parsing...");
    char* token = strtok( (char*)payload, delim );
    int   idx   = 0;
    while ( token != NULL && idx < sizeof(newData) ) {
      newData[idx] = atoi(token);
      token = strtok( NULL, delim );
      //Serial.println(newData[idx]);
      idx++;
    }

    /* Treat the information */
    if ( newData[0] != 1 ) {
      /* We are not getting correct information, get outta here! */
      goto returnSection;
    }
    else {
      /* We have received data about a plant, check if the status is
       * different from previous state */
      for ( idx = 0; idx < sizeof(newData); idx++ ) {
        if ( newData[idx] != stateData[idx] ) {
          Serial.println("New data!");
          Serial.print(newData[idx]);
          Serial.print(" - ");
          Serial.println(stateData[idx]);
          /* Data is different, update stateData and break out */
          memcpy( stateData, newData, sizeof(stateData) );

          /* Notify that we need to update the display */
          newState = STATE_GARDEN;
          updateFlag = true;
          break;
        }
      }
    }
    
  }
  else {
    /*Do something semi-random */
    countdown();
  }

returnSection:

  return;
}


void drawStateGarden( void )
{
  /* Draw the status of the current plant */

  /* Verify the plant display mode */
  if ( stateData[0] != 1 ) {
    /* We are not to display any plant info */
    return;
  }
  
  /* Display the plant ID */
  switch ( stateData[1] ) {
    case 0:
      DM_DrawBitMap( zero_bits, COLOR_WHITE, COLOR_BLACK );
      break;
    case 1:
      DM_DrawBitMap( one_bits, COLOR_WHITE, COLOR_BLACK );
      break;
    case 2:
      DM_DrawBitMap( two_bits, COLOR_WHITE, COLOR_BLACK );
      break;
  }
  delay(500);

  /* Display the status */
  if ( stateData[2] == 0 ) {
    /* All is well with the plant */
    DM_DrawBitMap( heart_bits, COLOR_RED, COLOR_BLACK );
    delay(1000);
    newState = STATE_MAIN;
  }
  else if ( stateData[2] == 1 ) {
    /* Plant needs water */
    DM_DrawBitMap( water_bits, COLOR_BLUE, COLOR_BLACK );
    delay(500);
  }

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
