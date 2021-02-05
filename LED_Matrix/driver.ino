/* The driver for the dot matrix LED display. */
/* KNOWN ISSUES:
 *  When we are dealing with more than ~90 colors, 
 *  (i.e. when each symbol is 2+ characters wide)
 *  the program does not work properly. This might be due
 *  to some issues when copying pointers and such.
 *  Might be a quick fix, might not be.
 */


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
    /* This commented section did nothing when it wasn't commented either */
//    if ( col == 0 ) {
//      // Start fresh on a new row
//      continue;
//    }
  }

  // Finalize
  matrix.show();
}


void DM_DrawColorImage( char* arr[] )
{
  /* Read xpm-array and display the image */
  byte hor_res;
  byte ver_res;
  byte colors;
  byte char_per_pixel;
  char raw_data[20];

  //DM_Clear();
  
  memset( raw_data, 0, sizeof(raw_data) );
  /* Parse the values message */
  char* delim = " ";
  char* token = strtok( (char*) arr[0], delim );
  hor_res = atoi(token);
  token = strtok( NULL, delim );
  ver_res = atoi(token);
  token = strtok( NULL, delim );
  colors = atoi(token);
  token = strtok( NULL, delim );
  char_per_pixel = atoi(token);

//  Serial.println(hor_res);
//  Serial.println(ver_res);
//  Serial.println(colors);
//  Serial.println(char_per_pixel);

  /* Establish the colors in a data structure */
  COLOR_MAP cols[colors];
  char symb[char_per_pixel];
  char* hex_code;

  for ( int i = 0; i < colors; i++ ) {
    //Serial.println(arr[i+1]);
    strncpy( raw_data, arr[i+1], sizeof(raw_data) );
    //raw_data[sizeof(raw_data)] = '\0'; 
    //Serial.print("Data size: ");
    //Serial.println(sizeof(raw_data));
    //Serial.println(raw_data);
    
    /* Fetch the symbol */
    //memset( symb, 0, char_per_pixel );
    memcpy( symb, raw_data, char_per_pixel );
    symb[char_per_pixel] = '\0';
//    Serial.print("Symbol '");
//    Serial.print(symb);
//    Serial.println("' ");
    
    /* Fetch the hex code */
    delim = "#";
    token = strtok( (char*) raw_data, delim );
    /* The first value is useless */
    hex_code = strtok( NULL, delim );

//    Serial.print("Hex: ");
//    Serial.println(hex_code);

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
//      Serial.print("'");
//      Serial.print(symbol);
//      Serial.println("'");
      char dummy[20];
//        Serial.print("'");
//        Serial.print(symbol);
//        Serial.println("'");
      for ( int i = 0; i < colors; i++ ) {
//        Serial.print("'");
//        Serial.print(cols[i].sym);
//        Serial.println("'");
        if ( strncmp( cols[i].sym, symbol, char_per_pixel ) == 0 ) {
          DM_SetPx( col, row, cols[i].color );
//          sprintf( dummy, "R:%d, G:%d, B:%d \n\r x:%d, y:%d", cols[i].color.r, cols[i].color.g, cols[i].color.b, col, row );
//          Serial.println(dummy);
        }
      }
    }
  }
  
  // Finalize
  
  matrix.show();
}



/* Local functions */
void TiminiLogo( DM_Color fg, DM_Color bg )
 {
  DM_DrawBitMap( timini_bits, fg, bg );
}


void _setNeoPixel( int px, DM_Color col ) {
  matrix.setPixelColor( px, matrix.Color(col.r, col.g, col.b));
}
