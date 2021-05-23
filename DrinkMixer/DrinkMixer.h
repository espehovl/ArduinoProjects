#ifndef _DRINKMIXER_H_
#define _DRINKMIXER_H_

/* Pin defines */

// TODO: Update these pins when Mega arrives!
#define LOADCELL_DATA_PIN       (    8 )
#define LOADCELL_SCLK_PIN       (    9 )

#define LCD_I2C_ADDRESS         ( 0x27 )
#define LCD_COLS                (   20 )
#define LCD_ROWS                (    4 )

#define SR_RELAY_LATCH_PIN      (    6 ) // RCLK
#define SR_RELAY_DATA_PIN       (    5 ) // SER
#define SR_RELAY_CLOCK_PIN      (    7 ) // SRCLK

/*  SPI pins:
    SCK:  13
    MISO: 12
    MOSI: 11
    CS:   10  */
#define SPI_CS_PIN 10

#endif