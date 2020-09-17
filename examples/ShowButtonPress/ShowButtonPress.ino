// This example displays the state of the UP button on the OLED display.

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();
}

void loop() {
  g_nps.oledClear();

  g_nps.oledPrintln(F("Up button is: "));
  if (digitalRead(PIN_UP_BUTTON)) {
    g_nps.oledPrintln(F("Pressed"));
  } else {
    g_nps.oledPrintln(F("Not pressed"));
  }

  g_nps.oledDisplay();
}