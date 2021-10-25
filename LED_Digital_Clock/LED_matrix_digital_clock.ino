/***********************************************************************
 * @file
 *       LED_matrix_digital_clock.ino
 * @author
 *       Espen Hovland (espehovl@github.com)
 * @details
 *       LED matrix digital clock application. Requires the Adafruit
 *       NeoPixel library (or you can use other similar libraries),
 *       and the DS3231 library from Eric Ayars.
 *
 **********************************************************************/

/* This file requires the Adafruit NeoPixel library! */
#include <DS3231.h>
#include <Wire.h>

#include "driver.h"

/* Pin defines for the NodeMCU V3 (ESP8266) */
#define SDA_PIN D2
#define SCL_PIN D1

#define NIGHT_MODE_START 20
#define NIGHT_MODE_END    8

/************************************************************************/

DS3231 RTC{};
bool h12flag, PMflag;
byte lastSecond, lastMinute, lastHour;
bool secondIndicator = false;

void drawSecondIndicator(bool showIndicator, DM_Color color, DM_Color background);
void drawHours(byte h, DM_Color color);
void drawMinutes(byte m, DM_Color color);
void drawNumber(byte tl_x, byte tl_y, byte num, DM_Color color);

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  DM_SetUpDisplay();

  RTC.setClockMode(false);

  lastSecond = RTC.getSecond();
  lastMinute = RTC.getMinute();
  lastHour = RTC.getHour(h12flag, PMflag);

  DM_FillScreen(COLOR_WHITE);
  drawSecondIndicator(secondIndicator, COLOR_BLACK);
  drawMinutes(lastMinute, COLOR_RED);
  drawHours(lastHour, COLOR_BLACK);
  DM_Show();
}

void loop()
{
  if (RTC.getSecond() != lastSecond)
  {
    DM_FillScreen(COLOR_BLACK);
    drawSecondIndicator(secondIndicator, COLOR_BLACK);
    drawMinutes(RTC.getMinute(), COLOR_RED);
    drawHours(RTC.getHour(h12flag, PMflag), COLOR_WHITE);

    lastSecond = RTC.getSecond();
    secondIndicator = !secondIndicator;

    if (RTC.getMinute() != lastMinute){
      lastMinute = RTC.getMinute();
    }

    if (RTC.getHour(h12flag, PMflag) != lastHour){
      lastHour = RTC.getHour(h12flag, PMflag);

      if (lastHour >= NIGHT_MODE_START || lastHour <= NIGHT_MODE_END){
        DM_SetBrightness(5); // Enter night mode
      }

      else {
        DM_SetBrightness(DM_BRIGHTNESS);
      }
    }
    DM_Show();
  }
}

void drawSecondIndicator(bool showIndicator, DM_Color color, DM_Color background = COLOR_WHITE)
{
  if (showIndicator)
  {
    DM_SetPx(8, 1, background);
    DM_SetPx(8, 3, background);
  }
  else
  {
    DM_SetPx(8, 1, color);
    DM_SetPx(8, 3, color);
  }
}

void drawHours(byte h, DM_Color color)
{
  /* Most significant digit */
  drawNumber(0, 0, h / 10, color);
  /* Least significant digit */
  drawNumber(4, 0, h % 10, color);
}

void drawMinutes(byte m, DM_Color color)
{
  /* Most significant digit */
  drawNumber(2, 5, m / 10, color);
  /* Least significant digit */
  drawNumber(6, 5, m % 10, color);
}

void drawNumber(byte tl_x, byte tl_y, byte num, DM_Color color)
{
  switch (num)
  {
  case 8:
    /* 8 is simply 0 plus 1 pixel */
    DM_SetPx(tl_x + 1, tl_y + 2, color);
  case 0:
    DM_SetPx(tl_x + 0, tl_y + 0, color);
    DM_SetPx(tl_x + 1, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 1, color);
    DM_SetPx(tl_x + 2, tl_y + 2, color);
    DM_SetPx(tl_x + 2, tl_y + 3, color);
    DM_SetPx(tl_x + 2, tl_y + 4, color);
    DM_SetPx(tl_x + 1, tl_y + 4, color);
    DM_SetPx(tl_x + 0, tl_y + 4, color);
    DM_SetPx(tl_x + 0, tl_y + 3, color);
    DM_SetPx(tl_x + 0, tl_y + 2, color);
    DM_SetPx(tl_x + 0, tl_y + 1, color);
    break;
  case 7:
    /* 7 is simply 1 plus 1 pixel */
    DM_SetPx(tl_x + 0, tl_y + 0, color);
  case 1:
    DM_SetPx(tl_x + 1, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 1, color);
    DM_SetPx(tl_x + 2, tl_y + 2, color);
    DM_SetPx(tl_x + 2, tl_y + 3, color);
    DM_SetPx(tl_x + 2, tl_y + 4, color);
    break;
  case 2:
    DM_SetPx(tl_x + 0, tl_y + 0, color);
    DM_SetPx(tl_x + 1, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 1, color);
    DM_SetPx(tl_x + 2, tl_y + 2, color);
    DM_SetPx(tl_x + 2, tl_y + 4, color);
    DM_SetPx(tl_x + 1, tl_y + 4, color);
    DM_SetPx(tl_x + 0, tl_y + 4, color);
    DM_SetPx(tl_x + 0, tl_y + 3, color);
    DM_SetPx(tl_x + 0, tl_y + 2, color);
    DM_SetPx(tl_x + 1, tl_y + 2, color);
    break;
  case 9:
    /* 9 is simply 3 plus two pixels */
    DM_SetPx(tl_x + 0, tl_y + 2, color);
    DM_SetPx(tl_x + 0, tl_y + 1, color);
  case 3:
    DM_SetPx(tl_x + 0, tl_y + 0, color);
    DM_SetPx(tl_x + 1, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 1, color);
    DM_SetPx(tl_x + 2, tl_y + 2, color);
    DM_SetPx(tl_x + 2, tl_y + 3, color);
    DM_SetPx(tl_x + 2, tl_y + 4, color);
    DM_SetPx(tl_x + 1, tl_y + 4, color);
    DM_SetPx(tl_x + 0, tl_y + 4, color);
    DM_SetPx(tl_x + 1, tl_y + 2, color);
    break;
  case 4:
    DM_SetPx(tl_x + 0, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 1, color);
    DM_SetPx(tl_x + 2, tl_y + 2, color);
    DM_SetPx(tl_x + 2, tl_y + 3, color);
    DM_SetPx(tl_x + 2, tl_y + 4, color);
    DM_SetPx(tl_x + 0, tl_y + 2, color);
    DM_SetPx(tl_x + 0, tl_y + 1, color);
    DM_SetPx(tl_x + 1, tl_y + 2, color);
    break;
  case 6:
    /* 6 is simply 5 plus one pixel */
    DM_SetPx(tl_x + 0, tl_y + 3, color);
  case 5:
    DM_SetPx(tl_x + 0, tl_y + 0, color);
    DM_SetPx(tl_x + 1, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 0, color);
    DM_SetPx(tl_x + 2, tl_y + 2, color);
    DM_SetPx(tl_x + 2, tl_y + 3, color);
    DM_SetPx(tl_x + 2, tl_y + 4, color);
    DM_SetPx(tl_x + 1, tl_y + 4, color);
    DM_SetPx(tl_x + 0, tl_y + 4, color);
    DM_SetPx(tl_x + 0, tl_y + 2, color);
    DM_SetPx(tl_x + 0, tl_y + 1, color);
    DM_SetPx(tl_x + 1, tl_y + 2, color);
    break;
  default:
    break;
  }
}