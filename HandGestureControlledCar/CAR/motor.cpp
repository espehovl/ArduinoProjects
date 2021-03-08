#include "Arduino.h"

#include "motor.h"
#include "TFY4190_CAR.h"


/* Set the direction and the speed of each motor */
void drive(Direction dir, int speedLeftMotor, int speedRightMotor)
{
    setMotors(dir);
    analogWrite(CTRL_A, speedRightMotor);
    analogWrite(CTRL_B, speedLeftMotor);

    //TODO: Vurder å legge inn en delay (noen ms) her, men det må man se hvordan det blir med den faktiske kontrollen og kommunikasjonen
}


/* Sets the motors to the desired direction/mode*/
void setMotors(Direction dir)
{
    uint8_t data;

    /* data will take the form of a 8 bit number, 
     * with the following format:
     *  | MSB |     |     |     |     |     |     | LSB |
     *  |     Not used          | IN4 | IN3 | IN2 | IN1 |
     *  |                       |   LEFT    |   RIGHT   |
     */

    switch (dir)
    {
    case Direction::forward:
        // Both motors move forward
        data = 0b0101;
        break;
    
    case Direction::backward:
        // Both motors move backward
        data = 0b1010;
        break;
    
    case Direction::left:
        // Right motor moves forward
        data = 0b0001;
        break;

    case Direction::right:
        // Left motor moves forward
        data = 0b0100;
        break;
    
    case Direction::sharp_left:
        // Left motor backward, right forward
        data = 0b1001;
        break;

    case Direction::sharp_right:
        // Right motor backward, left forward
        data = 0b0110;
        break;

    default:
        // Do nothing
        data = 0b0000;
        break;
    }

    // Write the data to the motor control pins via the shift register
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, data);
    digitalWrite(LATCH_PIN, HIGH);

    // We are done
    return;
}