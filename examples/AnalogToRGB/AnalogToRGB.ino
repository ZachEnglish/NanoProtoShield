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
