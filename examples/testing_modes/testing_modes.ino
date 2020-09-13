#include "NanoProtoShield.h"

//Declare Nano Proto Shield object
NanoProtoShield g_nps;

//timer used for gyro calculations
long g_timer = 0;

// ------------- MODE DEFINITIONS & DECLARATION -------------
//What mode is our program in. Exercises different pieces of the board in different modes.
enum MODES {  MODE_RGB_COLOR_CHASE,
              MODE_RGB_RAINBOW,
              MODE_RGB_PIXEL_SET,
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
uint8_t g_current_7seg_bit;

uint8_t g_current_RGB_bit;

int increment_value_with_max_rollover(int value, int max);
int decrement_value_with_max_rollover(int value, int max);

void setup() {
  Serial.begin(112500);
  //Start out at the beginning
  g_current_7seg_bit = 0;
  g_current_RGB_bit = 0;

  //Initialize the NanoProtoShield object
  g_nps.begin();

  //There will be the adafruit logo in memory on start up, this shows it briefly
  g_nps.OLED_display(250);

  g_nps.shift_test_sequence(250);

  //Set up the MPU6050 Gyro/Accel - Show DO NOT MOVE message on OLED dispaly
  g_nps.clear_all_displays();
  g_nps.OLED_print(F("Calculating gyro offset, do not move MPU6050"));
  g_nps.OLED_display();
  g_nps.MPU_calculate_offsets(1000);
  g_nps.clear_all_displays();

  //attach the ISR to the UP button.
  attachInterrupt( digitalPinToInterrupt(PIN_UP_BUTTON), isr_increment_mode, FALLING );
  g_nps.button_down_event_set(not_isr_decrement_mode);
} //end setup()


void loop() {
  byte b;
  byte bn;
  MODES starting_mode = g_mode;

  g_nps.button_check();

  // if(g_nps.button_pressed(BUTTON_DOWN))
  //   g_mode = decrement_value_with_max_rollover(g_mode, MODE_COUNT);
  
  switch (g_mode)
  {
    case MODE_RGB_COLOR_CHASE:
      g_nps.clear_all_displays(bit(DISPLAY_RGB_LEDS));

      if(starting_mode != g_mode)
        break;
      g_nps.RGB_color_wipe(255,   0,   0, 50); // Red
      g_nps.button_check();
      if(starting_mode != g_mode)
        break;
      g_nps.RGB_color_wipe(  0, 255,   0, 50); // Green
      g_nps.button_check();
      if(starting_mode != g_mode)
        break;
      g_nps.RGB_color_wipe(  0,   0, 255, 50); // Blue
      g_nps.button_check();
      break;

    case MODE_RGB_RAINBOW:
      g_nps.clear_all_displays(bit(DISPLAY_RGB_LEDS));

      g_nps.RGB_rainbow(1);
      break;

    case MODE_RGB_PIXEL_SET:
      g_nps.clear_all_displays(bit(DISPLAY_RGB_LEDS)|bit(DISPLAY_SHIFT_LEDS));

      g_nps.button_check();
      if(g_nps.button_pressed(BUTTON_RIGHT))
        g_current_RGB_bit = increment_value_with_max_rollover(g_current_RGB_bit, RGB_LED_COUNT);
      if(g_nps.button_pressed(BUTTON_LEFT))
        g_current_RGB_bit = decrement_value_with_max_rollover(g_current_RGB_bit, RGB_LED_COUNT);

      g_nps.shift_led_write(1<<g_current_RGB_bit);

      g_nps.RGB_set_pixel_color(g_current_RGB_bit,g_nps.pot1_read()*VOLTAGE_TO_RGB,g_nps.pot2_read()*VOLTAGE_TO_RGB,g_nps.pot3_read()*VOLTAGE_TO_RGB);
      g_nps.RGB_show();
      break;

    case MODE_SHIFT_LEDS:
      g_nps.clear_all_displays(bit(DISPLAY_SHIFT_LEDS));

      //Play with the shift registers
      g_nps.shift_led_write(0b10101010);
      delay(250);

      g_nps.shift_led_write(0b01010101);
      delay(250);
      break;

    case MODE_SHIFT_7SEG:
      g_nps.clear_all_displays(bit(DISPLAY_SHIFT_7SEG));

      g_current_7seg_bit = (g_current_7seg_bit + 1)%8;
      bitSet(b,g_current_7seg_bit);
      bn = ~b;

      g_nps.shift_7seg_write(b, bn);
      delay(150);
      break;

    case MODE_ROT_ENC:
      g_nps.clear_all_displays(bit(DISPLAY_SHIFT_7SEG));
      g_current_7seg_bit = (g_nps.rotary_encoder_read()/4)%8;
      bitSet(b,abs(g_current_7seg_bit));
      bn = ~b;

      g_nps.shift_7seg_write(b, bn);
      break;

    case MODE_OLED_HELLO_WORLD:
      g_nps.clear_all_displays(bit(DISPLAY_OLED));

      g_nps.OLED_print(F("Hello world!"));
      g_nps.OLED_display();
      break;

    case MODE_OLED_LOVE_MY_WIFE:
      g_nps.clear_all_displays(bit(DISPLAY_OLED));

      g_nps.OLED_println(F("I enjoy this!"));
      g_nps.OLED_println(F("I love my wife!"));
      g_nps.OLED_display();
      break;

    case MODE_TEMPERATURE_PRINT:
      g_nps.clear_all_displays(bit(DISPLAY_OLED));

      g_nps.take_temperature_reading();
      
      g_nps.OLED_println(F("Temperature is:"));
      g_nps.OLED_print((String)g_nps.get_temp_C());
      g_nps.OLED_println(F("C"));
      g_nps.OLED_print((String)g_nps.get_temp_F());
      g_nps.OLED_println(F("F"));
      g_nps.OLED_display();
      break;

    case MODE_6050_PRINT:
      g_nps.clear_all_displays(bit(DISPLAY_OLED));
      g_nps.MPU_update();

      if(millis() - g_timer > 1000){ // print data every second
        g_nps.OLED_print(F("TEMP: ")); g_nps.OLED_println((String)g_nps.MPU_get_temp());
        g_nps.OLED_print(F("ACC X: ")); g_nps.OLED_print((String)g_nps.MPU_get_acc_x());
        g_nps.OLED_print(F("\tY: ")); g_nps.OLED_print((String)g_nps.MPU_get_acc_y());
        g_nps.OLED_print(F("\tZ: ")); g_nps.OLED_println((String)g_nps.MPU_get_acc_z());
      
        g_nps.OLED_print(F("GYRO X: ")); g_nps.OLED_print((String)g_nps.MPU_get_gyro_x());
        g_nps.OLED_print(F("\tY: ")); g_nps.OLED_print((String)g_nps.MPU_get_gyro_y());
        g_nps.OLED_print(F("\tZ: ")); g_nps.OLED_println((String)g_nps.MPU_get_gyro_z());
      
        g_nps.OLED_print(F("X:")); g_nps.OLED_print((String)g_nps.MPU_get_acc_angle_x());
        g_nps.OLED_print(F("\tY:")); g_nps.OLED_println((String)g_nps.MPU_get_acc_angle_y());
        
        g_nps.OLED_print(F("ANGLE X: ")); g_nps.OLED_print((String)g_nps.MPU_get_angle_x());
        g_nps.OLED_print(F("\tY: ")); g_nps.OLED_print((String)g_nps.MPU_get_angle_y());
        g_nps.OLED_print(F("\tZ: ")); g_nps.OLED_println((String)g_nps.MPU_get_angle_z());
        g_nps.OLED_display();
        g_timer = millis();
      }
      break;

    case MODE_ANALOG_PRINT:
      g_nps.clear_all_displays(bit(DISPLAY_OLED));
      
      g_nps.OLED_print(F("POT1(V): ")); g_nps.OLED_println((String)g_nps.pot1_read());
      g_nps.OLED_print(F("POT2(V): ")); g_nps.OLED_println((String)g_nps.pot2_read());
      g_nps.OLED_print(F("POT3(V): ")); g_nps.OLED_println((String)g_nps.pot3_read());
      g_nps.OLED_print(F("PHOTO(V): ")); g_nps.OLED_println((String)g_nps.photo_read());
      g_nps.OLED_display();
      break;
  }//end switch
}//end loop()


// Interrupt Service Routine (ISR) to walk through the modes
void isr_increment_mode() {
  g_mode = increment_value_with_max_rollover(g_mode, MODE_COUNT);
  g_nps.interrupt();
}

int increment_value_with_max_rollover(int value, int max) {
  return (value + 1) % max;
}

int decrement_value_with_max_rollover(int value, int max) {
  if(--value < 0)
    value = max - 1;
  value %= max;
  return value;
}

void not_isr_decrement_mode() {
  //g_mode = decrement_value_with_max_rollover(g_mode, MODE_COUNT);
}