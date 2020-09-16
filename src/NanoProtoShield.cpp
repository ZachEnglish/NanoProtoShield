#include "Arduino.h"
#include "NanoProtoShield.h"


const byte g_map_7seg[] PROGMEM = {
  0b00111111, //0
  0b00000110, //1
  0b01011011, //2
  0b01001111, //3
  0b01100110, //4
  0b01101101, //5
  0b01111101, //6
  0b00000111, //7
  0b01111111, //8
  0b01101111, //9
  0b01110111, //A
  0b01111100, //b
  0b00111001, //C
  0b01011110, //d
  0b01111001, //E
  0b01110001  //F
};

#define DEC_7SEG 0b10000000

uint32_t g_RGB_data[8];

NanoProtoShield::NanoProtoShield() :
  m_rotary_encoder(PIN_ROT_ENC_A, PIN_ROT_ENC_B),
  m_one_wire(PIN_TEMPERATURE),
  m_mpu(Wire),
  m_oled_display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET),
  m_RGB(RGB_LED_COUNT,PIN_RGB_LED, NEO_GRB + NEO_KHZ800)
  //m_button_left(PIN_LEFT_BUTTON),
  //m_button_right(PIN_RIGHT_BUTTON)
  {
    //Initialize the class's memory of what data the shift registers have
    m_shift_7seg_left  = 0xFF;
    m_shift_7seg_right = 0xFF;
    m_shift_led        = 0x00;
    m_interrupt        = 0;
    m_button_state     = 0;
    m_button_pressed   = 0;
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

    clear_all_displays();
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
  memset(g_RGB_data, 0x00, sizeof(g_RGB_data));
  m_RGB.clear();
  m_RGB.show();
}

void NanoProtoShield::RGB_set_pixel_color(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b){
  m_RGB.setPixelColor( pixel % RGB_LED_COUNT, m_RGB.Color(r,g,b));
}

void NanoProtoShield::RGB_set_pixels_color(uint8_t r, uint8_t g, uint8_t b){
  for(int i = 0; i < RGB_LED_COUNT; i++)
    RGB_set_pixel_color(i,r,g,b);
}

void NanoProtoShield::RGB_set_pixel_color(uint8_t pixel, uint32_t color){
  m_RGB.setPixelColor( pixel, color );
}

void NanoProtoShield::RGB_set_pixels_color(uint32_t color){
  for(int i = 0; i < RGB_LED_COUNT; i++)
    RGB_set_pixel_color(i,color);
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

void NanoProtoShield::shift_7seg_write(uint8_t num){
  byte left, right;
  right = num % 10;
  right = pgm_read_byte_near(g_map_7seg + right) | ((num>=100)?DEC_7SEG:0x00); //get the digit and determine if the decimal is needed
  left = (num / 10) % 10;
  left = pgm_read_byte_near(g_map_7seg + left) | ((num>=200)?DEC_7SEG:0x00);
  
  shift_7seg_write(left, right);
}

void NanoProtoShield::shift_7seg_write_hex(uint8_t num){
  byte left, right;
  right = num & 0x0F;
  left = (num & 0xF0)>>4;
  
  shift_7seg_write(pgm_read_byte_near(g_map_7seg + left),pgm_read_byte_near(g_map_7seg + right));
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
  DallasTemperature temp_sensor(&m_one_wire);
    
  //Start up the temperature sensor library
  temp_sensor.begin();

  temp_sensor.requestTemperatures();
  m_temperature_C = temp_sensor.getTempCByIndex(0);
  m_temperature_F = temp_sensor.getTempFByIndex(0);
}

void NanoProtoShield::interrupt(){
  m_interrupt++;
}

void NanoProtoShield::clear_all_displays(DISPLAYS exceptions = DISPLAY_NONE){
  if ( !(exceptions & DISPLAY_SHIFT_LEDS) ) {
    shift_led_write(0x00);
    }
  if ( !(exceptions & DISPLAY_SHIFT_7SEG) ) {
    shift_7seg_write(0x00,0x00);
  }
  OLED_clear();
  if (!(exceptions & DISPLAY_OLED) ) {
    OLED_display();
  }
  if ( !(exceptions & DISPLAY_RGB_LEDS) ) {
    RGB_clear();
  }
}

void NanoProtoShield::MPU_calculate_offsets(int wait){
  m_mpu.begin();
  delay(wait);
  m_mpu.calcGyroOffsets();
}

void NanoProtoShield::MPU_update(){
  m_mpu.update();
}

void NanoProtoShield::button_check(){
  byte old_state = m_button_state;

  m_button_state = digitalRead(PIN_UP_BUTTON) << BUTTON_UP |
                   digitalRead(PIN_DOWN_BUTTON) << BUTTON_DOWN |
                   digitalRead(PIN_LEFT_BUTTON) << BUTTON_LEFT |
                   digitalRead(PIN_RIGHT_BUTTON) << BUTTON_RIGHT;

  m_button_pressed = m_button_pressed | (m_button_state &(~old_state));
  if(button_pressed(BUTTON_DOWN))
    m_button_down_event();
}

bool NanoProtoShield::button_pressed(BUTTONS b, bool clear = true){
  if(bitRead(m_button_pressed, b))
  {
    if(clear)
    {
      m_button_pressed = bitClear(m_button_pressed, b);
    }
    return true;
  }
  return false;
}

void NanoProtoShield::button_down_event_set(void (*butten_event)(void)){
  m_button_down_event = butten_event;
}