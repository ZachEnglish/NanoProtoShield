#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
    g_nps.begin();
}

void loop() {
    g_nps.shift_7seg_write(g_nps.rotary_encoder_read());
}