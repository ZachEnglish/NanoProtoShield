#include "Arduino.h"
#include "NanoProtoShield.h"

//Mapping for 7 segment display.
// AAAAAAA
// F     B
// F     B
//  GGGGG
// E     C
// E     C
// DDDDDDD dp
//
// bit                8 7654321
// byte packed -> 0b(dp)GFEDCBA
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
//7segment decimal point
//Can be bitwise 'or'ed (single '|') together with the other segments in the g_map_7seg to add the decimal point
#define DEC_7SEG 0b10000000


NanoProtoShield::NanoProtoShield() :
  m_rotary(PIN_ROT_ENC_B, PIN_ROT_ENC_A), //this order makes clockwise positive on the NPS
  m_oneWire(PIN_TEMPERATURE),
  m_mpu(Wire),
  m_oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET),
  m_rgb(RGB_LED_COUNT,PIN_RGB_LED, NEO_GRB + NEO_KHZ800)
  {
    //Initialize the class's memory of what data the shift registers have
    m_shift7segLeft  = 0xFF;
    m_shift7segRight = 0xFF;
    m_shiftLed       = 0x00;
    m_interrupt      = 0;
    m_buttonState    = 0;
    m_buttonPressed  = 0;

    for(int i = 0; i < BUTTON_COUNT; i++){
      m_buttonPressEvents[i] = NULL;
      m_buttonReleaseEvents[i] = NULL;
    }
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
    m_rgb.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    m_rgb.show();            // Turn OFF all pixels ASAP
    m_rgb.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

    // Set up the OLED display
    // This begin() call tries to put 1024 bytes on the stack for the display buffer, which
    // is half of the entire stack! Easy to blow it.
    if (!m_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for (;;); // Don't proceed, loop forever
    }
    
    m_oled.setTextSize(1);
    m_oled.setTextColor(SSD1306_WHITE);

    clearAllDisplays();
}


void NanoProtoShield::rgbColorWipe(uint8_t r, uint8_t g, uint8_t b, int wait) {
  uint8_t starting_interrupt = m_interrupt;
  uint32_t color = m_rgb.Color(r,g,b);
  for (int i = 0; i < m_rgb.numPixels(); i++) { // For each pixel in strip...
    m_rgb.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    m_rgb.show();                          //  Update strip to match
    if(m_rgbInterrupt)
      buttonCheckForEvent();
    if(m_interrupt != starting_interrupt)
      return;
    delay(wait);                           //  Pause for a moment
  }
}

void NanoProtoShield::rgbRainbow(int wait) {
  uint8_t starting_interrupt = m_interrupt;
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < m_rgb.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / m_rgb.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      m_rgb.setPixelColor(i, m_rgb.gamma32(m_rgb.ColorHSV(pixelHue)));
    }
    m_rgb.show(); // Update strip with new contents
    if(m_rgbInterrupt)
      buttonCheckForEvent();
    if(m_interrupt != starting_interrupt)
      return;
    delay(wait);  // Pause for a moment
  }
}

void NanoProtoShield::rgbClear() {
  m_rgb.clear();
  m_rgb.show();
}

void NanoProtoShield::rgbSetPixelsColor(uint8_t r, uint8_t g, uint8_t b){
  for(int i = 0; i < RGB_LED_COUNT; i++)
    rgbSetPixelColor(i,r,g,b);
}

void NanoProtoShield::rgbSetPixelsColor(uint32_t color){
  for(int i = 0; i < RGB_LED_COUNT; i++)
    rgbSetPixelColor(i,color);
}


void NanoProtoShield::oledDisplay(int clear_after = 0) {
  m_oled.display();

  if(clear_after > 0)
  {
    delay(clear_after);
    m_oled.clearDisplay();
  }
}

void NanoProtoShield::oledClear() {
  m_oled.clearDisplay();
  m_oled.setCursor(0, 0);
}


void NanoProtoShield::shift7segWrite(byte left, byte right){
  //invert the input because the 7 segment leds are active low
  m_shift7segLeft = ~left;
  m_shift7segRight = ~right;
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift7segRight); //this gets shifted to the second display, send it first
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift7segLeft);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shiftLed);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

void NanoProtoShield::shift7segPrint(uint8_t num){
  byte left, right;
  right = num % 10;
  right = pgm_read_byte_near(g_map_7seg + right) | ((num>=100)?DEC_7SEG:0x00); //get the digit and determine if the decimal is needed
  left = (num / 10) % 10;
  left = pgm_read_byte_near(g_map_7seg + left) | ((num>=200)?DEC_7SEG:0x00);
  
  shift7segWrite(left, right);
}

void NanoProtoShield::shift7segPrintHex(uint8_t num){
  byte left, right;
  right = num & 0x0F;
  left = (num & 0xF0)>>4;
  
  shift7segWrite(pgm_read_byte_near(g_map_7seg + left),pgm_read_byte_near(g_map_7seg + right));
}

void NanoProtoShield::shiftLedWrite(byte b){
  m_shiftLed = b;
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift7segRight); //this gets shifted to the second display, send it first
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shift7segLeft);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, m_shiftLed);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

void NanoProtoShield::shiftClear(){
  shift7segWrite(0x00,0x00);
  shiftLedWrite(0x00);
}

void NanoProtoShield::shiftTestSequence(int wait){
  shiftLedWrite(0b10101010);
  delay(wait);
  shiftLedWrite(0b01010101);
  delay(wait);
  shiftLedWrite(0x00);

  shift7segWrite(0b10101010, 0b10101010);
  delay(wait);
  shift7segWrite(0b01010101, 0b01010101);
  delay(wait);
  shift7segWrite(0b10101010, 0b01010101);
  delay(wait);
  shift7segWrite(0b01010101, 0b10101010);
  delay(wait);
  shift7segWrite(0x00, 0xFF);
  delay(wait);
  shift7segWrite(0xFF, 0x00);
  delay(wait);
  shiftClear();
}


void NanoProtoShield::takeTemperatureReading(){
  DallasTemperature tempSensor(&m_oneWire);
    
  //Start up the temperature sensor library
  tempSensor.begin();

  tempSensor.requestTemperatures();
  m_temperatureC = tempSensor.getTempCByIndex(0);
}


void NanoProtoShield::clearAllDisplays(DISPLAYS exceptions = DISPLAY_NONE){
  if ( !(exceptions & DISPLAY_SHIFT_LEDS) ) {
    shiftLedWrite(0x00);
    }
  if ( !(exceptions & DISPLAY_SHIFT_7SEG) ) {
    shift7segWrite(0x00,0x00);
  }
  if ( !(exceptions & DISPLAY_OLED) ) {
    m_oled.clearDisplay();
    m_oled.display();
  }
  if ( !(exceptions & DISPLAY_RGB_LEDS) ) {
    rgbClear();
  }
}

void NanoProtoShield::mpuCalculateOffsets(int wait){
  m_mpu.begin();
  delay(wait);
  m_mpu.calcGyroOffsets();
}

void NanoProtoShield::buttonCheckForEvent(){
  byte old_state = m_buttonState;

  m_buttonState = digitalRead(PIN_UP_BUTTON) << BUTTON_UP |
                   digitalRead(PIN_DOWN_BUTTON) << BUTTON_DOWN |
                   digitalRead(PIN_LEFT_BUTTON) << BUTTON_LEFT |
                   digitalRead(PIN_RIGHT_BUTTON) << BUTTON_RIGHT |
                   !digitalRead(PIN_ROT_ENC_BUTTON) << BUTTON_ROTARY;

  m_buttonPressed = m_buttonPressed | (m_buttonState &(~old_state));
  m_buttonReleased = m_buttonReleased | ((~m_buttonState) &(old_state)); 

  for(int i = 0; i < BUTTON_COUNT; i++){
    if(m_buttonPressEvents[i] && buttonPressed(i))
      m_buttonPressEvents[i]();
    else if(m_buttonReleaseEvents[i] && buttonReleased(i))
      m_buttonReleaseEvents[i]();
  }
}

bool NanoProtoShield::buttonPressed(BUTTON b, bool clear = true){
  if(bitRead(m_buttonPressed, b))
  {
    if(clear)
    {
      m_buttonPressed = bitClear(m_buttonPressed, b);
    }
    return true;
  }
  return false;
}

bool NanoProtoShield::buttonReleased(BUTTON b, bool clear = true){
  if(bitRead(m_buttonReleased, b))
  {
    if(clear)
    {
      m_buttonReleased = bitClear(m_buttonReleased, b);
    }
    return true;
  }
  return false;
}

void NanoProtoShield::buttonSetPressEvent(BUTTON b, void (*buttonEvent)(void)){
  m_buttonPressEvents[b] = buttonEvent; //TODO: add checking for 'b'
}

void NanoProtoShield::buttonSetReleaseEvent(BUTTON b, void (*buttonEvent)(void)){
  m_buttonReleaseEvents[b] = buttonEvent;
}

int incrementValueWithMaxRollover(int value, int max) {
  return (value + 1) % max;
}

int decrementValueWithMaxRollover(int value, int max) {
  if (--value < 0)
    value = max - 1;
  value %= max;
  return value;
}