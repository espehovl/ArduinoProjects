#ifndef _TFY4190_CAR_H_
#define _TFY4190_CAR_H_

//Shift register pins
#define LATCH_PIN   12  //RCLK
#define DATA_PIN    11  //SER
#define CLOCK_PIN   13  //SRCLK

// Motor H-bridge control pins
#define CTRL_A 9   //ENA, Right motor
#define CTRL_B 10  //ENB, Left motor

// Status LED pin
#define STATUS_LED_PIN A1

#endif