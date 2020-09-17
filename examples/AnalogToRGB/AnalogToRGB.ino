// This example uses the 3 potentiometers to set the red, green, and blue
// values of the 8 RGB LEDs. They should update in real time.

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();
}

void loop() {
  g_nps.rgbSetPixelsColor(
    g_nps.pot1Read()*VOLTAGE_TO_RGB,
    g_nps.pot2Read()*VOLTAGE_TO_RGB,
    g_nps.pot3Read()*VOLTAGE_TO_RGB);
  g_nps.rgbShow();
}