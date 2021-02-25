/*  ShowButtonPressEvent.ino

    Purpose:
      Event driven setting of buttons. Simple OLED text.

    What it does:
      This example displays the number of times the LEFT, RIGHT and DOWN buttons have
      had their events triggered using the OLED display.

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

volatile int g_leftCount = 0;
volatile int g_rightCount = 0;
volatile int g_downCount = 0;
volatile int g_rotCount = 0;

void setup() {
  g_nps.begin();

  //set the interrupt events for the left and right buttons. Calls the same funtion regardless of pressed or released. Just sets the state.
  g_nps.setLeftButtonInterrupt(leftPressed, BUTTON_PRESSED);

  g_nps.setRightButtonInterrupt(rightPressed, BUTTON_RELEASED);

  g_nps.setDownButtonInterrupt(downPressed, BUTTON_CHANGED);

  g_nps.setRotaryEncoderButtonInterrupt(rotPressed, BUTTON_PRESSED);
}

void loop() {
  g_nps.oledClear();

  g_nps.oledPrintln(F("Left (P) count is: "));
  g_nps.oledPrint(F("     "));
  g_nps.oledPrintln(g_leftCount);

  g_nps.oledPrintln(F("Right (R) count is: "));
  g_nps.oledPrint(F("     "));
  g_nps.oledPrintln(g_rightCount);

  g_nps.oledPrintln(F("Down (C) count is: "));
  g_nps.oledPrint(F("     "));
  g_nps.oledPrintln(g_downCount);

  g_nps.oledPrintln(F("Rot (P) count is: "));
  g_nps.oledPrint(F("     "));
  g_nps.oledPrintln(g_rotCount);

  g_nps.oledDisplay();
}

//called by the button event. Needs to set the current state of the button
void leftPressed() {
  g_leftCount++;
}

void rightPressed() {
  g_rightCount++;
}

void downPressed(){
  g_downCount++;
}

void rotPressed(){
  g_rotCount++;
}