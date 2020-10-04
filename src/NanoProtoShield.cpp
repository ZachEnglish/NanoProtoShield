#include "Arduino.h"
#include "NanoProtoShield.h"

//Mapping for 7 segment display. dp->decimal point
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

NanoProtoShield::NanoProtoShield(FEATURES features = FEATURE_ALL) {
  //Initialize the class's memory of what data the shift registers have
  m_shift7segLeft  = 0xFF;
  m_shift7segRight = 0xFF;
  m_shiftLed       = 0x00;
  m_interrupt      = 0;
  m_buttonState    = 0;
  m_buttonPressed  = 0;
  m_temperatureC   = 0;

  m_features = features;
}

NanoProtoShield::~NanoProtoShield() {
  if(m_oled)
    delete(m_oled);
  if(m_rotary)
    delete(m_rotary);
  if(m_oneWire)
    delete(m_oneWire);
  if(m_tempSensor)
    delete(m_tempSensor);
  if(m_mpu)
    delete(m_mpu);
  if(m_rgb)
    delete(m_rgb);
  
}

void NanoProtoShield::begin(INDEX_PINS pinout[] = NULL){
  if(pinout) {
    for(int i = 0; i < INDEX_PIN_COUNT; i++) {
      m_pinout[i] = pinout[i];
    }
  } else {
    initializePinIndexToDefault(m_pinout);
  }

  if( m_features & FEATURE_OLED ){
    m_oled = new Adafruit_SSD1306(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
  }
  if( m_features & FEATURE_ROTARY_TWIST ) {
    m_rotary = new Encoder(getPin(INDEX_PIN_ROT_ENC_B), getPin(INDEX_PIN_ROT_ENC_A)); //this order makes clockwise positive on the NPS
  }
  m_oneWire = NULL;
  m_tempSensor = NULL;
  if( m_features & FEATURE_MPU ) {
    m_mpu = new MPU6050(Wire);
  }
  if( m_features & FEATURE_RGB ) {
    m_rgb = new Adafruit_NeoPixel(RGB_LED_COUNT, getPin(INDEX_PIN_RGB_LED), NEO_GRB + NEO_KHZ800);
  }

  for(int i = 0; i < BUTTON_COUNT; i++){
    m_buttonPressEvents[i] = NULL;
    m_buttonReleaseEvents[i] = NULL;
  }

  //Set up shift register pins
  if(m_features & FEATURE_SHIFTS){
    pinMode(getPin(INDEX_PIN_SHIFT_LATCH), OUTPUT);
    pinMode(getPin(INDEX_PIN_SHIFT_CLOCK), OUTPUT);
    pinMode(getPin(INDEX_PIN_SHIFT_DATA), OUTPUT);
  }

  //Set up the buttons as imputs
  if(m_features & FEATURE_BUTTON_LEFT){
    pinMode(getPin(INDEX_PIN_LEFT_BUTTON), INPUT);
  }
  if(m_features & FEATURE_BUTTON_RIGHT){
    pinMode(getPin(INDEX_PIN_RIGHT_BUTTON), INPUT);
  }
  if(m_features & FEATURE_BUTTON_UP){
    pinMode(getPin(INDEX_PIN_UP_BUTTON), INPUT);
  }
  if(m_features & FEATURE_BUTTON_DOWN){
    pinMode(getPin(INDEX_PIN_DOWN_BUTTON), INPUT);
  }
  if(m_features & FEATURE_ROT_ENC_BUTTON){
    pinMode(getPin(INDEX_PIN_ROT_ENC_BUTTON), INPUT);
  }
  if(m_features & FEATURE_ROTARY_TWIST){
    pinMode(getPin(INDEX_PIN_ROT_ENC_A), INPUT);
    pinMode(getPin(INDEX_PIN_ROT_ENC_B), INPUT);
  }

  //Set up the RGB strip
  if(m_rgb){
    m_rgb->begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    m_rgb->show();            // Turn OFF all pixels ASAP
    m_rgb->setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  }

  // Set up the OLED display
  // This begin() call tries to put 1024 bytes on the stack for the display buffer, which
  // is half of the entire stack! Easy to blow it.
  if(m_oled){
    if (!m_oled->begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for (;;); // Don't proceed, loop forever
    }
  
    m_oled->setTextSize(1);
    m_oled->setTextColor(SSD1306_WHITE);
  }

  clearAllDisplays();
}


void NanoProtoShield::rgbColorWipe(uint8_t r, uint8_t g, uint8_t b, int wait) {
  if(m_rgb){
    uint8_t starting_interrupt = m_interrupt;
    uint32_t color = m_rgb->Color(r,g,b);
    for (int i = 0; i < m_rgb->numPixels(); i++) { // For each pixel in strip...
      m_rgb->setPixelColor(i, color);         //  Set pixel's color (in RAM)
      m_rgb->show();                          //  Update strip to match
      if(m_rgbInterrupt)
        buttonCheckForEvent();
      if(m_interrupt != starting_interrupt)
        return;
      delay(wait);                           //  Pause for a moment
    }
  }//if(m_rgb)
}

void NanoProtoShield::rgbRainbow(int wait) {
  if(m_rgb){
    uint8_t starting_interrupt = m_interrupt;
    // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this outer loop:
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
      for (int i = 0; i < m_rgb->numPixels(); i++) { // For each pixel in strip...
        // Offset pixel hue by an amount to make one full revolution of the
        // color wheel (range of 65536) along the length of the strip
        // (strip.numPixels() steps):
        int pixelHue = firstPixelHue + (i * 65536L / m_rgb->numPixels());
        // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
        // optionally add saturation and value (brightness) (each 0 to 255).
        // Here we're using just the single-argument hue variant. The result
        // is passed through strip.gamma32() to provide 'truer' colors
        // before assigning to each pixel:
        m_rgb->setPixelColor(i, m_rgb->gamma32(m_rgb->ColorHSV(pixelHue)));
      }
      m_rgb->show(); // Update strip with new contents
      if(m_rgbInterrupt)
        buttonCheckForEvent();
      if(m_interrupt != starting_interrupt)
        return;
      delay(wait);  // Pause for a moment
    }
  }//if(m_rgb)
}

void NanoProtoShield::rgbClear() {
  if(m_rgb){
    m_rgb->clear();
    m_rgb->show();
  }
}

void NanoProtoShield::rgbSetPixelsColor(uint8_t r, uint8_t g, uint8_t b){
  if(m_rgb){
    for(int i = 0; i < RGB_LED_COUNT; i++)
      rgbSetPixelColor(i,r,g,b);
  }
}

void NanoProtoShield::rgbSetPixelsColor(uint32_t color){
  if(m_rgb){
    for(int i = 0; i < RGB_LED_COUNT; i++)
      rgbSetPixelColor(i,color);
  }
}


void NanoProtoShield::oledDisplay(int clear_after = 0) {
  if(m_oled){
    m_oled->display();

    if(clear_after > 0)
    {
      delay(clear_after);
      m_oled->clearDisplay();
    }
  }//if(m_oled)
}

void NanoProtoShield::oledClear() {
  if(m_oled){
    m_oled->clearDisplay();
    m_oled->setCursor(0, 0);
  }
}


void NanoProtoShield::shift7segWrite(byte left, byte right){
  if(m_features & FEATURE_SHIFTS) {
    //invert the input because the 7 segment leds are active low
    m_shift7segLeft = ~left;
    m_shift7segRight = ~right;
    digitalWrite(getPin(INDEX_PIN_SHIFT_LATCH), LOW);
    shiftOut(getPin(INDEX_PIN_SHIFT_DATA), getPin(INDEX_PIN_SHIFT_CLOCK), MSBFIRST, m_shift7segRight); //this gets shifted to the second display, send it first
    shiftOut(getPin(INDEX_PIN_SHIFT_DATA), getPin(INDEX_PIN_SHIFT_CLOCK), MSBFIRST, m_shift7segLeft);
    shiftOut(getPin(INDEX_PIN_SHIFT_DATA), getPin(INDEX_PIN_SHIFT_CLOCK), MSBFIRST, m_shiftLed);
    digitalWrite(getPin(INDEX_PIN_SHIFT_LATCH), HIGH);
  }
}

void NanoProtoShield::shift7segPrint(uint8_t num){
  if(m_features & FEATURE_SHIFTS) {
    byte left, right;
    right = num % 10;
    right = pgm_read_byte_near(g_map_7seg + right) | ((num>=100)?DEC_7SEG:0x00); //get the digit and determine if the decimal is needed
    left = (num / 10) % 10;
    left = pgm_read_byte_near(g_map_7seg + left) | ((num>=200)?DEC_7SEG:0x00);
    
    shift7segWrite(left, right);
  }
}

void NanoProtoShield::shift7segPrintHex(uint8_t num){
  if(m_features & FEATURE_SHIFTS) {
    byte left, right;
    right = num & 0x0F;
    left = (num & 0xF0)>>4;
    
    shift7segWrite(pgm_read_byte_near(g_map_7seg + left),pgm_read_byte_near(g_map_7seg + right));
  }
}

void NanoProtoShield::shiftLedWrite(byte b){
  if(m_features & FEATURE_SHIFTS) {
    m_shiftLed = b;
    digitalWrite(getPin(INDEX_PIN_SHIFT_LATCH), LOW);
    shiftOut(getPin(INDEX_PIN_SHIFT_DATA), getPin(INDEX_PIN_SHIFT_CLOCK), MSBFIRST, m_shift7segRight); //this gets shifted to the second display, send it first
    shiftOut(getPin(INDEX_PIN_SHIFT_DATA), getPin(INDEX_PIN_SHIFT_CLOCK), MSBFIRST, m_shift7segLeft);
    shiftOut(getPin(INDEX_PIN_SHIFT_DATA), getPin(INDEX_PIN_SHIFT_CLOCK), MSBFIRST, m_shiftLed);
    digitalWrite(getPin(INDEX_PIN_SHIFT_LATCH), HIGH);
  }
}

void NanoProtoShield::shiftClear(){
  if(m_features & FEATURE_SHIFTS) {
    shift7segWrite(0x00,0x00);
    shiftLedWrite(0x00);
  }
}

void NanoProtoShield::shiftTestSequence(int wait){
  if(m_features & FEATURE_SHIFTS) {
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
}


void NanoProtoShield::takeTemperatureReading(){
  if(!m_oneWire){
    m_oneWire = new OneWire(getPin(INDEX_PIN_TEMPERATURE));
  }
  if(!m_tempSensor){
    m_tempSensor = new DallasTemperature(m_oneWire);
  }
    
  //Start up the temperature sensor library
  m_tempSensor->begin();

  m_tempSensor->requestTemperatures();
  m_temperatureC = m_tempSensor->getTempCByIndex(0);

  delete(m_oneWire);
  m_oneWire = NULL;
  delete(m_tempSensor);
  m_tempSensor = NULL;
}


void NanoProtoShield::clearAllDisplays(DISPLAYS exceptions = DISPLAY_NONE){
  if ( !(exceptions & DISPLAY_SHIFT_LEDS) ) {
    shiftLedWrite(0x00);
    }
  if ( !(exceptions & DISPLAY_SHIFT_7SEG) ) {
    shift7segWrite(0x00,0x00);
  }
  if ( !(exceptions & DISPLAY_OLED) ) {
    if(m_oled){
      m_oled->clearDisplay();
      m_oled->display();
    }
  }
  if ( !(exceptions & DISPLAY_RGB_LEDS) ) {
    rgbClear();
  }
}

void NanoProtoShield::mpuCalculateOffsets(int wait){
  if(m_mpu){
    m_mpu->begin();
    delay(wait);
    m_mpu->calcGyroOffsets();
  }
}

void NanoProtoShield::buttonCheckForEvent(){
  byte old_state = m_buttonState;

  m_buttonState = 0;

  if(m_features & FEATURE_BUTTON_UP){
    m_buttonState |= digitalRead(getPin(INDEX_PIN_UP_BUTTON)) << BUTTON_UP;
  }
  if(m_features & FEATURE_BUTTON_DOWN){
    m_buttonState |= digitalRead(getPin(INDEX_PIN_DOWN_BUTTON)) << BUTTON_DOWN;
  }
  if(m_features & FEATURE_BUTTON_LEFT){
    m_buttonState |= digitalRead(getPin(INDEX_PIN_LEFT_BUTTON)) << BUTTON_LEFT;
  }
  if(m_features & FEATURE_BUTTON_RIGHT){
    m_buttonState |= digitalRead(getPin(INDEX_PIN_RIGHT_BUTTON)) << BUTTON_RIGHT;
  }
  if(m_features & FEATURE_ROT_ENC_BUTTON){
    m_buttonState |= !digitalRead(getPin(INDEX_PIN_ROT_ENC_BUTTON)) << BUTTON_ROTARY;//Hardware has this input inverted (active high)
  }

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

void initializePinIndexToDefault(INDEX_PINS *iPen) {
  if(iPen){
    iPen[INDEX_PIN_LEFT_BUTTON]    = PIN_DEFAULT_LEFT_BUTTON;
    iPen[INDEX_PIN_RIGHT_BUTTON]   = PIN_DEFAULT_RIGHT_BUTTON;
    iPen[INDEX_PIN_UP_BUTTON]      = PIN_DEFAULT_UP_BUTTON;
    iPen[INDEX_PIN_DOWN_BUTTON]    = PIN_DEFAULT_DOWN_BUTTON;
    iPen[INDEX_PIN_ROT_ENC_BUTTON] = PIN_DEFAULT_ROT_ENC_BUTTON;
    iPen[INDEX_PIN_ROT_ENC_A]      = PIN_DEFAULT_ROT_ENC_A;
    iPen[INDEX_PIN_ROT_ENC_B]      = PIN_DEFAULT_ROT_ENC_B;
    iPen[INDEX_PIN_SHIFT_LATCH]    = PIN_DEFAULT_SHIFT_LATCH;
    iPen[INDEX_PIN_SHIFT_CLOCK]    = PIN_DEFAULT_SHIFT_CLOCK;
    iPen[INDEX_PIN_SHIFT_DATA]     = PIN_DEFAULT_SHIFT_DATA;
    iPen[INDEX_PIN_TEMPERATURE]    = PIN_DEFAULT_TEMPERATURE;
    iPen[INDEX_PIN_RGB_LED]        = PIN_DEFAULT_RGB_LED;
    iPen[INDEX_PIN_POT1]           = PIN_DEFAULT_POT1;
    iPen[INDEX_PIN_POT2]           = PIN_DEFAULT_POT2;
    iPen[INDEX_PIN_POT3]           = PIN_DEFAULT_POT3;
    iPen[INDEX_PIN_PHOTO]          = PIN_DEFAULT_PHOTO;
    iPen[INDEX_PIN_IR_RX]          = PIN_DEFAULT_IR_RX;
    iPen[INDEX_PIN_IR_TX]          = PIN_DEFAULT_IR_TX;
  }
}