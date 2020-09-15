#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
    g_nps.begin();
}

void loop() {
    float lightLevel =  g_nps.photo_read();
    byte whichBit = lightLevel * 8.0f / 5.0f;

    g_nps.shift_led_write(1<<whichBit);
}