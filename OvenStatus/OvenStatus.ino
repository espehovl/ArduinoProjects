/*******************************************************
 * Oven status indicator and logger
 * 
 * AUTHOR: 
 *  Espen Hovland
 * 
 * INFO:
 *  Application for reading and logging the temperature 
 *  of an oven using a DS18B20 temperature sensor as a 
 *  probe on the oven, and a DHT11 sensor for measuring
 *  the temperature and relative humidity in the room.
 * 
 *  The data is uploaded to a Thingspeak server, for
 *  plotting purposes.
 * 
 * HARDWARE:
 *  - Wemos D1 mini
 *  - DS18B20 sensor
 *  - DHT11 sensor (3-pin breakout module)
 *  - 1602 LCD display with I2C module
 * 
*******************************************************/
#include <DHTesp.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define OVEN_TEMP_PIN      2 // D4
#define STATUS_LED_PIN     0 // D3
#define ROOM_TEMP_PIN     16 // D0

#define LCD_I2C_ADDRESS  0x3F
#define LCD_HOR_CHARS    16
#define LCD_VER_CHARS     2

OneWire           oneWireObj(OVEN_TEMP_PIN);
DallasTemperature pipeTempSensor(&oneWireObj);
DHTesp            roomTempSensor;
WiFiClient        client;
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_HOR_CHARS, LCD_VER_CHARS);

/* Credentials */
static const char* ssid     = "YOUR WIFI SSID HERE";
static const char* pass     = "YOUR WIFI PASS HERE";
static const char server[]  = "thingspeak.com";
static const char apiKey[]  = "YOUR API KEY";

static int          ovenStatus;
static float        pipeTemp;
static float        roomTemp; 
static float        roomRH;
static const float  pipeTempThreshold = 45.0; // celsius
static const int    uploadInterval    = 30;   // seconds

void  printDisplay    ( float temp, float ovenTemp, float rhum );
void  upload          ( float temp, float rhum, float ovenTemp, int ovenState );
void  getTemperatures ( float &temp, float &rhum, float &ovenTemp );
void  delaySeconds    ( int s );

void setup()
{
  Serial.begin(9600);
  
  /* Activate I2C */
  Wire.begin();
  
  /* Connect WiFi */
  WiFi.begin(ssid, pass);
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.println("Connecting to WiFi...");
    delay(500);
  }
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  /* Enable LED pin */
  pinMode(STATUS_LED_PIN, OUTPUT);

  /* Set up the room temperature sensor */
  roomTempSensor.setup(ROOM_TEMP_PIN, DHTesp::DHT11);

  /* Initialize the lcd display */
  lcd.init();
  lcd.backlight();
}

void loop()
{  
  /* Perform temperature sensor reading */
  getTemperatures(roomTemp, roomRH, pipeTemp);
  
  /* Set the oven status */
  ovenStatus = pipeTemp > pipeTempThreshold ? 1 : 0;

  /* Update display and send the data to Thingspeak */
  printDisplay(roomTemp, pipeTemp);
  upload(roomTemp, roomRH, pipeTemp, ovenStatus);
  
  /* Delay for a while */
  delaySeconds(uploadInterval);
}


/*****************************************************
 * 
 * @brief Prints the temperature to the 16x2 LCD display
 * 
 * @param temp Temperature of the room
 * 
 * @param ovenTemp Temperature of the oven 
 * 
 * @param rhum Relative humidity of the room
 * 
 ***************************************************/
void printDisplay( float temp, float ovenTemp, float rhum )
{
  char str[20]; //Slightly oversized, to allow for printf-magic
  lcd.clear();
  
  /* Print ambient temperature and humidity */
  lcd.setCursor(0,0);
  sprintf(str, "Room: %.0f%c %.1f%c%c", rhum, '%', temp, 223, 'C');
  lcd.print(str);

  /* Print tube temp */
  lcd.setCursor(0,1);
  sprintf(str, "Pipe:%9.1f%c%c", ovenTemp, 223, 'C');
  lcd.print(str);

  return;
}

/*****************************************************
 * 
 * @brief Uploads the oven temperature, room temperature,
 *        relative humidity and status of the oven to
 *        the Thingspeak server
 * 
 * @param temp The room temperature
 * 
 * @param rhum The relative humidity
 * 
 * @param ovenTemp The temperature of the oven
 * 
 * @param ovenState The state of the oven (i.e. on or off)
 * 
 ***************************************************/
void upload( float temp, float rhum, float ovenTemp, int ovenState )
{
  Serial.println( "Sending data: T = " + String(temp) + ", state = " + String(ovenState) );
  if ( client.connect(server, 80) ) {
    /* Send state and temperature data */
    client.println("GET https://api.thingspeak.com/update?api_key="
                 +String(apiKey)
                 +"&field1="+String(ovenState)
                 +"&field2="+String(temp)
                 +"&field3="+String(ovenTemp)
                 +"&field4="+String(rhum)
                 +" HTTP/1.1");
                 
    client.println("Host: https://api.thingspeak.com");
    client.println("Connection: close");
    client.println();
    Serial.println("Data delivered!\n\r");
  }

  /* Write the oven status to the LED */
  digitalWrite(STATUS_LED_PIN, ovenState);

  return;
}


/*****************************************************
 * 
 * @brief Read the temperature of the DHT11 sensor
 *        and store the results.
 * 
 * @param temp Reference to the variable in which
 *        to store the temperature reading
 * 
 * @param rhum Reference to the variable in which 
 *        to store the relative humidity reading
 * 
 * @param ovenTemp Reference to the variable in which
 *        to store the oven temperature reading
 * 
 ***************************************************/
void getTemperatures( float &temp, float &rhum, float &ovenTemp )
{
  delaySeconds(2);
  
  /* Get the room temperature */
  temp = roomTempSensor.getTemperature();
  rhum = roomTempSensor.getHumidity();
  
  /* Read the oven temperature */
  pipeTempSensor.requestTemperatures();
  ovenTemp = pipeTempSensor.getTempCByIndex(0);

  return;
}

/* Slightly more intuitive way of delaying for longer periods */
void delaySeconds( int s )
{
  delay(s * 1000);
}
