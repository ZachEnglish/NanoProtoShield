#include "Arduino.h"
#include "NanoProtoShield.h"

NanoProtoShield::NanoProtoShield() :
  m_RGB_Strip(RGB_LED_COUNT,PIN_RGB_LED, NEO_GRB + NEO_KHZ800),
  m_oled_display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET),
  m_rotary_encoder(PIN_ROT_ENC_A, PIN_ROT_ENC_B),
  m_one_wire(PIN_TEMPERATURE),
  m_temp_sensor(&m_one_wire),
  m_mpu(Wire)
  {
    //Set up the OLED display
    if (!m_oled_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) { // Address 0x3C for 128x64
      Serial.println(F("SSD1306 allocation failed"));
      for (;;); // Don't proceed, loop forever
    }

    m_shift_7seg_left  = 0xFF;
    m_shift_7seg_right = 0xFF;
    m_shift_led        = 0x00;

    //Set up shift register pins
    pinMode(PIN_SHIFT_LATCH, OUTPUT);
    pinMode(PIN_SHIFT_CLOCK, OUTPUT);
    pinMode(PIN_SHIFT_DATA, OUTPUT);

    //Set up the buttons as imputs
    pinMode(PIN_LEFT_BUTTON, INPUT);
    pinMode(PIN_RIGHT_BUTTON, INPUT);
    pinMode(PIN_UP_BUTTON, INPUT);
    pinMode(PIN_DOWN_BUTTON, INPUT);
    pinMode(PIN_ROT_ENC_BUTTON, INPUT);
    pinMode(PIN_ROT_ENC_A, INPUT);
    pinMode(PIN_ROT_ENC_B, INPUT);
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void NanoProtoShield::RGB_strip_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait){
  uint32_t color = m_RGB_Strip.Color(r,g,b);
  for (int i = 0; i < m_RGB_Strip.numPixels(); i++) { // For each pixel in strip...
    m_RGB_Strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    m_RGB_Strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void NanoProtoShield::RGB_strip_rainbow(int wait) {
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

void NanoProtoShield::RGB_strip_clear() {
  m_RGB_Strip.clear();
  m_RGB_Strip.show();
}


void NanoProtoShield::OLED_display(int clear_after = 0) {
  m_oled_display.display();

  if(clear_after > 0)
  {
    delay(clear_after);
    m_oled_display.clearDisplay();
  }
}

void NanoProtoShield::OLED_clear() {
  m_oled_display.clearDisplay();
  m_oled_display.display();
}

void NanoProtoShield::OLED_print(String s) {
  m_oled_display.clearDisplay();
  m_oled_display.setTextSize(1);
  m_oled_display.setTextColor(SSD1306_WHITE);
  m_oled_display.setCursor(0, 0);
  m_oled_display.println(s);
  m_oled_display.display();
}

float NanoProtoShield::read_pot1() {
  return analogRead(PIN_POT1) * ANALOG_TO_VOLTAGE;
}

float NanoProtoShield::read_pot2() {
  return analogRead(PIN_POT2) * ANALOG_TO_VOLTAGE;
}

float NanoProtoShield::read_pot3() {
  return analogRead(PIN_POT3) * ANALOG_TO_VOLTAGE;
}

float NanoProtoShield::read_photo() {
  return analogRead(PIN_PHOTO) * ANALOG_TO_VOLTAGE;
}

void NanoProtoShield::MPU_calculate_offsets(int wait){
  m_mpu.begin();
  delay(wait);
  m_mpu.calcGyroOffsets();
}

void NanoProtoShield::shift_7seg_set(byte left, byte right){
  //invert the input because the 7 segment leds are active low
  m_shift_7seg_left = ~left;
  m_shift_7seg_right = ~right;
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_7seg_right); //this gets shifted to the second display, send it first
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_7seg_left);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_led);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

void NanoProtoShield::shift_led_set(byte b){
  m_shift_led = b;
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_7seg_right); //this gets shifted to the second display, send it first
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_7seg_left);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_led);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

byte NanoProtoShield::shift_7seg_get_left(){
  return ~m_shift_7seg_left;
}

byte NanoProtoShield::shift_7seg_get_right(){
  return ~m_shift_7seg_right;
}

byte NanoProtoShield::shift_led_get(){
  return m_shift_led;
}