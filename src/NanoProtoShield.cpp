#include "Arduino.h"
#include "NanoProtoShield.h"

NanoProtoShield::NanoProtoShield() :
    m_RGB_Strip(RGB_LED_COUNT,PIN_RGB_LED, NEO_GRB + NEO_KHZ800),
    m_oled_display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET)
    {
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void NanoProtoShield::RGB_Strip_Color_Wipe(uint8_t r, uint8_t g, uint8_t b, int wait){
    uint32_t color = m_RGB_Strip.Color(r,g,b);
    for (int i = 0; i < m_RGB_Strip.numPixels(); i++) { // For each pixel in strip...
        m_RGB_Strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
        m_RGB_Strip.show();                          //  Update strip to match
        delay(wait);                           //  Pause for a moment
    }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void NanoProtoShield::RGB_Strip_Rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < m_RGB_Strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / m_RGB_Strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      m_RGB_Strip.setPixelColor(i, m_RGB_Strip.gamma32(m_RGB_Strip.ColorHSV(pixelHue)));
    }
    m_RGB_Strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

void NanoProtoShield::RGB_Strip_Clear() {
    m_RGB_Strip.clear();
    m_RGB_Strip.show();
}