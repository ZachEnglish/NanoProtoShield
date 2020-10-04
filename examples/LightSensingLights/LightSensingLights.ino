/*  LightSensingLights.ino

    Purpose:
      Read from the analog light sensor. Some basic bit shifting math. Writing
      out to the "regular" 8 LEDs on the board.

    What it does:
      This example uses the 8 regular LEDs (fed from the shift registors) to
      indicate the level of light coming in from the light sensor. Should go
      from the leftmost LED in darkness to the rightmost LED in very bright
      light.

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
  float lightLevel =  g_nps.lightMeterRead();
  byte whichBit = lightLevel * 8.0f / 5.0f;

  g_nps.shiftLedWrite(1 << whichBit);
}