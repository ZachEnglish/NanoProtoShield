/*  DrawBitmap.ino

    Purpose:
        The purpose of this example is to show how to draw a bitmap onto the OLED

    What it does:
        Draws a snowflake and a smiley

    TODOs (Things that are broken/not yet working/could be improved):
        None

    Special Notes:
        None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

#define SNOWFLAKE_HEIGHT   16
#define SNOWFLAKE_WIDTH    16
static const unsigned char PROGMEM snowflakeBmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#define SMILEY_HEIGHT   16
#define SMILEY_WIDTH    16
static const unsigned char PROGMEM smileyBmp[] =
{ B00000111, B11100000,
  B00001000, B00010000,
  B00110000, B00001100,
  B00100000, B00000100,
  B01000100, B00100010,
  B10001010, B01010010,
  B10000100, B00100001,
  B10000000, B00000001,
  B10000000, B00000001,
  B10000000, B00000001,
  B10010000, B00001010,
  B01001000, B00010010,
  B00100111, B11100100,
  B00110000, B00001100,
  B00001100, B00110000,
  B00000011, B11000000
};

void setup() {
  g_nps.begin();
}

void loop() {
  int x, y;

  x = (OLED_SCREEN_WIDTH - SNOWFLAKE_WIDTH) / 2;
  y = (OLED_SCREEN_HEIGHT - SNOWFLAKE_HEIGHT) / 2;

  g_nps.oledClear();

  g_nps.oledDrawBitmap(x, y - 20, snowflakeBmp, SNOWFLAKE_WIDTH, SNOWFLAKE_HEIGHT);

  g_nps.oledDrawBitmap(x, y + 20, smileyBmp, SMILEY_WIDTH, SMILEY_HEIGHT);

  g_nps.oledDisplay();
}