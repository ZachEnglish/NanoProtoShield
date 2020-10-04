/*  DrawCirlcle.ino

    Purpose:
        The purpose of this example is to show how to use the pinout overriding
        functionality to get a better rotary encoder experience with the OLED
        in use.

    What it does:
        A circle will be drawn in the center of the display which can be sized
        using the rotary encoder and moved with the four directional buttons.

    TODOs (Things that are broken/not yet working/could be improved):
        Directional button control, mentioned in the previous section, is not
        implemented.

    Special Notes:
        For this to work well, it requires a HW change on your NPS! You must
        take the jumpers off the pins for "Up Sw" and "Enc. B" and connect those
        those pins across each other with extra wires ("Enc. B" -> "Up Sw" and
        "Up Sw" -> "Enc. B"). Links to pictures showing the setup below:
        https://github.com/ZachEnglish/NanoProtoShield/blob/master/examples/DrawCircle/DrawCircleWiring.jpg
        https://github.com/ZachEnglish/NanoProtoShield/blob/master/examples/DrawCircle/DrawCircleWiringCloseUp.jpg
        If you do not want to do the HW modification, you can comment out the
        definition of HW_MOD_FOR_ENCODER_PERFORMANCE below and just have a
        rotary knob that doesn't quite work at fast speeds.
*/

#include "NanoProtoShield.h"

#define HW_MOD_FOR_ENCODER_PERFORMANCE

NanoProtoShield g_nps(FEATURE_ALL);
int lastRotary = -1;

void setup() {
#ifdef HW_MOD_FOR_ENCODER_PERFORMANCE
  //Create a custom pinout for this sketch. Swap the rotary encoder pin B and the up button
  //so that the rotary encoder can have both of the interrupts available on the Nano.
  INDEX_PINS custom_pinout[INDEX_PIN_COUNT];
  initializePinIndexToDefault(custom_pinout);
  custom_pinout[INDEX_PIN_UP_BUTTON] = PIN_DEFAULT_ROT_ENC_B;
  custom_pinout[INDEX_PIN_ROT_ENC_B] = PIN_DEFAULT_UP_BUTTON;
  g_nps.begin(custom_pinout);
#else
  g_nps.begin();
#endif
}

void loop() {
  bool stateChanged = false;
  int currentRotary = g_nps.rotaryRead();

  stateChanged = (currentRotary != lastRotary);

  if (stateChanged) {
    lastRotary = currentRotary;
    g_nps.oledClear();
    g_nps.oledPrint(F("Raidus: "));
    g_nps.oledPrint(currentRotary);
    g_nps.oledDrawCircle(64, 32, currentRotary);
    g_nps.oledDisplay();
  }
}
