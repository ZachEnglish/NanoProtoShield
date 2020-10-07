/*  CountTwistsWith1602LCD.ino

    Purpose:
      Show how to write values to the 7 segment display and read from the
      rotary encoder. Also show how to use the 1602 (16 columns by 2 rows)
      LCD display.

    What it does:
      This example uses the two 7 segment digits (and also the 1602 LCD) to
      show how many times the rotary encoder has been turned. Clockwise makes
      the count go up and counter-clockwise makes it go down. It is stored in
      an 8 bit value, so it wraps about 0-255. The decimal places in the 7
      segment indicate one hundred or two hundred. E.g. 1.2. = 212 and
      12. = 112.

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      To get the 1602 to work you have to plug it in! Connect the wires as
      per the directions below:
          1602 I2C Backpack         NanoProtoShield 5V I2C Header
                Gnd           ->            G
                VCC           ->            V
                SDA           ->            D
                SCL           ->            C
*/

#include "NanoProtoShield.h"

#include <LiquidCrystal_I2C.h>

NanoProtoShield g_nps;

LiquidCrystal_I2C g_lcd(0x27, 16, 2);

int g_lastRotaryValue;

void setup() {
  g_nps.begin();
  g_lcd.init();
  g_lcd.backlight();

  g_lastRotaryValue = -1;
}

void loop() {
  int rotaryValue = g_nps.rotaryRead();

  if( rotaryValue != g_lastRotaryValue ){
    g_nps.shift7segPrint(rotaryValue);

    g_lcd.clear();
    g_lcd.print(rotaryValue);

    g_lastRotaryValue = rotaryValue;
  }
}