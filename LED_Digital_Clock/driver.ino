/* The driver for the dot matrix LED display. */

#include "driver.h"
#include "Adafruit_NeoPixel.h"


/***********************************************
 * @brief Initialize display and set brightness.
***********************************************/
void DM_SetUpDisplay() 
{
  matrix.begin();
  matrix.setBrightness( DM_BRIGHTNESS );
  return;
}


/***********************************************
 * @brief Clear the display.
***********************************************/
void DM_Clear( void ) 
{
  matrix.clear();
  matrix.show();
  return;
}


/***********************************************
 * @brief Show whatever image currently in the 
 *        LED strip memory.
***********************************************/
void DM_Show( void )
{
  matrix.show();
  return;
}


/***********************************************
 * @brief Set the brightness to a custom value,
 *        ranging from 0-255.
***********************************************/
void DM_SetBrightness(byte brightness){
  matrix.setBrightness(brightness);
  return;
}


/***********************************************
 * @brief Set the color of the pixel at (x, y)
 *        (zero-indexed).
 * @param x   int representing the x coordinate.
 * @param y   int representing the y coordinate.
 * @param col Desired pixel color
***********************************************/
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
  return;
}


/***********************************************
 * @brief Fill the entire display with one color.
 * @param col The color to fill the display with.
***********************************************/
void DM_FillScreen( DM_Color col )
{
  for ( int i = 0; i < DM_NUM_PX; i++ ) {
    _setNeoPixel( i, col );
  }

  return;
}


/***********************************************
 * @brief Draw a bitmap to the display.
 * @param arr The bitmap to be drawn.
 * @param fg  Foreground color.
 * @param bg  Background color.
***********************************************/
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
  }

  // Finalize
  DM_Show();
  return;
}

/***********************************************
 * @brief Draw an XPM color image to the display.
 * @param arr XPM array to draw.
***********************************************/
void DM_DrawColorImage( char* arr[] )
{
  /* Read xpm-array and display the image */
  byte hor_res;
  byte ver_res;
  byte colors;
  byte char_per_pixel;
  char raw_data[20];

  memset( raw_data, 0, sizeof(raw_data) );
  /* Parse the values message */
  char* delim    = " ";
  char* token    = strtok( (char*) arr[0], delim );
  hor_res        = atoi(token);
  token          = strtok( NULL, delim );
  ver_res        = atoi(token);
  token          = strtok( NULL, delim );
  colors         = atoi(token);
  token          = strtok( NULL, delim );
  char_per_pixel = atoi(token);

  /* Establish the colors in a data structure */
  COLOR_MAP cols[colors];
  char symb[char_per_pixel];
  char* hex_code;

  for ( int i = 0; i < colors; i++ ) {
    strncpy( raw_data, arr[i+1], sizeof(raw_data) );

    /* Fetch the symbol */
    memcpy( symb, raw_data, char_per_pixel );
    symb[char_per_pixel] = '\0';

    /* Fetch the hex code */
    delim = "#";
    token = strtok( (char*) raw_data, delim );

    /* The first value is useless */
    hex_code = strtok( NULL, delim );

    /* Parse the hex code into RGB */
    char red_channel[] = "0x00";
    red_channel[2] = hex_code[0];
    red_channel[3] = hex_code[1];

    char green_channel[] = "0x00";
    green_channel[2] = hex_code[2];
    green_channel[3] = hex_code[3];

    char blue_channel[] = "0x00";
    blue_channel[2] = hex_code[4];
    blue_channel[3] = hex_code[5];

    /* Add entry to the dictionary */
    cols[i] = (COLOR_MAP){new char(*symb), (DM_Color){ strtol(red_channel, NULL, 0), strtol(green_channel, NULL, 0), strtol(blue_channel, NULL, 0)  } };
  }

  /* Draw the actual image */
  char symbol[char_per_pixel];
  for ( int row = 0; row < ver_res; row++ ) {
    for ( int col = 0; col < hor_res; col += char_per_pixel ) {
      for ( int s = 0; s < char_per_pixel; s++ ) {
        symbol[s] = arr[1 + colors + row][col + s];
      }
      symbol[char_per_pixel] = '\0';
      char dummy[20];
      for ( int i = 0; i < colors; i++ ) {
        if ( strncmp( cols[i].sym, symbol, char_per_pixel ) == 0 ) {
          DM_SetPx( col, row, cols[i].color );
        }
      }
    }
  }

  // Finalize
  DM_Show();
  return;
}


/***********************************************
 * @brief Set an individual pixel to a color.
 *        This is just a wrapper, replace with
 *        the LED strip library of your choice.
 * @param px  The pixel to light up.
 * @param col The color to draw.
***********************************************/
void _setNeoPixel( int px, DM_Color col ) {
  matrix.setPixelColor( px, matrix.Color(col.r, col.g, col.b));
}
