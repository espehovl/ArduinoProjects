/* Glove application
 * Wemos D1 mini
 * Lots of inspiration from: https://github.com/bekspace/lolin_d1_mini-mpu6050/blob/master/d1_mini_mpu.ino 
 * 
 * */

#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_SSD1306.h"
#include "graphics.h"

#define SDA_PIN ( 4 )         // SDA, D2
#define SCL_PIN ( 5 )         // SCL, D1
#define I2C_SLAVE ( 0x14 )    // I2C address of slave (car). Arbitrary, but must match with the address set up by the car.

#define BTN_PIN  ( 2 ) // D4

#define DISP_WIDTH   (  128 )
#define DISP_HEIGHT  (   64 )
#define DISP_ADDRESS ( 0x3C ) // I2C address of display

#define IMU_ADDRESS ( (uint8_t)0x68 ) // I2C address of accelerometer

/* IMU raw data variables */
int16_t gyroRawX;
int16_t gyroRawY;
int16_t gyroRawZ;
int16_t accRawX;
int16_t accRawY;
int16_t accRawZ;

/* Arrays for storing calculated angles */
float accAngle[2];
float gyroAngle[2];
float angle[2];

/* Even more user-friendly variables */
float pitch;
float roll;

/* Timing variables */
unsigned long loopTimer;
unsigned long elapsedTime = 0; // Elapsed loop time in ms

/* Counter for the display service */
unsigned long ctr = 0;

/* Button pressed flag */
volatile bool running = false;

/* Create display object */
Adafruit_SSD1306 disp(DISP_WIDTH, DISP_HEIGHT, &Wire, -1);

void setupIMU(void);
void _readIMU(void);
void getRefinedData(void);
void startupSequence(int delayDuration);
void drawArrow(int speed, bool forward, int turn, bool right);

IRAM_ATTR void btnCallback(void);

void setup()
{
    /* Start I2C communications */
    Wire.begin();

    /* Start serial communications */
    Serial.begin(115200);

    /* Start the wireless communications */
    // TODO:

    /* Start the display */
    Serial.print("Starting display service... ");
    if (!disp.begin(SSD1306_SWITCHCAPVCC, DISP_ADDRESS)){
        Serial.println("display failed.");
    while (true);
    }
    disp.clearDisplay();
    disp.setTextSize(1);
    disp.setTextColor(WHITE);
    Serial.println("successful!");
    
    /* Set up the IMU unit */
    setupIMU();

    /* Set up the button interrupt */
    pinMode(BTN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN_PIN), btnCallback, FALLING);

    /* Do a nice little display sequence and wait for a while, to let the user prepare the glove */
    startupSequence(3);

    /* Start the timer as we finish setup */
    loopTimer = millis();
}

void loop()
{
    /* Get useful data from the accelerometer */
    getRefinedData();

    /* Convert the pitch into speed */
    int maxSpeed = 255;
    bool forwards;
    int speed;
    
    forwards = pitch > 0 ? true : false; // true if positive pitch (forward tilt), false otherwise

    if (forwards) {
        speed = map(pitch, 0, 50, 0, maxSpeed);
    }
    else {
        speed = map(pitch, 0, -50, 0, maxSpeed);
    }
    
    /* Set the max value if we get overflow */
    speed = speed > maxSpeed ? maxSpeed : speed;

    /* Convert the roll into turn */
    int maxTurn = 255;
    bool rightTurn;
    int turn;

    rightTurn = roll > 0 ? true : false; // true if rolling to the right, false otherwise

    if (rightTurn){
        turn = map(roll, 0, 50, 0, maxTurn);
    }
    else {
        turn = map(roll, 0, -50, 0, maxTurn);
    }

    /* Set the max vaue if we get overflow */
    turn = turn > maxTurn ? maxTurn : turn;

    /* Increment display counter */
    ctr++;

    if (running){
        /* If remote is running, display and transmit data */

        /* Display useful data */
        if (ctr > 100){
            char buf[50];

            //sprintf(buf, "Pitch: % 5.3f \n\rRoll:  % 5.3f", pitch, roll);
            
            disp.clearDisplay();
            disp.setCursor(0, 56);
            //disp.println(buf);
            
            sprintf(buf, "Speed:%c%3d Turn: %c%3d", forwards ? '+' : '-', speed, rightTurn ? '+' : '-', turn);

            /* Provide some feedback on the controller display */
            drawArrow(speed, forwards, turn, rightTurn);

            disp.println(buf);
            disp.invertDisplay(false);
            disp.display();
            
            /* Reset the counter */
            ctr = 0;
        }

        // TODO: Transmit the data to the car
        // Send: forwards, speed, rightTurn, turn
        Wire.beginTransmission(I2C_SLAVE);
        Wire.write((uint8_t)forwards);
        Wire.write((uint8_t)speed);
        Wire.write((uint8_t)rightTurn);
        Wire.write((uint8_t)turn);
        Wire.endTransmission();
    }

    // TODO: Consider error handling/feedback from car
    
    else if (!running){
        /* Do not send any useful data, but notify user on display */
        if (ctr > 200){
            char buf[50];
        
            disp.clearDisplay();
            disp.setCursor(12, 10);
            disp.setTextSize(3);
            disp.print("PAUSED");
        
            disp.setCursor(0, 40);
            disp.setTextSize(1);
            disp.print(" Press button to\n\r resume control");

            disp.invertDisplay(true);
            disp.display();

            /* Reset the counter */
            ctr = 0;
        }

        /* Send four empty bytes to stop car from moving */
        Wire.beginTransmission(I2C_SLAVE);
        Wire.write(0);
        Wire.write(0);
        Wire.write(0);
        Wire.write(0);
        Wire.endTransmission();
    }

    /* Wait until the loop has used 4 ms before proceeding */
    while (millis() - loopTimer < 4);
    elapsedTime = millis() - loopTimer; // elapsedTime is used for reading useful data
    loopTimer = millis();
}


/******************************************************************************************/


/* Set up the IMU for operation */
void setupIMU()
{
    /* Not sure about the details here, might look into that later */
    Serial.print("Activating IMU...");
    
    /* Activate the MPU6050 */
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);

    Serial.println("done!");

    return;
}


/* Read data from the IMU */
void _readIMU()
{
    int dummy;

    /* Read the raw gyro and accelerometer data */
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x3B);
    Wire.endTransmission(false);

    /* Request 14 bytes of data from the MPU-6050 */
    Wire.requestFrom(IMU_ADDRESS, 14, true);

    /* Wait until all bytes are received */
    while(Wire.available() < 14);

    /* Read 2 8-bit values into 1 16-bit value*/
    accRawX     = Wire.read()<<8 | Wire.read();
    accRawY     = Wire.read()<<8 | Wire.read();
    accRawZ     = Wire.read()<<8 | Wire.read();
    dummy       = Wire.read()<<8 | Wire.read(); // This is actually temperature data!
    gyroRawX    = Wire.read()<<8 | Wire.read();
    gyroRawY    = Wire.read()<<8 | Wire.read();
    gyroRawZ    = Wire.read()<<8 | Wire.read();

    return;
}


/* Read data and perform all the mathematics */
void getRefinedData()
{
    /* Ensure that fresh raw data is available */
    _readIMU();

    /* 131.0 apparently comes from the datasheet */
    gyroAngle[0] = gyroRawX / 131.0;
    gyroAngle[1] = gyroRawY / 131.0;

    /* Calculate pitch angle */
    accAngle[0] = atan((double)(accRawY/16384.0) / sqrt(pow((accRawX/16384.0), 2) + pow((accRawZ/16384.0), 2))) * 57.296;
    /* Calculate roll angle */
    accAngle[1] = atan(-(double)(accRawX/16384.0) / sqrt(pow((accRawY/16384.0), 2) + pow((accRawZ/16384.0), 2))) * 57.296;

    /* Finally, calculate "real" orientation */
    angle[0] = 0.98 * (angle[0] + gyroAngle[0] * (elapsedTime / 1000.0)) + 0.02 * accAngle[0];
    angle[1] = 0.98 * (angle[1] + gyroAngle[1] * (elapsedTime / 1000.0)) + 0.02 * accAngle[1];

    /* Store the data in the user-friendly variables. This is just up to preference and depends on the orientation of the unit */
    pitch = angle[1];
    roll  = angle[0];

    return;
}


/* Make a nice startup-and-wait sequence on the display */
void startupSequence(int delayDuration)
{
    char buf[50];

    sprintf(buf, "Controller starting,\n\rhold unit still...");
    
    for (int i = delayDuration; i > 0; i--){
        /* Display the countdown */
        disp.clearDisplay();
        disp.setTextSize(1);
        disp.setCursor(0, 0);
        disp.print(buf);
        disp.setTextSize(3);
        disp.setCursor(54, 30);
        disp.print(i);
        disp.display();
        
        delay(1000);
    }

    /* Return to more useful display settings */
    disp.clearDisplay();
    disp.setTextSize(1);

    return;
}


/* Draw an arrow on the display for user feedback purposes */
void drawArrow(int speed, bool forward, int turn, bool right)
{
    /* Cordinates for the bitmaps (top left corner placement on display) */
    const int x = 46;
    const int y = 10;

    /* Thresholds (copied from the CAR sketch's "deadzone" variables) */
    const int speedThreshold     = 60;
    const int turnThreshold      = 50;
    const int sharpTurnThreshold = 150;

    if (forward && speed > speedThreshold && turn < turnThreshold){
        /* Forward */
        disp.drawBitmap(x, y, forward_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
    }
    else if (!forward && speed > speedThreshold && turn < turnThreshold){
        /* Backwards */
        disp.drawBitmap(x, y, reverse_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
    }
    else if (forward && speed > speedThreshold && right && turn > turnThreshold){
        /* Forward, to the right */
        disp.drawBitmap(x+8, y, right_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
    }
    else if (right && turn > sharpTurnThreshold){
        /* Stationary right turn */
        disp.drawBitmap(x, y, east_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
    }
    else if (forward && speed > speedThreshold && !right && turn > turnThreshold){
        /* Forward, to the left */
        disp.drawBitmap(x-8, y, left_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
    }
    else if (!right && turn > sharpTurnThreshold){
        /* Stationary left turn */
        disp.drawBitmap(x, y, west_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
    }
    else {
        /* Car is not moving */
        disp.drawBitmap(x, y, stay_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
    }

    return;
}


/* Interrupt service routine */
IRAM_ATTR void btnCallback(void)
{
    /* Toggle the button flag */
    running = !running;

    return;
}
