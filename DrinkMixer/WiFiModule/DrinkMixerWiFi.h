#ifndef _DRINKMIXER_WIFI_H_
#define _DRINKMIXER_WIFI_H_

// These must match those of "Recipes.h" from the Mega
#define NUMBER_OF_RECIPES   (  12 )
#define MAX_INGREDIENTS     (   5 )

/* Defines for serial communications bytes */
#define COMMS_INIT_OK       (  0x00 ) // Comms are set up and OK
#define COMMS_IP_FLAG       (  0x01 ) // IP address incoming
#define COMMS_RECIPE_FLAG   (  0x02 ) // Recipes are incoming

#define COMMS_IP_ACK        (  0xFF ) // IP address recieved
#define COMMS_RECIPE_ACK    (  0xFE ) // Recipes received


// Don't think we need this one
// struct Pump {
//     int     id;
//     String  drink;
// };

struct Ingredient {
    String beverage;
    int    volume;     // Amount in mL
};

struct Recipe{
    String      name;
    Ingredient  ingredients[MAX_INGREDIENTS]; // Hard code hallelujah
    int         num_ingredients;
};

#endif