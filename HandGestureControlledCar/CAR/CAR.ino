/***********************************************
 * Vehicle application
 *
 * FILE:
 *      CAR.ino
 *
 * AUTHOR:
 *      Espen Hovland, April 2021
 *
 * HARDWARE:
 *      Arduino UNO R3
 *      L298N dual H-bridge motor driver
 *      HC595 shift register
 *
 * INFO:
 *      An ESP8266 ESP-01 is connected to the UNO
 *      via serial connection. See separate code.
 *
 **********************************************/

#include <SoftwareSerial.h>


//Shift register pins
#define LATCH_PIN   12  //RCLK
#define DATA_PIN    11  //SER
#define CLOCK_PIN   13  //SRCLK

// Motor H-bridge control pins
#define CTRL_A 9   //ENA, Right motor
#define CTRL_B 10  //ENB, Left motor

#define STATUS_LED_PIN A1
#define ESP_READY_PIN  A0

// Enum for the different motor combinations
enum class Direction
{
    forward,
    backward,
    left,
    right,
    sharp_left,
    sharp_right,
    stop
};

/* Variables for motor control */
Direction dir;
int speedLeftWheel;
int speedRightWheel;

/* Instructions received from the controller */
int  speed;
bool forwards;
int  turn;
bool rightTurn;

/* Data transmission-related bytes */
const uint8_t ack = 0x69;
const uint8_t end = 0x42;

/* Set up a software serial port for reading the data from the ESP8266 */
SoftwareSerial ESP8266(6, 7); // TXD-pin -> 6, RXD-pin -> 7

/* Local functions */
void setMotors(Direction dir);
void drive(Direction dir, int speedLeftMotor, int speedRightMotor);

/************************************************************************/

void setup()
{
    /* Start serial communications */
    Serial.begin(9600);
    Serial.println("Car connected");

    /* Shift register pins */
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    /* Motor control pins */
    pinMode(CTRL_A, OUTPUT);
    pinMode(CTRL_B, OUTPUT);

    /* Status LED pin */
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    /* Start the software-serial communication with the WiFi module */
    ESP8266.begin(38400);

    /* When WiFi-module is ready (pin is pulled LOW), proceed */
    while ( digitalRead(ESP_READY_PIN) != LOW );

    /* Wait for the controller to send the key */
    Serial.println("Waiting for key...");
    while (true) {
        if (ESP8266.available() == 6) {
            const uint8_t key[] = {ack, 1, 2, 3, 4, end};
            uint8_t buf[6];

            ESP8266.readBytes(buf, sizeof(buf));
            Serial.print(buf[0]);
            Serial.print(buf[1]);
            Serial.print(buf[2]);
            Serial.print(buf[3]);
            Serial.print(buf[4]);
            Serial.println(buf[5]);

            if (memcmp(key, buf, sizeof(key)) == 0){
                /* If the bytes received matches the key, then we are good to go! */
                break;
            }
        }
        delay(200);
    }

    /* Turn on status LED to signal that the car is ready */
    digitalWrite(STATUS_LED_PIN, HIGH);

    Serial.println("Car ready.");
}

void loop()
{
    /* Check for 6 incoming bytes */
    uint8_t buf[6];
    while (ESP8266.available() >=6 ){
        if (ESP8266.read() == ack){
            forwards  = (bool)(ESP8266.read());
            speed     =  (int)(ESP8266.read());
            rightTurn = (bool)(ESP8266.read());
            turn      =  (int)(ESP8266.read());
            if (ESP8266.read() == end){
                /* Proper data has been received */
                break;
            }
            else {
                continue;
            }
        }
    }

    /* Establish deadzones of the speed and turn variables */
    /* Note that speed and turn are in the interval [0,255] */
    /* These values must be adjusted */
    const int speedDeadzone         = 60;
    const int turnDeadzone          = 50;
    const int hardTurnLimit         = 200;
    const int mediumTurnLimit       = 150;
    const int manualOverrideSpeed   = 200;

    if (turn >= hardTurnLimit){
        // Sharp turn coming up!
        if (rightTurn){
            dir = Direction::sharp_right;
        }
        else {
            dir = Direction::sharp_left;
        }
        // Also set the turning speed manually in this case. Might need adjusting!
        // A relatively high speed gives faster turns
        speedLeftWheel  = manualOverrideSpeed;
        speedRightWheel = manualOverrideSpeed;
    }

    else if (turn >= mediumTurnLimit){
        //Medium turn coming up!
        if (rightTurn){
            dir = Direction::right;
        }
        else {
            dir = Direction::left;
        }
        // Also set the turning speed manually in this case. Might need adjusting!
        // A relatively high speed gives faster turns
        speedLeftWheel  = manualOverrideSpeed;
        speedRightWheel = manualOverrideSpeed;
    }

    else if (speed >= speedDeadzone) {
        // As long as we don't do a sharp or medium turn, we mostly go either forwards or backwards
        // We do slow turns by adjusting motor speed, instead of completely stopping a wheel
        if (forwards){
            // Drive forwards
            dir = Direction::forward;
        }
        else if (!forwards){
            // Drive backwards
            dir = Direction::backward;
        }

        // Initially, we can assume that we will drive straight...
        speedLeftWheel  = speed;
        speedRightWheel = speed;

        // ...but we must consider slow turns, this is done by adjusting the motor speeds
        // This part may need some thinking and adjustments!
        if (turn >= turnDeadzone){
            float turnEagerness = 1.1;
            if (rightTurn) {
                // The idea: Reduce the speed of the wheel as a function of the turn(roll) angle.
                // We will never actually reach 255 in this part of the code, but I think it's
                // nice and easy to relate to, if we keep it linear. Consider adjusting.
                speedRightWheel -= turnEagerness * map(turn, turnDeadzone, 255, 0, speedRightWheel);
            }
            else {
                speedLeftWheel -= turnEagerness *  map(turn, turnDeadzone, 255, 0, speedLeftWheel);
            }
        }
    }

    else {
        // Do not move the car
        dir = Direction::stop;
        speedLeftWheel  = 0;
        speedRightWheel = 0;
    }

    /* Ensure no odd speed values */
    speedLeftWheel  =  speedLeftWheel > 0 ?  speedLeftWheel : 0;
    speedRightWheel = speedRightWheel > 0 ? speedRightWheel : 0;

    // Finally, send the data to the motors
    drive(dir, speedLeftWheel, speedRightWheel);

    // We are done for the iteration.
}


/********************************************************************************************/

/* Set the direction and the speed of each motor */
void drive(Direction dir, int speedLeftMotor, int speedRightMotor)
{
    setMotors(dir);
    analogWrite(CTRL_A, speedRightMotor);
    analogWrite(CTRL_B, speedLeftMotor);

    return;
}


/* Sets the motors to the desired direction/mode*/
void setMotors(Direction dir)
{
    uint8_t data;

    /* data will take the form of a 8 bit number, 
     * with the following format:
     *  | MSB |     |     |     |     |     |     | LSB |
     *  |   Driving lights      | IN4 | IN3 | IN2 | IN1 |
     *  | RR  | RL  | FL  | FR  |   LEFT    |   RIGHT   |
     */

    switch (dir)
    {
    case Direction::forward:
        // Both motors move forward
        data = 0b00110101;
        break;

    case Direction::backward:
        // Both motors move backward
        data = 0b11001010;
        break;

    case Direction::left:
        // Right motor moves forward
        data = 0b00100001;
        break;

    case Direction::right:
        // Left motor moves forward
        data = 0b00010100;
        break;

    case Direction::sharp_left:
        // Left motor backward, right forward
        data = 0b01101001;
        break;

    case Direction::sharp_right:
        // Right motor backward, left forward
        data = 0b10010110;
        break;

    case Direction::stop:
        // Stop
        data = 0b11110000;
        break;
    default:
        // Do nothing
        data = 0b00000000;
        break;
    }

    // Write the data to the motor control pins via the shift register
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, data);
    digitalWrite(LATCH_PIN, HIGH);

    // We are done
    return;
}