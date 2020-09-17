#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();

  g_nps.oledPrint(F("Calculating gyro offset, do not move MPU6050"));
  g_nps.oledDisplay();
  g_nps.mpuCalculateOffsets(1000);
  g_nps.clearAllDisplays();
}

uint8_t mapAngleToBrightness(float a) {
  return ( (a + 90.0f) * (255.0f / 180.0f) );
}

uint16_t mapAngleToHue(float a) {
  return ( (a + 90.0f) * (255.0f * 255.0f / 180.0f) );
}

void loop() {
  uint8_t brightness;
  uint16_t hue;
  float x, y;

  g_nps.mpuUpdate();

  x = g_nps.mpuGetAccAngleX();
  y = g_nps.mpuGetAccAngleY();

  brightness = mapAngleToBrightness(x);
  hue = mapAngleToHue(y);

  g_nps.rgbSetPixelsColor(hue);
  g_nps.rgbSetBrightness(brightness);
  g_nps.rgbShow();
}
