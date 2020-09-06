#include "Arduino.h"
#include "NanoProtoShield.h"


uint32_t g_RGB_data[8];

NanoProtoShield::NanoProtoShield(FEATURES feature_list = ALL_FEATURES) :
  m_rotary_encoder(PIN_ROT_ENC_A, PIN_ROT_ENC_B),
  m_one_wire(PIN_TEMPERATURE),
  m_temp_sensor(&m_one_wire),
  m_mpu(Wire),
  m_oled_display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET),
  m_RGB(RGB_LED_COUNT,PIN_RGB_LED, NEO_GRB + NEO_KHZ800)
  {
    //Initialize the class's memory of what data the shift registers have
    m_shift_7seg_left  = 0xFF;
    m_shift_7seg_right = 0xFF;
    m_shift_led        = 0x00;
    m_interrupt        = 0;
  
}

void NanoProtoShield::begin(){
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
    
    //Start up the temperature sensor library
    m_temp_sensor.begin();

    //Set up the RGB strip
    m_RGB.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    m_RGB.show();            // Turn OFF all pixels ASAP
    m_RGB.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

    // Set up the OLED display
    if (!m_oled_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for (;;); // Don't proceed, loop forever
    }
    
    m_oled_display.setTextSize(1);
    m_oled_display.setTextColor(SSD1306_WHITE);

    memset(g_RGB_data, 0x00, sizeof(g_RGB_data));
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void NanoProtoShield::RGB_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait) {
  uint8_t starting_interrupt = m_interrupt;
  uint32_t color = m_RGB.Color(r,g,b);
  for (int i = 0; i < m_RGB.numPixels(); i++) { // For each pixel in strip...
    m_RGB.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    m_RGB.show();                          //  Update strip to match
    if(m_interrupt != starting_interrupt)
      return;
    delay(wait);                           //  Pause for a moment
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void NanoProtoShield::RGB_rainbow(int wait) {
  uint8_t starting_interrupt = m_interrupt;
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < m_RGB.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / m_RGB.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      m_RGB.setPixelColor(i, m_RGB.gamma32(m_RGB.ColorHSV(pixelHue)));
    }
    m_RGB.show(); // Update strip with new contents
    if(m_interrupt != starting_interrupt)
      return;
    delay(wait);  // Pause for a moment
  }
}

void NanoProtoShield::RGB_clear() {
  m_RGB.clear();
  m_RGB.show();
}

void NanoProtoShield::RGB_set_pixel_color(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b){
  m_RGB.setPixelColor(pixel, m_RGB.Color(r,g,b));
}

void NanoProtoShield::RGB_set_brightness(uint8_t brightness){
  m_RGB.setBrightness(brightness);
}

void NanoProtoShield::RGB_show(){
  m_RGB.show();
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
  m_oled_display.setCursor(0, 0);
}

size_t NanoProtoShield::OLED_print(const __FlashStringHelper *c) {
  return m_oled_display.print(c);
}

size_t NanoProtoShield::OLED_print(const String &c) {
  return m_oled_display.print(c);
}

size_t NanoProtoShield::OLED_print(const char c[]) {
  return m_oled_display.print(c);
}

size_t NanoProtoShield::OLED_print(char c) {
  return m_oled_display.print(c);
}

size_t NanoProtoShield::OLED_print(const Printable& c) {
  return m_oled_display.print(c);
}

size_t NanoProtoShield::OLED_println(const __FlashStringHelper *c) {
  return m_oled_display.println(c);
}

size_t NanoProtoShield::OLED_println(const String &s) {
  return m_oled_display.println(s);
}

size_t NanoProtoShield::OLED_println(const char c[]) {
  return m_oled_display.println(c);
}

size_t NanoProtoShield::OLED_println(char c) {
  return m_oled_display.println(c);
}

size_t NanoProtoShield::OLED_println(const Printable& c) {
  return m_oled_display.println(c);
}

size_t NanoProtoShield::OLED_println(void) {
  return m_oled_display.println();
}

void NanoProtoShield::shift_7seg_write(byte left, byte right){
  //invert the input because the 7 segment leds are active low
  m_shift_7seg_left = ~left;
  m_shift_7seg_right = ~right;
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_7seg_right); //this gets shifted to the second display, send it first
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_7seg_left);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_led);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

void NanoProtoShield::shift_led_write(byte b){
  m_shift_led = b;
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_7seg_right); //this gets shifted to the second display, send it first
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_7seg_left);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift_led);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

byte NanoProtoShield::shift_7seg_left_read(){
  return ~m_shift_7seg_left;
}

byte NanoProtoShield::shift_7seg_right_read(){
  return ~m_shift_7seg_right;
}

byte NanoProtoShield::shift_led_read(){
  return m_shift_led;
}

void NanoProtoShield::shift_clear(){
  shift_7seg_write(0x00,0x00);
  shift_led_write(0x00);
}

void NanoProtoShield::shift_test_sequence(int wait){
  shift_led_write(0b10101010);
  delay(wait);
  shift_led_write(0b01010101);
  delay(wait);
  shift_led_write(0x00);

  shift_7seg_write(0b10101010, 0b10101010);
  delay(wait);
  shift_7seg_write(0b01010101, 0b01010101);
  delay(wait);
  shift_7seg_write(0b10101010, 0b01010101);
  delay(wait);
  shift_7seg_write(0b01010101, 0b10101010);
  delay(wait);
  shift_7seg_write(0x00, 0xFF);
  delay(wait);
  shift_7seg_write(0xFF, 0x00);
  delay(wait);
  shift_clear();
}

void NanoProtoShield::take_temperature_reading(){
  m_temp_sensor.requestTemperatures();
  m_temperature_C = m_temp_sensor.getTempCByIndex(0);
  m_temperature_F = m_temp_sensor.getTempFByIndex(0);
}

void NanoProtoShield::interrupt(){
  m_interrupt++;
}

void NanoProtoShield::clear_all_displays(DISPLAYS exception = DISPLAY_NONE){
  if ( exception != DISPLAY_SHIFT_LEDS ) {
    shift_led_write(0x00);
    }
  if ( exception != DISPLAY_SHIFT_7SEG ) {
    shift_7seg_write(0x00,0x00);
  }
  OLED_clear();
  if ( exception != DISPLAY_OLED ) {
    OLED_display();
  }
  if ( exception != DISPLAY_RGB_LEDS ) {
    RGB_clear();
  }
}

void NanoProtoShield::MPU_calculate_offsets(int wait){
  m_mpu.begin();
  delay(wait);
  m_mpu.calcGyroOffsets();
}

void NanoProtoShield::MPU_update() {
  m_mpu.update();
}