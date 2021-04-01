
#include "Wire.h"

#include "TFY4190_CAR.h"
#include "motor.h"

#define I2C_SLAVE ( 0x14 )

Direction dir;
int speedLeftWheel;
int speedRightWheel;
int speed;
bool forwards;
int turn;
bool rightTurn;

void receiveEvent();

void setup()
{
    Serial.begin(9600);
    Serial.println("Car connected");

    /* Set up I2C communications */ 
    Wire.begin(I2C_SLAVE);
    Wire.onReceive(receiveEvent);

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

    /* TODO: Set up wireless communications and ensure proper connection */
    //...

    //Dummy delay for simulation setup of comms:
    for (int i = 5; i > 0; i--){
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(100);
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(900);
    }

    /* Comms are set up, notify by lighting up status LED, and proceed with the main loop */
    digitalWrite(STATUS_LED_PIN, HIGH);
}

void loop()
{
    // Receive the signal from the controller
    //TODO: *this

    
    // Interpret the signal from the controller
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
        // We do slow turns by adjust motor speed, instead of completely stopping a wheel
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
        // TODO: This part may need some thinking and adjustments!

        if (turn >= turnDeadzone){
            float turnEagerness = 1.0;
            if (rightTurn) {
                // The idea: Reduce the speed of the wheel, as a function of the turn(roll) angle.
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

    char buf[90];
    sprintf(buf, "Turn: %d, speedright: %d, speedleft: %d", turn, speedRightWheel, speedLeftWheel);
    Serial.println(buf);

    /* Ensure no odd speed values */
    speedLeftWheel  =  speedLeftWheel > 0 ?  speedLeftWheel : 0;
    speedRightWheel = speedRightWheel > 0 ? speedRightWheel : 0;

    // Finally, send the data to the motors
    drive(dir, speedLeftWheel, speedRightWheel);

    // We are done for the iteration.
}

void receiveEvent()
{
    if (Wire.available() == 4){
        forwards    = Wire.read();
        speed       = Wire.read();
        rightTurn   = Wire.read();
        turn        = Wire.read();
    }

    return;
}
