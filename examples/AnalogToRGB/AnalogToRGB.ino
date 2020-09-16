#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
    g_nps.begin();
}

void loop() {
    g_nps.RGB_set_pixels_color(
        g_nps.pot1_read()*VOLTAGE_TO_RGB,
        g_nps.pot2_read()*VOLTAGE_TO_RGB,
        g_nps.pot3_read()*VOLTAGE_TO_RGB);
    g_nps.RGB_show();
}