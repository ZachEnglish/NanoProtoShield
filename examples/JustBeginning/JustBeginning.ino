#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
    g_nps.begin();
}

void loop() {
    g_nps.OLED_clear();

    g_nps.OLED_println(F("Up button is: "));
    if(digitalRead(PIN_UP_BUTTON)){
        g_nps.OLED_println(F("Pressed"));
    } else {
        g_nps.OLED_println(F("Not pressed"));
    }
    
    g_nps.OLED_display();
}