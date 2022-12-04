#ifndef _DRINKMIXER_H_
#define _DRINKMIXER_H_


#define COMMS_INIT_OK           (  0x00 ) // Comms are set up and OK
#define COMMS_IP_FLAG           (  0x01 ) // IP address incoming
#define COMMS_RECIPE_FLAG       (  0x02 ) // Recipes are incoming
#define COMMS_SSID_FLAG         (  0x03 ) // SSID is incoming
#define COMMS_PASS_FLAG         (  0x04 ) // Password is incoming

#define COMMS_IP_ACK            (  0xFF ) // IP address recieved
#define COMMS_RECIPE_ACK        (  0xFE ) // Recipes received
#define COMMS_SSID_ACK          (  0xFD ) // SSID received
#define COMMS_PASS_ACK          (  0xFC ) // Password received

#define COMMS_BEGIN_ORDER       (  0xFD ) // Order transfer begin
#define COMMS_END_ORDER         (  0xFC ) // Order transfer complete

#define ROT_ENC_BTN_PIN         (    2 ) // Interrupt-enabled pin
#define ROT_ENC_DAT_PIN         (    3 ) // (S1) Interrupt-enabled pin
#define ROT_ENC_CLK_PIN         (    4 ) // (S2)

#define SR_RELAY_DATA_PIN       (    5 ) // SER
#define SR_RELAY_LATCH_PIN      (    6 ) // RCLK
#define SR_RELAY_CLOCK_PIN      (    7 ) // SRCLK

#define LOADCELL_DATA_PIN       (    8 )
#define LOADCELL_SCLK_PIN       (    9 )

#define CLEAN_NOTIFY_PIN        (   18 ) // Interrupt-enabled pin
#define ORDER_NOTIFY_PIN        (   19 ) // Interrupt-enabled pin

#define LCD_I2C_ADDRESS         ( 0x27 )
#define LCD_COLS                (   20 )
#define LCD_ROWS                (    4 )

#define LED_STRIP_DATA_PIN      (   10 )
#define LED_STRIP_NUM_LEDS      (   52 )
#define LED_STRIP_BOTTLE_LEDS   (    8 )
#define LED_STRIP_PARTY_LIGHTS  (    8 ) // Start index of party lights
#define LED_STRIP_BRIGHTNESS    (   200 )

/*  SPI pins:
    SCK:  52
    MISO: 50
    MOSI: 51
    CS:   53  */
#define SPI_CS_PIN              (   53 )

#define MAX_FILENAME_LENGTH     (   30 )




// Pump number to LED number translation
const uint8_t pump_LED_to_ID[] = {3, 2, 1, 0, 4, 5, 6, 7};

#endif