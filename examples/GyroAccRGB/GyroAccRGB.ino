// This example sets the hue and brightness of the RGB LEDs based on readings
// from the acceleromter and gyroscope. The goal is to see them changed by
// tilting the board different directions.

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

//timer used for gyro calculations
long g_timer = 0;

void setup() {
  g_nps.begin();

  g_nps.oledPrint(F("Calculating gyro offset, do not move MPU6050"));
  g_nps.oledDisplay();
  g_nps.mpuCalculateOffsets(1000);
  g_nps.clearAllDisplays();
}


uint8_t mapAngleToRgb(float a) {
  while (a > 360.0f)
    a -= 360.0f;
  while (a < 0.0f)
    a += 360.0f;
  return ( a * (255.0f / 360.0f) );
}

void loop() {
  float x, y, z;

  g_nps.mpuUpdate();

  if (millis() - g_timer > 100) { // update LEDs 10x every second
    x = g_nps.mpuGetAngleX();
    y = g_nps.mpuGetAngleY();
    z = g_nps.mpuGetAngleZ();

    g_nps.rgbSetPixelsColor(mapAngleToRgb(x), mapAngleToRgb(y), mapAngleToRgb(z));
    g_nps.rgbShow();

    g_nps.oledClear();

    g_nps.oledPrint(F("ANGLE X: ")); g_nps.oledPrintln(x, 2);
    g_nps.oledPrint(F("ANGLE Y: ")); g_nps.oledPrintln(y, 2);
    g_nps.oledPrint(F("ANGLE Z: ")); g_nps.oledPrintln(z, 2);
    g_nps.oledPrint(F("R : ")); g_nps.oledPrintln(mapAngleToRgb(x));
    g_nps.oledPrint(F("G. : ")); g_nps.oledPrintln(mapAngleToRgb(y));
    g_nps.oledPrint(F("B : ")); g_nps.oledPrintln(mapAngleToRgb(z));
    g_nps.oledDisplay();

    g_timer = millis();
  }
}
