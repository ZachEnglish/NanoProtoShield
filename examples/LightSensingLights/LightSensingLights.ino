#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();
}

void loop() {
  float lightLevel =  g_nps.photoRead();
  byte whichBit = lightLevel * 8.0f / 5.0f;

  g_nps.shiftLedWrite(1 << whichBit);
}
