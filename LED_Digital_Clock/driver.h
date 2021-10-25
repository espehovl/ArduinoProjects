#ifndef _DRIVER_H_
#define _DRIVER_H_

#include "color.h"
#include "Adafruit_NeoPixel.h"

/* Defines */
#define DM_HOR_RES    ( 10 )
#define DM_VER_RES    ( 10 )
#define DM_NUM_PX     ( DM_HOR_RES * DM_VER_RES )
#define DM_SIG_PIN    ( D4 )
#define DM_BRIGHTNESS ( 50 )

/* NeoPixel object representing the LED matrix */
Adafruit_NeoPixel matrix( DM_NUM_PX, DM_SIG_PIN, NEO_GRB + NEO_KHZ800 ); 

/* Set to 1 if the rows are connected "snake"-wise, 0 else */
#define DM_SNAKE 1


typedef struct {
  char* sym;
  DM_Color color;
} COLOR_MAP;


void DM_SetUpDisplay    ( void );
void DM_Clear           ( void );
void DM_Show            ( void );
void DM_SetBrightness   ( byte brightness );
void DM_SetPx           ( int x, int y, DM_Color col );
void DM_FillScreen      ( DM_Color col );
void DM_DrawBitMap      ( const unsigned char arr[], DM_Color fg, DM_Color bg );
void DM_DrawColorImage  ( char* arr[] );

void _setNeoPixel( int px, DM_Color col );

#endif
