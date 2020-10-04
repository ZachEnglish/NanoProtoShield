/*  AnimateBitmap.ino

    Purpose:
      Show how to create a bitmap and draw it on the OLED display

    What it does:
      Draws a smiley face and makes it move around the display

    TODOs (Things that are broken/not yet working/could be improved):
        None

    Special Notes:
      Draws (hehehe) heavily from the Adafruit SSD1306 example ssd1306_128x64_i2c.ino
*/

#include "NanoProtoShield.h"

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

NanoProtoShield g_nps;

#define NUMBER_OF_BITMAPS 10
#define XPOS   0 // Indexes into the 'bitmapInfo' array
#define YPOS   1
#define DELTAY 2
int8_t bitmapInfo[NUMBER_OF_BITMAPS][3];

void setup() {
  g_nps.begin();

  for (int i = 0; i < NUMBER_OF_BITMAPS; i++) {
    bitmapInfo[i][XPOS]   = random(1 - SMILEY_WIDTH, OLED_SCREEN_WIDTH);
    bitmapInfo[i][YPOS]   = -SMILEY_HEIGHT;
    bitmapInfo[i][DELTAY] = random(1, 6); //how fast the bitmaps move "fall" on the screen
  }
}

void loop() {
  g_nps.oledClear();

  //draw the bitmaps
  for (int i = 0; i < NUMBER_OF_BITMAPS; i++) {
    g_nps.oledDrawBitmap(bitmapInfo[i][XPOS], bitmapInfo[i][YPOS], smileyBmp, SMILEY_WIDTH, SMILEY_HEIGHT);
  }

  g_nps.oledDisplay();
  delay(200);

  //Update coordinates of each bitmap...
  for (int i = 0; i < NUMBER_OF_BITMAPS; i++) {
    bitmapInfo[i][YPOS] += bitmapInfo[i][DELTAY];
    // If bitmap is off the bottom of the screen...
    if (bitmapInfo[i][YPOS] >= OLED_SCREEN_HEIGHT) {
      // Reinitialize to a random position, just off the top
      bitmapInfo[i][XPOS]   = random(1 - SMILEY_WIDTH, OLED_SCREEN_WIDTH);
      bitmapInfo[i][YPOS]   = -SMILEY_HEIGHT;
      bitmapInfo[i][DELTAY] = random(1, 6);
    }
  }
}