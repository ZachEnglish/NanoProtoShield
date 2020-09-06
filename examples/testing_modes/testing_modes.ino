#include "NanoProtoShield.h"

//Declare Nano Proto Shield object
NanoProtoShield g_nps;

//timer used for gyro calculations
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
              MODE_TEMPERATURE_PRINT,
              MODE_6050_PRINT,
              MODE_ANALOG_PRINT,
              MODE_COUNT };
volatile MODES g_mode = MODE_RGB_COLOR_CHASE;


//Used to keep track of what bit is lit/unlit on the 7seg displays while manipulating them
int g_current_bit;


enum DISPLAYS { DISPLAY_NONE, DISPLAY_RGB_LEDS, DISPLAY_SHIFT_REGS, DISPLAY_OLED };
//due to the default argument being supplied, this function needs a forward declaration
void clear_all_displays(DISPLAYS exception = DISPLAY_NONE);


void setup() {
  g_current_bit = 0;
  
  g_nps.OLED_display(250);


  //Play with the shift registers
  shift_led(0b10101010);
  delay(250);
  shift_led(0b01010101);
  delay(250);

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


  //Set up the MPU6050 Gyro/Accel - Show DO NOT MOVE message on OLED dispaly
  clear_all_displays(DISPLAY_OLED);

  g_nps.OLED_print(F("Calculating gyro offset, do not move MPU6050"));
  g_nps.MPU_calculate_offsets(1000);
  g_nps.OLED_clear();

  //Start up the temperature sensor library
  g_nps.m_temp_sensor.begin();

  clear_all_displays();

  //attach the ISR to the UP button.
  attachInterrupt( digitalPinToInterrupt(PIN_UP_BUTTON), increment_mode, FALLING );
} //end setup()


void loop() {
  byte b;
  byte bn;
  
  switch (g_mode)
  {
    case MODE_RGB_COLOR_CHASE:
      clear_all_displays(DISPLAY_RGB_LEDS);

      g_nps.RGB_strip_color_wipe(255,   0,   0, 50); // Red
      g_nps.RGB_strip_color_wipe(  0, 255,   0, 50); // Green
      g_nps.RGB_strip_color_wipe(  0,   0, 255, 50); // Blue
      break;

    case MODE_RGB_RAINBOW:
      clear_all_displays(DISPLAY_RGB_LEDS);

      g_nps.RGB_strip_rainbow(1);
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
      g_current_bit = (g_nps.m_rotary_encoder.read()/4)%8;
      b = 1 << abs(g_current_bit);
      bn = b ^ 0xFF;

      shift_7seg(b, bn);
      delay(100);
      break;

    case MODE_OLED_HELLO_WORLD:
      clear_all_displays(DISPLAY_OLED);

      g_nps.OLED_print(F("Hello world!"));
      break;

    case MODE_OLED_LOVE_MY_WIFE:
      clear_all_displays(DISPLAY_OLED);
      g_nps.OLED_print(F("I enjoy this!\nI love my wife!"));
      break;

    case MODE_TEMPERATURE_PRINT:
      clear_all_displays(DISPLAY_OLED);

      g_nps.m_temp_sensor.requestTemperatures();
      
      g_nps.OLED_print(
        "Temperature is:\n"
        + (String) g_nps.m_temp_sensor.getTempCByIndex(0) + "C\n"
        + (String) g_nps.m_temp_sensor.getTempFByIndex(0) + "F"
        );
      break;

    case MODE_6050_PRINT:
      clear_all_displays(DISPLAY_OLED);
      g_nps.m_mpu.update();

      if(millis() - g_timer > 1000){ // print data every second
        g_nps.OLED_print(
          "TEMP : " + (String) g_nps.m_mpu.getTemp()
          + "\nACC X : " + (String) g_nps.m_mpu.getAccX()
          + "\tY : " + (String) g_nps.m_mpu.getAccY()
          + "\tZ : " + (String) g_nps.m_mpu.getAccZ()
          + "\nGYRO X : " + (String) g_nps.m_mpu.getGyroX()
          + "\tY : " + (String) g_nps.m_mpu.getGyroY()
          + "\tZ : " + (String) g_nps.m_mpu.getGyroZ()
          + "\nACC ANG X : " + (String) g_nps.m_mpu.getAccAngleX()
          + "\tY : " + (String) g_nps.m_mpu.getAccAngleY()
          + "\nANGLE X : " + (String) g_nps.m_mpu.getAngleX()
          + "\tY : " + (String) g_nps.m_mpu.getAngleY()
          + "\tZ : " + (String) g_nps.m_mpu.getAngleZ()
        );
        g_timer = millis();
      }
      break;

    case MODE_ANALOG_PRINT:
      clear_all_displays(DISPLAY_OLED);
      
      g_nps.OLED_print(
        "POT1(V): " + (String) g_nps.read_pot1() + "V\n" +
        "POT2(V): " + (String) g_nps.read_pot2() + "V\n" +
        "POT3(V): " + (String) g_nps.read_pot3() + "V\n" +
        "PHOTO(V): " + (String) g_nps.read_photo() + "V\n"
        );
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


void clear_RGB() {
  g_nps.RGB_strip_clear();
}


void clear_all_displays(DISPLAYS exception = DISPLAY_NONE) {
  if ( exception != DISPLAY_SHIFT_REGS ) {
    clear_shifts();
  }
  if ( exception != DISPLAY_OLED ) {
    clear_OLED();
  }
  if ( exception != DISPLAY_RGB_LEDS ) {
    clear_RGB();
  }
}
