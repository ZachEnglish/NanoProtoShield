// This example displays the state of the LEFT and RIGHT buttons on the OLED display.
// Uses events to do the setting.

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

volatile bool g_leftPressed;
volatile bool g_rightPressed;

void setup() {
  g_nps.begin();

  //set the initial state of both buttons
  g_leftPressed = g_nps.buttonLeftPressed();
  g_rightPressed = g_nps.buttonRightPressed();

  //set the events for the left and right buttons. Calls the same funtion regardless of pressed or released. Just sets the state.
  //These functions (leftPressed & rightPressed) will get called when the corresponding event is calculated by buttonCheckForEvent()
  g_nps.buttonSetPressEvent(BUTTON_LEFT, leftPressed);
  g_nps.buttonSetReleaseEvent(BUTTON_LEFT, leftPressed);

  g_nps.buttonSetPressEvent(BUTTON_RIGHT, rightPressed);
  g_nps.buttonSetReleaseEvent(BUTTON_RIGHT, rightPressed);
}

void loop() {
  g_nps.oledClear();
  g_nps.buttonCheckForEvent(); //when using button events you need to check in the loop to catch the state change

  g_nps.oledPrintln(F("Left button is: "));
  g_nps.oledPrintln(g_leftPressed?F("Pressed"):F("Not pressed"));

  g_nps.oledPrintln(F("Right button is: "));
  g_nps.oledPrintln(g_rightPressed?F("Pressed"):F("Not pressed"));

  g_nps.oledDisplay();
}

//called by the button event. Needs to set the current state of the button
void leftPressed(){
  g_leftPressed = g_nps.buttonLeftPressed();
}

void rightPressed(){
  g_rightPressed = g_nps.buttonRightPressed();
}