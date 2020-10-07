/*  CountTwists.ino

    Purpose:
      Show how to write values to the 7 segment display and read from the
      rotary encoder.

    What it does:
      This example uses the two 7 segment digits to show how many times the
      rotary encoder has been turned. Clockwise makes the count go up and
      counter-clockwise makes it go down. It is stored in an 8 bit value, so
      it wraps about 0-255. The decimal places in the 7 segment indicate one
      hundred or two hundred. E.g. 1.2. = 212 and 12. = 112

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();
}

void loop() {
  g_nps.shift7segPrint(g_nps.rotaryRead());
}