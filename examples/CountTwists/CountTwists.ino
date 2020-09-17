#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();
}

void loop() {
  g_nps.shift7segWrite(g_nps.rotaryEncoderRead());
}
