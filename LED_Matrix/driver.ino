/* The driver for the dot matrix LED display. */

#include <Adafruit_NeoPixel.h>

#include "driver.h"
#include "color.h"


/* Global functions */
void DM_SetUpDisplay() 
{
  matrix.begin();
  matrix.setBrightness( DM_BRIGHTNESS );
}


void DM_Clear( void ) 
{
  //DM_FillScreen( COLOR_BLACK );
  matrix.clear();
  matrix.show();
}


void DM_SetPx( int x, int y, DM_Color col )
{
  /* Check the parameters */
  if ( x > DM_HOR_RES || x < 0 || y > DM_VER_RES || y < 0 ) {
    /* Invalid coordinates */
    goto wrapUp;
  }
  
  #if ( DM_SNAKE == 1 )
    /* Every other row must be corrected for */
    if ( y % 2 == 0 ) {
      /* Do nothing special */
      _setNeoPixel( y * DM_HOR_RES + x, col );
    }
    else if ( y % 2 != 0 ) {
      _setNeoPixel( y * DM_HOR_RES + ( DM_HOR_RES - 1 - x ), col );
    }
  #else
    /* Do nothing with the coordinates */
    _setNeoPixel( y * DM_HOR_RES + x, col );
  #endif

wrapUp:
  // Finalize
  //matrix.show();
  return;
}


void DM_FillScreen( DM_Color col )
{
  for ( int i = 0; i < DM_NUM_PX; i++ ) {
    _setNeoPixel( i, col );
  }

  // Finalize
  matrix.show();
}


void DM_DrawBitMap( const unsigned char arr[], DM_Color fg, DM_Color bg )
{
  int hor_res = DM_HOR_RES;
  int ver_res = DM_VER_RES;
  
  DM_FillScreen( bg );
  
  int col = 0;
  int row = 0;
  for ( int i = 0; i < 20; i++ ){
    // For each byte in the bitmap...
    for ( int j = 0; j < 8; j++ ){
      // For each bit in the byte...
      if ( arr[i] & ( 1 << j ) ){
        // If a pixel should be black, make it black
        DM_SetPx( col, row, fg );
      }
      col++;
      if ( col >= hor_res ) {
        // If we have gone through all the pixels in the row, reset the column
        // number and increment the row number. Ignore the rest of the bits
        // in the byte.
        col = 0;
        row++;
        break;
      }
    }
    if ( col == 0 ) {
      // Start fresh on a new row
      continue;
    }
  }

  // Finalize
  matrix.show();
}

void TiminiLogo( DM_Color fg, DM_Color bg )
 {
  DM_DrawBitMap( timini_bits, fg, bg );
}

/* Local functions */
void _setNeoPixel( int px, DM_Color col ) {
  matrix.setPixelColor( px, matrix.Color(col.r, col.g, col.b));
  
}
