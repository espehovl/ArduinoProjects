#ifndef _MOTOR_H_
#define _MOTOR_H_


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

void setMotors(Direction dir);

void drive(Direction dir, int speedLeftMotor, int speedRightMotor);

#endif