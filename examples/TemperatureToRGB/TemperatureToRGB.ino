/*  TemperatureToRGB.ino

    Purpose:
      Learn how to read from the temperature sensor. Also how to set the color of the
      RGB pixels using a hue value.

    What it does:
      This example sets the color of the 8 RGB LEDs based on the temperature of the
      temperature sensor. It is supposed to go from a "cool" blue to a "warm" red in
      the span of room temperature to "pinched between fingers" temperature.

    TODOs (Things that are broken/not yet working/could be improved):
      Currently seems to go backwards around the color wheel... need to debug.

    Special Notes:
      None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();
}

//seems to go backwards, needs work.
//Feel free to ignore the math going on inside this function, understanding it
//requires quite a bit of color space understanding and math.
//For the curious: https://en.wikipedia.org/wiki/HSL_and_HSV will get you started.
uint16_t get_hue_from_temperature(float t, float cold, float hot) {
  float hue;

  if (t < cold)
    t = cold;
  if (t > hot)
    t = hot;

  hue = 7650.0f + 61200.0f * (hot - t) / (hot - cold);

  return hue;
}

void loop() {
  uint16_t hue;

  g_nps.takeTemperatureReading();
  hue = get_hue_from_temperature(g_nps.getTempF(), 80, 90);

  g_nps.rgbSetPixelsColor( g_nps.rgbGetColorFromHsv(hue));
  g_nps.rgbShow();
}