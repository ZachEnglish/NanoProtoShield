//The purpose of this example is to show how to use some of the more advanced/direct
//drawing methods available with the OLED display. A circle will be drawn in the
//center of the display which can be sized using the rotary encoder and moved with
//the four directional buttons.
//TODO Directional button control
//FYI Have to move the encoder very slowly or many twists are missed

#include "NanoProtoShield.h"

NanoProtoShield g_nps;
int lastRotary = -1;

void setup(){
    g_nps.begin();
}

void loop(){
    bool stateChanged = false;
    int currentRotary = g_nps.rotaryRead();

    stateChanged = (currentRotary != lastRotary);

    if(stateChanged){
        lastRotary = currentRotary;
        g_nps.oledClear();
        g_nps.oledPrint(F("Raidus: "));
        g_nps.oledPrint(currentRotary);
        g_nps.oledDrawCircle(64, 32, currentRotary);
        g_nps.oledDisplay();
    }
}