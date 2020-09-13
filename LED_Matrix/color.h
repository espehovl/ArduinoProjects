#ifndef _COLOR_H_
#define _COLOR_H_

#define COLOR_RED       ( (DM_Color){ 255,   0,   0 } )
#define COLOR_GREEN     ( (DM_Color){   0, 255,   0 } )
#define COLOR_BLUE      ( (DM_Color){   0,   0, 255 } )

#define COLOR_WHITE     ( (DM_Color){ 255, 255, 255 } )
#define COLOR_BLACK     ( (DM_Color){   0,   0,   0 } )

#define COLOR_FJELLBEKK ( (DM_Color){  57, 154, 213 } )
#define COLOR_YELLOW    ( (DM_Color){ 255, 255,   0 } )

/* Types */
typedef struct {
  int r;
  int g;
  int b;
} DM_Color;

#endif
