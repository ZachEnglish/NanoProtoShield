#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
    g_nps.begin();
}

void loop() {
    for(int i = 0; i < RGB_LED_COUNT; i++){
        g_nps.RGB_set_pixel_color(
            i,
            g_nps.pot1_read()*VOLTAGE_TO_RGB,
            g_nps.pot2_read()*VOLTAGE_TO_RGB,
            g_nps.pot3_read()*VOLTAGE_TO_RGB);
    }
    g_nps.RGB_show();
}