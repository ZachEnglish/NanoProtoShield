
#include <Adafruit_NeoPixel.h> //for talking to RGB LEDs
#include <SPI.h> //for talking to OLED display
#include <Wire.h> //for talking to OLED display
#include <Adafruit_GFX.h> //for talking to OLED display
#include <Adafruit_SSD1306.h> //for talking to OLED display
#include "Adafruit_LEDBackpack.h" //for talking to the 4x14 segment display over I2C
#include <Encoder.h> //for talking to the rotary encoded
#include <OneWire.h> //for talking to one wire devices like the temperature sensor.
    //Needed to modify the library to enable internal pull-up. See https://github.com/bigjosh/OneWireNoResistor/commit/ebba80cf61920aef399efa252826b1b59feb6589?branch=ebba80cf61920aef399efa252826b1b59feb6589&diff=split
#include <DallasTemperature.h> //for talking to the temperature sensor
#include <MPU6050_light.h> //for talking to the gyro/accelerometer


// ------------- BOARD SPECIFIC DEFINES -------------
//For version 3.0
//Button and rotary encoder pins. Only 2 & 3 can have interrupts on the Nano
#define PIN_LEFT_BUTTON     5
#define PIN_RIGHT_BUTTON    6
#define PIN_UP_BUTTON       3
#define PIN_DOWN_BUTTON     4
#define PIN_ROT_ENC_BUTTON  8
#define PIN_ROT_ENC_A       2
#define PIN_ROT_ENC_B       7
#define PIN_SHIFT_LATCH     10
#define PIN_SHIFT_CLOCK     11
#define PIN_SHIFT_DATA      13
#define PIN_TEMPERATURE     A2
#define PIN_LED             12
#define PIN_POT1            A0
#define PIN_POT2            A1
#define PIN_POT3            A6
#define PIN_PHOTO           A7
#define PIN_IR_RX           A3
#define PIN_IR_TX           9


// ------------- RGB LED SETUP -------------
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 8
// Declare our NeoPixel strip object:
Adafruit_NeoPixel g_strip(LED_COUNT, PIN_LED, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


// ------------- OLED DISPLAY SETUP -------------
#define OLED_SCREEN_WIDTH 128 // OLED display width, in pixels
#define OLED_SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 g_oled_display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);


// ------------- 4x14SEG DISPLAY SETUP -------------
Adafruit_AlphaNum4 g_alpha4 = Adafruit_AlphaNum4();


// ------------- ROTARY ENCODER SETUP -------------
Encoder g_rot_enc(PIN_ROT_ENC_A, PIN_ROT_ENC_B);


// ------------- OneWire TEMPERATURE SETUP -------------
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire g_oneWire(PIN_TEMPERATURE);
DallasTemperature g_temp_sensor(&g_oneWire);


// ------------- MPU6050/GYRO/ACCEL SETUP -------------
MPU6050 g_mpu(Wire);
long g_timer = 0;


// ------------- MODE DEFINITIONS & DECLARATION -------------
//What mode is our program in. Exercises different pieces of the board in different modes.
enum MODES {  MODE_RGB_COLOR_CHASE,
              MODE_RGB_RAINBOW,
              MODE_SHIFT_LEDS,
              MODE_SHIFT_7SEG,
              MODE_ROT_ENC,
              MODE_OLED_HELLO_WORLD,
              MODE_OLED_LOVE_MY_WIFE,
              MODE_ALPHANUM_COOL,
              MODE_TEMPERATURE_PRINT,
              MODE_6050_PRINT,
              MODE_ANALOG_PRINT,
              MODE_COUNT };
volatile MODES g_mode = MODE_RGB_COLOR_CHASE;


//Used to keep track of what bit is lit/unlit on the 7seg displays while manipulating them
int g_current_bit;


enum DISPLAYS { DISPLAY_NONE, DISPLAY_RGB_LEDS, DISPLAY_SHIFT_REGS, DISPLAY_OLED, DISPLAY_ALPHANUM };
//due to the default argument being supplied, this function needs a forward declaration
void clear_all_displays(DISPLAYS exception = DISPLAY_NONE);


void setup() {
  g_current_bit = 0;
  
  //Set up the OLED display
  if (!g_oled_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  g_oled_display.display();
  delay(250);

  // Clear the buffer
  g_oled_display.clearDisplay();


  //Play with the shift registers
  pinMode(PIN_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_SHIFT_CLOCK, OUTPUT);
  pinMode(PIN_SHIFT_DATA, OUTPUT);

  shift_led(0b10101010);
  delay(250);
  shift_led(0b01010101);
  delay(250);

  shift_7seg(0b10101010, 0b10101010);
  delay(250);
  shift_7seg(0b01010101, 0b01010101);
  delay(250);


  clear_shifts();


  //Set up the RGB strip
  g_strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  g_strip.show();            // Turn OFF all pixels ASAP
  g_strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)


  //Set up the quad alphanum (I2C)
  g_alpha4.begin(0x70);  // pass in the address
  g_alpha4.clear();
  g_alpha4.writeDisplay();


  //Set up the MPU6050 Gyro/Accel - Show DO NOT MOVE message on OLED dispaly
  clear_all_displays(DISPLAY_OLED);

  g_oled_display.clearDisplay();
  g_oled_display.setTextSize(1);
  g_oled_display.setTextColor(SSD1306_WHITE);
  g_oled_display.setCursor(0, 0);

  g_oled_display.println(F("Calculating gyro offset, do not move MPU6050"));
  g_oled_display.display();
  g_mpu.begin();
  delay(1000);
  g_mpu.calcGyroOffsets();
  g_oled_display.clearDisplay();
  g_oled_display.display();


  //Start up the temperature sensor library
  g_temp_sensor.begin();


  //Set up the buttons as imputs
  pinMode(PIN_LEFT_BUTTON, INPUT);
  pinMode(PIN_RIGHT_BUTTON, INPUT);
  pinMode(PIN_UP_BUTTON, INPUT);
  pinMode(PIN_DOWN_BUTTON, INPUT);
  pinMode(PIN_ROT_ENC_BUTTON, INPUT);
  pinMode(PIN_ROT_ENC_A, INPUT);
  pinMode(PIN_ROT_ENC_B, INPUT);


  clear_all_displays();


  //attach the ISR to the UP button.
  attachInterrupt( digitalPinToInterrupt(PIN_UP_BUTTON), increment_mode, FALLING );
} //end setup()


void loop() {
  byte b;
  byte bn;
  float pot1, pot2, pot3, photo;
  
  switch (g_mode)
  {
    case MODE_RGB_COLOR_CHASE:
      clear_all_displays(DISPLAY_RGB_LEDS);

      colorWipe(g_strip.Color(255,   0,   0), 50); // Red
      colorWipe(g_strip.Color(  0, 255,   0), 50); // Green
      colorWipe(g_strip.Color(  0,   0, 255), 50); // Blue
      break;

    case MODE_RGB_RAINBOW:
      clear_all_displays(DISPLAY_RGB_LEDS);

      rainbow(1);
      break;

    case MODE_SHIFT_LEDS:
      clear_all_displays(DISPLAY_SHIFT_REGS);

      //Play with the shift registers
      shift_led(0b10101010);
      delay(250);

      shift_led(0b01010101);
      delay(250);
      break;

    case MODE_SHIFT_7SEG:
      clear_all_displays(DISPLAY_SHIFT_REGS);

      g_current_bit = (g_current_bit + 1)%8;
      b = 1 << g_current_bit;
      bn = b ^ 0xFF;

      shift_7seg(b, bn);
      delay(100);
      break;

    case MODE_ROT_ENC:
      clear_all_displays(DISPLAY_SHIFT_REGS);
      g_current_bit = (g_rot_enc.read()/4)%8;
      b = 1 << abs(g_current_bit);
      bn = b ^ 0xFF;

      shift_7seg(b, bn);
      delay(100);
      break;

    case MODE_OLED_HELLO_WORLD:
      clear_all_displays(DISPLAY_OLED);

      g_oled_display.clearDisplay();
      g_oled_display.setTextSize(1);
      g_oled_display.setTextColor(SSD1306_WHITE);
      g_oled_display.setCursor(0, 0);

      g_oled_display.println(F("Hello world!"));
      g_oled_display.display();
      break;

    case MODE_OLED_LOVE_MY_WIFE:
      clear_all_displays(DISPLAY_OLED);

      g_oled_display.clearDisplay();
      g_oled_display.setTextSize(1);
      g_oled_display.setTextColor(SSD1306_WHITE);
      g_oled_display.setCursor(0, 0);

      g_oled_display.println(F("I enjoy this!\n"));
      g_oled_display.println(F("I love my wife!"));
      g_oled_display.display();
      break;

    case MODE_ALPHANUM_COOL:
      clear_all_displays(DISPLAY_ALPHANUM);

      g_alpha4.clear();
      g_alpha4.writeDigitAscii(0, 'C');
      g_alpha4.writeDigitAscii(1, 'o');
      g_alpha4.writeDigitAscii(2, 'o');
      g_alpha4.writeDigitAscii(3, 'l');
      g_alpha4.writeDisplay();
      break;

    case MODE_TEMPERATURE_PRINT:
      clear_all_displays(DISPLAY_OLED);

      g_temp_sensor.requestTemperatures();
      
      g_oled_display.clearDisplay();
      g_oled_display.setTextSize(1);
      g_oled_display.setTextColor(SSD1306_WHITE);
      g_oled_display.setCursor(0, 0);
      
      g_oled_display.println(F("Temperature is:\n"));
      g_oled_display.println(g_temp_sensor.getTempCByIndex(0));
      g_oled_display.display();
      break;

    case MODE_6050_PRINT:
      clear_all_displays(DISPLAY_OLED);
      g_mpu.update();

      if(millis() - g_timer > 1000){ // print data every second
        g_oled_display.clearDisplay();
        g_oled_display.setTextSize(1);
        g_oled_display.setTextColor(SSD1306_WHITE);
        g_oled_display.setCursor(0, 0);
        //g_oled_display.print(F("TEMPERATURE  : ")); g_oled_display.println(g_mpu.getTemp()); //commented out because it is ONE too many vertical lines and angle is more important
        g_oled_display.print(F("ACCELERO   X : ")); g_oled_display.print(g_mpu.getAccX());
        g_oled_display.print("\tY : "); g_oled_display.print(g_mpu.getAccY());
        g_oled_display.print("\tZ : "); g_oled_display.println(g_mpu.getAccZ());
      
        g_oled_display.print(F("GYRO       X : ")); g_oled_display.print(g_mpu.getGyroX());
        g_oled_display.print("\tY : "); g_oled_display.print(g_mpu.getGyroY());
        g_oled_display.print("\tZ : "); g_oled_display.println(g_mpu.getGyroZ());
      
        g_oled_display.print(F("ACC ANGLE  X : ")); g_oled_display.print(g_mpu.getAccAngleX());
        g_oled_display.print("\tY : "); g_oled_display.println(g_mpu.getAccAngleY());
        
        g_oled_display.print(F("ANGLE      X : ")); g_oled_display.print(g_mpu.getAngleX());
        g_oled_display.print("\tY : "); g_oled_display.print(g_mpu.getAngleY());
        g_oled_display.print("\tZ : "); g_oled_display.println(g_mpu.getAngleZ());
        g_oled_display.display();
        g_timer = millis();
      }
      break;

    case MODE_ANALOG_PRINT:
      clear_all_displays(DISPLAY_OLED);
      
      //get the current values and convert to voltage
      pot1 = analogRead(PIN_POT1) * (5.0 / 1023.0);
      pot2 = analogRead(PIN_POT2) * (5.0 / 1023.0);
      pot3 = analogRead(PIN_POT3) * (5.0 / 1023.0);
      photo = analogRead(PIN_PHOTO) * (5.0 / 1023.0);
      
      g_oled_display.clearDisplay();
      g_oled_display.setTextSize(1);
      g_oled_display.setTextColor(SSD1306_WHITE);
      g_oled_display.setCursor(0, 0);
      g_oled_display.print(F("POT1(V): ")); g_oled_display.println(pot1);
      g_oled_display.print(F("POT2(V): ")); g_oled_display.println(pot2);
      g_oled_display.print(F("POT3(V): ")); g_oled_display.println(pot3);
      g_oled_display.print(F("PHOTO(V): ")); g_oled_display.println(photo);
      g_oled_display.display();
      break;
  }//end switch
}//end loop()


// ISR to walk through the modes
void increment_mode() {
  g_mode = (g_mode + 1) % MODE_COUNT;
}


//set the LEDs via shift register. high lights up, low turns off. Clears the 7seg display while at it
void shift_led(byte b) {
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, 0xFF);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, 0xFF);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, b);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}


//set the 7seg via shift register. low lights up, high turns off. Clears the led display while at it
void shift_7seg(byte b1, byte b2) {
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, b2); //this gets shifted to the second display, send it first
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, b1);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, 0x00);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}


//clears both the shift register displays
void clear_shifts() {
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, 0xFF);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, 0xFF);
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, 0x00);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}


void clear_OLED() {
  g_oled_display.clearDisplay();
  g_oled_display.display();
}


void clear_alphanum() {
  g_alpha4.clear();
  g_alpha4.writeDisplay();
}


void clear_RGB() {
  g_strip.clear();
  g_strip.show();
}


void clear_all_displays(DISPLAYS exception = DISPLAY_NONE) {
  if ( exception != DISPLAY_SHIFT_REGS ) {
    clear_shifts();
  }
  if ( exception != DISPLAY_OLED ) {
    clear_OLED();
  }
  if ( exception != DISPLAY_ALPHANUM ) {
    clear_alphanum();
  }
  if ( exception != DISPLAY_RGB_LEDS ) {
    clear_RGB();
  }
}


//----------------- Below is all borrowed code from library examples -----------------


// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < g_strip.numPixels(); i++) { // For each pixel in strip...
    g_strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    g_strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}


// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < g_strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / g_strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      g_strip.setPixelColor(i, g_strip.gamma32(g_strip.ColorHSV(pixelHue)));
    }
    g_strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}
