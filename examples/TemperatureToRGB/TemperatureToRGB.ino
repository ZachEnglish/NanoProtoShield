#include "NanoProtoShield.h"

NanoProtoShield g_nps;

void setup() {
  g_nps.begin();
}


//seems to go backwards, needs work.
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
