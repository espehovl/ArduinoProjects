/***********************************************
 * Vehicle receiver application
 *
 * FILE:
 *      ESP8266.ino
 *
 * AUTHOR:
 *      Espen Hovland, April 2021
 *
 * HARDWARE:
 *      ESP8266 ESP-01
 *
 * INFO:
 *      Code for receiving data from the glove
 *      and forwarding it to the vehicle.
 *
 **********************************************/

#include <ESP8266WiFi.h>
#include <espnow.h>

#define USE_SERIAL Serial

void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength);

void setup()
{
    USE_SERIAL.begin(38400);

    USE_SERIAL.println(WiFi.macAddress());

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    pinMode(LED_BUILTIN, OUTPUT);

    if (esp_now_init() != 0){
        // Init failed!
        digitalWrite(LED_BUILTIN, HIGH); // Do not light the LED
        return;
    }

    esp_now_register_recv_cb(dataReceived);

    digitalWrite(LED_BUILTIN, LOW); // Turn on the status LED if successful
}

void loop()
{
    /* That's all, folks! */
}

void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength)
{
    /* Data is received, send it to the car driver */
    /* Data is received as: ack, forwards, speed, rightTurn, turn, end */

    /* No need for this */
    (void)senderMac;

    uint8_t ack = 0x69;
    uint8_t end = 0x42;

    /* 6 bytes are expected, pass them along to the serial comms. */
    if (dataLength == 6 && data[0] == ack && data[5] == end){
        USE_SERIAL.write(data[0]);
        USE_SERIAL.write(data[1]);
        USE_SERIAL.write(data[2]);
        USE_SERIAL.write(data[3]);
        USE_SERIAL.write(data[4]);
        USE_SERIAL.write(data[5]);
    }

    return;
}