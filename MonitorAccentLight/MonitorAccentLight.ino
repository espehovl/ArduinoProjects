/***************************************************************
 * AUTHOR: 
 *  Espen Hovland
 * 
 * INFO:
 *  Application for monitor ambience lighting. Consists of a
 *  Wemos D1 Mini connected to 10 LED long "NeoPixel" strip. In
 *  addition, a generic breakout light sensor is added, in
 *  order to be able to dim the lights down when the room is
 *  dark.
 * 
 * There are a few modes/states to be aware of:
 *  STATE_BOOT  - The initial state that the system is in at
 *                boot. Does not mean much in itself, it just
 *                ensures the entry of the proper state after
 *                resetting/turning on the Wemos D1 mini
 * 
 *  STATE_MAIN  - The "daytime" state. Allows max brightness
 *                and is only active during the day time.
 * 
 *  STATE_NIGHT - The "night time" state. A lower max
 *                brightness is set. Only active during the
 *                specified night hours.
 * 
 *  MODE_DARK   - "Dark mode". When the room is dark, the 
 *                brightness of the LED strip is set to a 
 *                specified dark mode brightness (off).
 * 
 *  MODE_LIGHT  - "Light mode". When the room is lit above
 *                a certain threshold, the LED strip is turned
 *                on into the appropriate state, i.e. main 
 *                state or night state.
 * 
 * NOTE:
 *  Due to the few ground pins on the Wemos D1 mini, and because
 *  I don't want to use a breadboard, an improvised ground pin
 *  is enabled by setting one digital pin as an output, but
 *  in the LOW state. It works flawlessly so far, at least.
 * 
 **************************************************************/

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>
#include <WidgetRTC.h>

/* 
Do you want the system to run according to the time of the RTC
in order to toggle between high/low brightness of the LED strip,
leave this at 1. Light sensing will still be enabled.

If you want the system to operate only according to the ambient
light, set this to 0. This will override any time-control.
*/
#define OPERATION_MODE   0


/* DEFINES */
#define LED_PIN       ( 14 ) // D5
#define LED_COUNT     ( 10 ) 

#define FAUX_GND      ( 12 ) // D6

/* States/modes */
#define STATE_BOOT    (  0 )
#define STATE_MAIN    (  1 )
#define STATE_NIGHT   (  2 )

#define MODE_DARK     (  0 )
#define MODE_LIGHT    (  1 )

/* Time limits (hours) for night and day mode */
#define NIGHT_START   ( 22 ) 
#define NIGHT_END     (  8 )

/* Read interval of ambient light sensor. 
   Must be larger than 1 second */
#if OPERATION_MODE == 1
  #define READ_INTERVAL (  5 ) // seconds
#elif OPERATION_MODE == 0
  #define READ_INTERVAL (  2 ) // seconds
#endif

/* Credentials */
static const char* ssid = "YOUR WIFI SSID HERE";
static const char* pass = "YOUR WIFI PASS HERE";
static const char* auth = "YOUR BLYNK AUTH TOKEN HERE";

/* Local variables */
static uint8_t        R = 255;
static uint8_t        G = 255;
static uint8_t        B = 255;
static bool           updateFlag          = false;
static bool           updateBrightness    = false;
static bool           readBrightness      = false;
static uint8_t        brightness;
static uint8_t        oldBrightness;
static uint8_t        oldNightBrightness;
static uint8_t        darkBrightness      = 0;
static const uint8_t  initBrightness      = 100;
static const uint8_t  maxBrightness       = 200;
static const uint8_t  maxNightBrightness  = 100;
static uint16_t       ambientBrightness;
static const uint16_t ambientBrtThreshold = 5; // TODO: Adjust this appropriately! 
static uint8_t        currentState;
static uint8_t        prevState;
static uint8_t        mode;

/* Local object constructions */
Adafruit_NeoPixel strip( LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800 );
WidgetRTC         rtc;
 

/* Blynk functionalities */
/* Start the RTC service when connected to Blynk */
BLYNK_CONNECTED()
{
  rtc.begin();
}

/* Output from the RGB color picker */
BLYNK_WRITE(V0)
{
  R = param[0].asInt();
  G = param[1].asInt();
  B = param[2].asInt();
  updateFlag = true;
}

/* Output from brightness slider */
BLYNK_WRITE(V1)
{
  brightness = param.asInt();
  
  /* Ensure no brightness higher than max value allowed */
  if ( brightness > maxBrightness ) {
    brightness = maxBrightness;
    Blynk.virtualWrite(V1, brightness);
  }

  updateFlag = true;
  
  /* Print the timestamp to the serial monitor */
  char buf[25];
  sprintf(buf, "Time: %02d:%02d:%02d", hour(), minute(), second());
  Serial.println(buf);

}


/* LOCAL FUNCTIONS */

void getAmbientBrightness(uint16_t &brt);
void updateStrip();
void enterNightMode();
void leaveNightMode();
void enterDarkMode();
void leaveDarkMode();

/**************************************************************/
void setup() {
  Serial.begin(9600);

  /* Create a "faux" ground pin for the light sensor */
  pinMode(FAUX_GND, OUTPUT);
  digitalWrite(FAUX_GND, LOW);

  /* Enable analog reading of light sensor */
  pinMode(A0, INPUT);

  Blynk.begin(auth, ssid, pass);

  /* Set the initial brightness and update the Blynk widgets */
  brightness = initBrightness;
  Blynk.virtualWrite(V1, brightness);
  Blynk.virtualWrite(V0, R, G, B);

  /* Set up NeoPixel strip*/
  strip.begin();
  strip.show();
  strip.setBrightness(brightness);

  /* Allow the RTC module to connect for a second */
  delay(1000);

  /* Print the start-up time (mainly for debugging purposes) */
  char buf[30];
  sprintf(buf, "\nStart time: %02d:%02d:%02d", hour(), minute(), second());
  Serial.println(buf);
  
  /* Emable the strip */
  updateStrip();

  /* Initialize the correct state */
  currentState = STATE_BOOT;
  prevState = currentState;
  mode = MODE_LIGHT;
}

void loop() {
  Blynk.run();

  #if OPERATION_MODE == 1
    /* The light will be dimmed at night and brighter in the daytime */
    if (hour() >= NIGHT_START && (currentState != STATE_NIGHT)) { //} || currentState == STATE_BOOT)) {
      enterNightMode();
    }
    else if (hour() >= NIGHT_END && hour() < NIGHT_START && (currentState != STATE_MAIN)){//} || currentState == STATE_BOOT)) {
      leaveNightMode();
    }
  #elif OPERATION_MODE == 0
    if (currentState != STATE_MAIN) {
      /* Enter a base state the first time only */
      currentState = STATE_MAIN;
      brightness   = maxBrightness;
    
      /* Update the brightness slider in Blynk */
      Blynk.virtualWrite(V1, brightness);
  
      /* Revert to the original max value for brightness slider */
      Blynk.setProperty(V1, "max", maxBrightness);
  
      /* Flag an update of the strip to the new brightness */
      updateFlag = true;
    }
    
  #endif

  if ( second() % READ_INTERVAL == 0 ) {
    readBrightness = true;
  }
  
  if (readBrightness) {
    /* Get and print the ambient brightness in the room */
    Serial.print("Reading ambient brightness: ");
    getAmbientBrightness(ambientBrightness);
    Serial.println(ambientBrightness);

    if (mode == MODE_LIGHT && ambientBrightness < ambientBrtThreshold ) {
      enterDarkMode();
    }
    else if (mode == MODE_DARK && ambientBrightness > ambientBrtThreshold ) {
      leaveDarkMode();
    }
  } 
  
  if (updateFlag) {
    /* Update the strip */
    updateStrip();
    updateFlag = false;
  }
  
  if (readBrightness) {
    /* Delay for a second, to avoid multiple rounds of reading the ambient light */
    readBrightness = false;
    delay(1000);
  }
}

/*************************************************************/

/*****************************************************
 * 
 * @brief Get the ambient brightness in the room as
 *        measured by the light sensor.
 * 
 * @param brt Reference to a uint16_t in which to
 *        store the brightness value.
 * 
 *****************************************************/
void getAmbientBrightness(uint16_t &brt)
{
  /* Set the brightness from the measured value
     and "convert" it to a more meaningful number */
  brt = 1024 - analogRead(A0);

  return;
}


/*****************************************************
 * 
 * @brief Refresh the LED strip with brightness/colors
 * 
 *****************************************************/
void updateStrip()
{
  /* Update the brightness */
  strip.setBrightness(brightness);
  
  /* Set the color */
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(R, G, B));
  }

  /* Update the strip */
  strip.show();
}


/*****************************************************
 * 
 * @brief Enter the night mode. Old brightness will be
 *        stored, and a new, lower brightness is
 *        enabled. Updates the widgets in Blynk with
 *        new limits and values for the brightness.
 * 
 *****************************************************/
void enterNightMode()
{ 
  static bool prevCalled;
  
  if (currentState == STATE_BOOT) {
    Serial.println("Booted into NIGHT mode");
  }
  else {
    Serial.println("Entering night mode...");
  }
  
  currentState = STATE_NIGHT;

  /* Store the daytime brightness */
  oldBrightness = brightness;

  /* Restore the old night brightness, iff the value has been set previously */
  if (prevCalled) {
    brightness = oldNightBrightness;
  }

  /* Avoid maxing out the brightness */
  if (brightness > maxNightBrightness) {
    brightness = maxNightBrightness;
  }

  /* Update the brightness slider in Blynk */
  Blynk.virtualWrite(V1, brightness);
  
  /* Set the new max value for the brightness slider */
  Blynk.setProperty(V1, "max", maxNightBrightness);
  
  /* Flag an update of the strip to the new brightness */
  updateFlag = true;

  /* Notify that this function has been called */
  prevCalled = true;
}


/*****************************************************
 * 
 * @brief Leave the night mode. Old brightness will be
 *        restored. Updates the widgets in Blynk with
 *        new limits and values for the brightness.
 * 
 *****************************************************/
void leaveNightMode()
{
  static bool prevCalled;
  
  if (currentState == STATE_BOOT) {
    Serial.println("Booted into DAY mode");
  }
  else {
    Serial.println("Leaving night mode...");
  }
  
  currentState = STATE_MAIN;

  /* Store the nighttime brightness */
  oldNightBrightness = brightness;

  /* Restore the old night brightness, iff the value has been set previously */
  if (prevCalled) {
    brightness = oldBrightness;
  }
  else {
    brightness = initBrightness;
  }

  /* Update the brightness slider in Blynk */
  Blynk.virtualWrite(V1, brightness);
  
  /* Revert to the original max value for brightness slider */
  Blynk.setProperty(V1, "max", maxBrightness);
  
  /* Flag an update of the strip to the new brightness */
  updateFlag = true;

  /* Notify that this function has been called */
  prevCalled = true;
}


/*****************************************************
 * 
 * @brief Enter the dark mode. Old brightness will be
 *        stored, and a new, lower brightness (off) is
 *        enabled. Updates the widgets in Blynk with
 *        new values for the brightness.
 * 
 *****************************************************/
void enterDarkMode()
{
  Serial.println("Dark mode enabled");
  mode = MODE_DARK;

  /* Store the previous state */
  prevState = currentState;

  /* Store the previous brightness */
  oldBrightness = brightness;

  /* Set new value for brightness */
  brightness = darkBrightness;

  /* Update the brightness slider in Blynk */
  Blynk.virtualWrite(V1, brightness);

  updateFlag = true;
}


/*****************************************************
 * 
 * @brief Leave the dark mode. Old brightness will be
 *        restored. Updates the widgets in Blynk with
 *        new values for the brightness.
 * 
 *****************************************************/
void leaveDarkMode()
{
  Serial.println("Dark mode disabled");
  mode = MODE_LIGHT;

  /* Revert to the previous state */
  currentState = prevState;

  /* Restore the previous brightness */
  brightness = oldBrightness;

  /* Update the brightness slider in Blynk */
  Blynk.virtualWrite(V1, brightness);

  updateFlag = true;
}