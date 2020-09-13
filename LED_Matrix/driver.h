#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <Adafruit_NeoPixel.h>
#include "color.h"

/* Defines */
#define DM_HOR_RES    ( 10 )
#define DM_VER_RES    ( 10 )
#define DM_NUM_PX     ( DM_HOR_RES * DM_VER_RES )
#define DM_SIG_PIN    (  0 )
#define DM_BRIGHTNESS (  5 )


/* Set to 1 if the rows are connected "snake"-wise, 0 else */
#define DM_SNAKE 1

/* Typedefs */

typedef enum {
  STATE_MAIN,
  STATE_GARDEN,
  STATE_ERROR,
} STATE;

/* Variables */
Adafruit_NeoPixel matrix( DM_NUM_PX, DM_SIG_PIN, NEO_GRB + NEO_KHZ800 ); 


/* Global functions */
void DM_SetUpDisplay( void );
void DM_Clear( void );
void DM_SetPx( int x, int y, DM_Color col );
void DM_FillScreen( DM_Color col );
void DM_DrawBitMap( const unsigned char arr[], DM_Color fg, DM_Color bg );

void TiminiLogo( DM_Color fg, DM_Color bg = COLOR_BLACK );

/* Local functions */
void _setNeoPixel( int px, DM_Color col );

#endif
