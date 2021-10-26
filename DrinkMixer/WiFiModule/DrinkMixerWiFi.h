#ifndef _DRINKMIXER_WIFI_H_
#define _DRINKMIXER_WIFI_H_

#include "../Recipes.h"
#include "../DrinkMixer.h"

#define NOTIFY_PIN (  2 )

struct WebRecipe {
    char name[MAX_STRING_LENGTH];
    uint8_t num_ingredients;
    char ingredients[MAX_INGREDIENTS][MAX_STRING_LENGTH];
};

#endif