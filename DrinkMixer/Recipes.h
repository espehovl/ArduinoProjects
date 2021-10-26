#ifndef _RECIPES_H_
#define _RECIPES_H_

#include <Arduino.h>

// #define FILENAME      "recipe.txt"  // Filename of file with recipes on SD-card
#define NUMBER_OF_PUMPS    (   8 )
#define NUMBER_OF_RECIPES  (  12 )
#define MAX_INGREDIENTS    (   5 )
#define LONG_DRINK_VOLUME  ( 350 )  // mL
#define MAX_STRING_LENGTH  (  21 )  // The display is 20 characters wide

struct Pump {
    int  id;
    char drink[MAX_STRING_LENGTH];
};

struct Ingredient {
    char beverage[MAX_STRING_LENGTH];
    int  volume;                      // Amount in mL
};

struct Recipe{
    char        name[MAX_STRING_LENGTH];
    Ingredient  ingredients[MAX_INGREDIENTS]; // Hard code hallelujah
    int         num_ingredients;
};

#endif