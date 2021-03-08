

#include "TFY4190_CAR.h"
#include "motor.h"


void setup()
{
    Serial.begin(9600);
    Serial.println("Car connected");

    pinMode(LATCH_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    
    pinMode(CTRL_A, OUTPUT);
    pinMode(CTRL_B, OUTPUT);

    /* For potmeter reading */
    pinMode(A0, INPUT);
}

void loop()
{
    int velocity;
    // Test the motors

    for (unsigned int i = 0; i < 6; i++)
    {
        velocity = analogRead(A0);
        Serial.println(velocity);
        drive(static_cast<Direction>(i), velocity, velocity);
        delay(1000);
    }

}
