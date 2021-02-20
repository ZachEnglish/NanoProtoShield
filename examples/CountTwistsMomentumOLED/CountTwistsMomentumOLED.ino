/*  CountTwistsMomentumOLED.ino

    Purpose:
      Show how to write values to the OLED display and read from the rotary
      encoder using momentum.

    What it does:
      Shows the current count from the rotary encoder on the OLED. The value
      will increase faster if the encoder is twisted faster: it has momentum.

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps(FEATURE_ALL, true);

void setup() {
  g_nps.begin();
}

void loop() {
  g_nps.oledClear();
  g_nps.oledPrint(g_nps.rotaryRead());
  g_nps.oledDisplay();
}