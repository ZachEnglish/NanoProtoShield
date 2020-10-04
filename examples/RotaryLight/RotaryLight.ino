/*  RotaryLight.ino

    Purpose:
      Some basic bit shifting math. Writing out to the "regular" 8 LEDs on the board.
      Reading from the rotary encoder's twists and button press.
      Butten event logic.

    What it does:
      This example lights up one of the 8 shift-driven LEDs when the rotary encoder is pressed.
      Turning the rotary encoder moves the light

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps;
uint8_t whichLed;
volatile bool showLed;

void setup() {
  g_nps.begin();

  whichLed = 0;
  showLed = false;

  g_nps.buttonSetPressEvent(BUTTON_ROTARY, setShow);
  g_nps.buttonSetReleaseEvent(BUTTON_ROTARY, unsetShow);

  g_nps.oledPrintln(F("Turn the knob to change the light. Press the knob to see the light."));
  g_nps.oledDisplay();
}

void loop() {
  int rotaryValue = g_nps.rotaryRead();
  g_nps.buttonCheckForEvent();

  if (rotaryValue < 0) {
    rotaryValue += 8;
    g_nps.rotaryWrite(rotaryValue);
  }
  whichLed = rotaryValue % 8;

  if (showLed)
    g_nps.shiftLedWrite(1 << whichLed);
  else
    g_nps.shiftClear();
}

void setShow() {
  showLed = true;
}

void unsetShow() {
  showLed = false;
}
