#ifndef _RECIPES_H_
#define _RECIPES_H_

#include <Arduino.h>

// #define FILENAME      "recipe.txt"  // Filename of file with recipes on SD-card
#define NUMBER_OF_PUMPS    (   8 )
#define NUMBER_OF_RECIPES  (  12 )  // We can increase this when we get the Mega
#define MAX_INGREDIENTS    (   5 )  // We can increase this when we get the Mega
#define LONG_DRINK_VOLUME  ( 350 )  // mL

struct Pump {
    int     id;
    String  drink;
};

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