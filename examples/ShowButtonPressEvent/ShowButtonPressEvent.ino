/*  ShowButtonPressEvent.ino

    Purpose:
      Event driven setting of buttons. Simple OLED text.

    What it does:
      This example displays the state of the LEFT and RIGHT buttons on the OLED display.

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

volatile bool g_leftPressed;
volatile bool g_rightPressed;

void setup() {
  g_nps.begin();

  //set the initial state of both buttons
  g_leftPressed = g_nps.buttonLeftPressed();
  g_rightPressed = g_nps.buttonRightPressed();

  //set the interrupt events for the left and right buttons. Calls the same funtion regardless of pressed or released. Just sets the state.
  g_nps.pinSetEvent(g_nps.getPin(INDEX_PIN_LEFT_BUTTON), leftPressed, CHANGE);

  g_nps.pinSetEvent(g_nps.getPin(INDEX_PIN_RIGHT_BUTTON), rightPressed, CHANGE);
}

void loop() {
  g_nps.oledClear();

  g_nps.oledPrintln(F("Left button is: "));
  g_nps.oledPrintln(g_leftPressed ? F("Pressed") : F("Not pressed"));

  g_nps.oledPrintln(F("Right button is: "));
  g_nps.oledPrintln(g_rightPressed ? F("Pressed") : F("Not pressed"));

  g_nps.oledDisplay();
}

//called by the button event. Needs to set the current state of the button
void leftPressed() {
  g_leftPressed = g_nps.buttonLeftPressed();
}

void rightPressed() {
  g_rightPressed = g_nps.buttonRightPressed();
}
