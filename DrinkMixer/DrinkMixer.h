#ifndef _DRINKMIXER_H_
#define _DRINKMIXER_H_


#define COMMS_INIT_OK           (  0x00 ) // Comms are set up and OK
#define COMMS_IP_FLAG           (  0x01 ) // IP address incoming
#define COMMS_RECIPE_FLAG       (  0x02 ) // Recipes are incoming

#define COMMS_IP_ACK            (  0xFF ) // IP address recieved
#define COMMS_RECIPE_ACK        (  0xFE ) // Recipes received

#define COMMS_BEGIN_ORDER       (  0xFD ) // Order transfer begin
#define COMMS_END_ORDER         (  0xFC ) // Order transfer complete

#define ROT_ENC_DAT_PIN         (    3 ) // (S1) Interrupt-enabled pin
#define ROT_ENC_CLK_PIN         (    4 ) // (S2)
#define ROT_ENC_BTN_PIN         (    2 ) // Interrupt-enabled pin

#define ORDER_NOTIFY_PIN        (   19 ) // Interrupt-enabled pin
#define CLEAN_NOTIFY_PIN        (   18 ) // Interrupt-enabled pin

#define LOADCELL_DATA_PIN       (    8 )
#define LOADCELL_SCLK_PIN       (    9 )

#define LCD_I2C_ADDRESS         ( 0x27 )
#define LCD_COLS                (   20 )
#define LCD_ROWS                (    4 )

#define SR_RELAY_LATCH_PIN      (    6 ) // RCLK
#define SR_RELAY_DATA_PIN       (    5 ) // SER
#define SR_RELAY_CLOCK_PIN      (    7 ) // SRCLK

#define LED_STRIP_DATA_PIN      (   10 ) // TODO:
#define LED_STRIP_NUM_LEDS      (    0 ) // TODO:

/*  SPI pins:
    SCK:  52
    MISO: 50
    MOSI: 51
    CS:   53  */
#define SPI_CS_PIN              (   53 )

#define MAX_FILENAME_LENGTH     (   30 )


#endif