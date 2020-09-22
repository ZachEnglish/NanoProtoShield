// This example uses the two 7 segment digits to show how many times the
// rotary encoder has been turned. Clockwise makes the count go up and
// counter-clockwise makes it go down. It is stored in an 8 bit value, so
// it wraps about 0-255. The decimal places in the 7 segment indicate one
// hundred or two hundred. E.g. 1.2. = 212 and 12. = 120

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();
}

void loop() {
  g_nps.shift7segPrint(g_nps.rotaryRead());
}