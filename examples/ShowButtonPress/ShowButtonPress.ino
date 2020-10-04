/*  ShowButtonPress.ino

    Purpose:
      Direct reading of the button state (not event driven). Simple OLED text.

    What it does:
      This example displays the state of the UP button on the OLED display.

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
  g_nps.oledClear();

  g_nps.oledPrintln(F("Up button is: "));
  if (g_nps.buttonUpPressed()) {
    g_nps.oledPrintln(F("Pressed"));
  } else {
    g_nps.oledPrintln(F("Not pressed"));
  }

  g_nps.oledDisplay();
}