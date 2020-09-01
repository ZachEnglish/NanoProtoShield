#include "NanoProtoShield.h"

//Declare our Nano Proto Shield object
NanoProtoShield g_nps;


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
  if (!g_nps.m_oled_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  g_nps.m_oled_display.display();
  delay(250);

  // Clear the buffer
  g_nps.m_oled_display.clearDisplay();


  //Play with the shift registers
  pinMode(PIN_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_SHIFT_CLOCK, OUTPUT);
  pinMode(PIN_SHIFT_DATA, OUTPUT);

  shift_led(0b10101010);
  delay(250);
  shift_led(0b01010101);
  delay(250);
  //shift_led(0b10101010);
  //delay(500);
  //shift_led(0b01010101);
  //delay(500);

  shift_7seg(0b10101010, 0b10101010);
  delay(250);
  shift_7seg(0b01010101, 0b01010101);
  delay(250);
  shift_7seg(0b10101010, 0b01010101);
  delay(250);
  shift_7seg(0b01010101, 0b10101010);
  delay(250);

  clear_shifts();


  //Set up the RGB strip
  g_nps.m_RGB_Strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  g_nps.m_RGB_Strip.show();            // Turn OFF all pixels ASAP
  g_nps.m_RGB_Strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)


  //Set up the quad alphanum (I2C)
  g_alpha4.begin(0x70);  // pass in the address
  g_alpha4.clear();
  g_alpha4.writeDisplay();


  //Set up the MPU6050 Gyro/Accel - Show DO NOT MOVE message on OLED dispaly
  clear_all_displays(DISPLAY_OLED);

  g_nps.m_oled_display.clearDisplay();
  g_nps.m_oled_display.setTextSize(1);
  g_nps.m_oled_display.setTextColor(SSD1306_WHITE);
  g_nps.m_oled_display.setCursor(0, 0);

  g_nps.m_oled_display.println(F("Calculating gyro offset, do not move MPU6050"));
  g_nps.m_oled_display.display();
  g_mpu.begin();
  delay(1000);
  g_mpu.calcGyroOffsets();
  g_nps.m_oled_display.clearDisplay();
  g_nps.m_oled_display.display();


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

      g_nps.RGB_Strip_Color_Wipe(255,   0,   0, 50); // Red
      g_nps.RGB_Strip_Color_Wipe(  0, 255,   0, 50); // Green
      g_nps.RGB_Strip_Color_Wipe(  0,   0, 255, 50); // Blue
      break;

    case MODE_RGB_RAINBOW:
      clear_all_displays(DISPLAY_RGB_LEDS);

      g_nps.RGB_Strip_Rainbow(1);
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

      g_nps.m_oled_display.clearDisplay();
      g_nps.m_oled_display.setTextSize(1);
      g_nps.m_oled_display.setTextColor(SSD1306_WHITE);
      g_nps.m_oled_display.setCursor(0, 0);

      g_nps.m_oled_display.println(F("Hello world!"));
      g_nps.m_oled_display.display();
      break;

    case MODE_OLED_LOVE_MY_WIFE:
      clear_all_displays(DISPLAY_OLED);

      g_nps.m_oled_display.clearDisplay();
      g_nps.m_oled_display.setTextSize(1);
      g_nps.m_oled_display.setTextColor(SSD1306_WHITE);
      g_nps.m_oled_display.setCursor(0, 0);

      g_nps.m_oled_display.println(F("I enjoy this!\n"));
      g_nps.m_oled_display.println(F("I love my wife!"));
      g_nps.m_oled_display.display();
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
      
      g_nps.m_oled_display.clearDisplay();
      g_nps.m_oled_display.setTextSize(1);
      g_nps.m_oled_display.setTextColor(SSD1306_WHITE);
      g_nps.m_oled_display.setCursor(0, 0);
      
      g_nps.m_oled_display.println(F("Temperature is:\n"));
      g_nps.m_oled_display.println(g_temp_sensor.getTempCByIndex(0));
      g_nps.m_oled_display.display();
      break;

    case MODE_6050_PRINT:
      clear_all_displays(DISPLAY_OLED);
      g_mpu.update();

      if(millis() - g_timer > 1000){ // print data every second
        g_nps.m_oled_display.clearDisplay();
        g_nps.m_oled_display.setTextSize(1);
        g_nps.m_oled_display.setTextColor(SSD1306_WHITE);
        g_nps.m_oled_display.setCursor(0, 0);
        //g_nps.m_oled_display.print(F("TEMPERATURE  : ")); g_nps.m_oled_display.println(g_mpu.getTemp()); //commented out because it is ONE too many vertical lines and angle is more important
        g_nps.m_oled_display.print(F("ACCELERO   X : ")); g_nps.m_oled_display.print(g_mpu.getAccX());
        g_nps.m_oled_display.print("\tY : "); g_nps.m_oled_display.print(g_mpu.getAccY());
        g_nps.m_oled_display.print("\tZ : "); g_nps.m_oled_display.println(g_mpu.getAccZ());
      
        g_nps.m_oled_display.print(F("GYRO       X : ")); g_nps.m_oled_display.print(g_mpu.getGyroX());
        g_nps.m_oled_display.print("\tY : "); g_nps.m_oled_display.print(g_mpu.getGyroY());
        g_nps.m_oled_display.print("\tZ : "); g_nps.m_oled_display.println(g_mpu.getGyroZ());
      
        g_nps.m_oled_display.print(F("ACC ANGLE  X : ")); g_nps.m_oled_display.print(g_mpu.getAccAngleX());
        g_nps.m_oled_display.print("\tY : "); g_nps.m_oled_display.println(g_mpu.getAccAngleY());
        
        g_nps.m_oled_display.print(F("ANGLE      X : ")); g_nps.m_oled_display.print(g_mpu.getAngleX());
        g_nps.m_oled_display.print("\tY : "); g_nps.m_oled_display.print(g_mpu.getAngleY());
        g_nps.m_oled_display.print("\tZ : "); g_nps.m_oled_display.println(g_mpu.getAngleZ());
        g_nps.m_oled_display.display();
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
      
      g_nps.m_oled_display.clearDisplay();
      g_nps.m_oled_display.setTextSize(1);
      g_nps.m_oled_display.setTextColor(SSD1306_WHITE);
      g_nps.m_oled_display.setCursor(0, 0);
      g_nps.m_oled_display.print(F("POT1(V): ")); g_nps.m_oled_display.println(pot1);
      g_nps.m_oled_display.print(F("POT2(V): ")); g_nps.m_oled_display.println(pot2);
      g_nps.m_oled_display.print(F("POT3(V): ")); g_nps.m_oled_display.println(pot3);
      g_nps.m_oled_display.print(F("PHOTO(V): ")); g_nps.m_oled_display.println(photo);
      g_nps.m_oled_display.display();
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
  g_nps.m_oled_display.clearDisplay();
  g_nps.m_oled_display.display();
}


void clear_alphanum() {
  g_alpha4.clear();
  g_alpha4.writeDisplay();
}


void clear_RGB() {
  g_nps.RGB_Strip_Clear();
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
