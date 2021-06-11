#ifndef _DRINKMIXER_H_
#define _DRINKMIXER_H_


/* Defines for serial communications bytes */
#define COMMS_INIT_OK       (  0x00 ) // Comms are set up and OK
#define COMMS_IP_FLAG       (  0x01 ) // IP address incoming
#define COMMS_RECIPE_FLAG   (  0x02 ) // Recipes are incoming

#define COMMS_IP_ACK        (  0xFF ) // IP address recieved
#define COMMS_RECIPE_ACK    (  0xFE ) // Recipes received

/* Pin defines */

// TODO: Add defines for the interrupt-requiring pins:
// 3 for the rotary encoder
// Any additional buttons?


#define LOADCELL_DATA_PIN       (    8 )
#define LOADCELL_SCLK_PIN       (    9 )

#define LCD_I2C_ADDRESS         ( 0x27 )
#define LCD_COLS                (   20 )
#define LCD_ROWS                (    4 )

#define SR_RELAY_LATCH_PIN      (    6 ) // RCLK
#define SR_RELAY_DATA_PIN       (    5 ) // SER
#define SR_RELAY_CLOCK_PIN      (    7 ) // SRCLK

/*  SPI pins:
    SCK:  52
    MISO: 50
    MOSI: 51
    CS:   53  */
#define SPI_CS_PIN 53

#define NUM_DIRECTORY_FILES     (    3 ) // Number of directory files to display
#define MAX_FILENAME_LENGTH     (   30 )
// void SRSendCommand(uint8_t *data, int len);

#endif