/*  CountTwistsMomentumOLED.ino

    Purpose:
      Show how to write values to the OLED display and read from the rotary
      encoder using momentum.

    What it does:
      Shows the current count from the rotary encoder on the OLED. Shows the
      state of the momentum setting. When momemtum isenabled, the value will
      increase faster if the encoder is twisted faster: it has momentum.

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();

  g_nps.setRotaryEncoderButtonInterrupt(changeMomemtum, FALLING);
}

void loop() {
  g_nps.oledClear();
  if(g_nps.rotaryGetMomemtum()){
    g_nps.oledPrintln(F("Momemtum: On"));
  } else {
    g_nps.oledPrintln(F("Momemtum: Off"));
  }
  g_nps.oledPrint(F("Value: "));
  g_nps.oledPrint(g_nps.rotaryRead());
  g_nps.oledDisplay();
}

void changeMomemtum(){
  if(g_nps.rotaryGetMomemtum()){
    g_nps.rotaryDisableMomemtum();
  } else {
    g_nps.rotaryEnableMomemtum();
  }
}