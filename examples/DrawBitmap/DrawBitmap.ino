//The purpose of this example is to show how to draw a bitmap onto the OLED
//Draws a snowflake and animates it
//TODO Something ain't right about the bitmap, seems to be drawing random memory or something else
//     Guessing it is not properly grabbing memory from the PROGMEM or something

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
  B00000000, B00110000 };

void setup(){
    g_nps.begin();
}

void loop(){
    g_nps.oledClear();
    g_nps.oledDrawBitmap(0, 0, snowflakeBmp, SNOWFLAKE_WIDTH, SNOWFLAKE_HEIGHT);
    g_nps.oledDisplay();
}