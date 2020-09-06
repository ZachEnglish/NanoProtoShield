#ifndef NANOPROTOSHIELD_H
#define NANOPROTOSHIELD_H

#include "Arduino.h"
#include <Adafruit_NeoPixel.h> //for talking to RGB LEDs
#include <SPI.h> //for talking to OLED display
#include <Wire.h> //for talking to OLED display
#include <Adafruit_GFX.h> //for talking to OLED display
#include <Adafruit_SSD1306.h> //for talking to OLED display
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
#define PIN_RGB_LED         12
#define PIN_POT1            A0
#define PIN_POT2            A1
#define PIN_POT3            A6
#define PIN_PHOTO           A7
#define PIN_IR_RX           A3
#define PIN_IR_TX           9


// ------------- RGB LED DEFINES -------------
// How many NeoPixels are attached to the Arduino?
#define RGB_LED_COUNT 8


// ------------- OLED DISPLAY DEFINES -------------
#define OLED_SCREEN_WIDTH 128 // OLED display width, in pixels
#define OLED_SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_ADDRESS 0x3C


#define ANALOG_TO_VOLTAGE (5.0 / 1023.0)

enum DISPLAYS { DISPLAY_NONE, DISPLAY_RGB_LEDS, DISPLAY_SHIFT_LEDS, DISPLAY_SHIFT_7SEG, DISPLAY_OLED };

enum FEATURES {
    BUTTON_LEFT     = 1<<0,
    BUTTON_RIGHT    = 1<<1,
    BUTTON_UP       = 1<<2,
    BUTTON_DOWN     = 1<<3,
    ROT_ENC_BUTTON  = 1<<4,
    ROT_ENC         = 1<<5,
    SHIFT_OUT       = 1<<6,
    TEMPERATURE     = 1<<7,
    RGB_LED         = 1<<8,
    POT1            = 1<<9,
    POT2            = 1<<10,
    POT3            = 1<<11,
    PHOTO           = 1<<12,
    IR_RX           = 1<<13,
    IR_TX           = 1<<14,
    ALL_FEATURES    = BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN | ROT_ENC_BUTTON | ROT_ENC | SHIFT_OUT
                        | TEMPERATURE | RGB_LED | POT1 | POT2 | POT3 | PHOTO | IR_RX | IR_TX
    };

//Declare class object to abstract and hide many of the complexities of the board
class NanoProtoShield {
    public:

    NanoProtoShield(FEATURES feature_list = ALL_FEATURES);

    void begin();

    void RGB_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait);
    void RGB_rainbow(int wait);
    void RGB_clear(); //ZDE: Debating if this should be here or if it hides the underlying class too much
    void RGB_set_pixel_color(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b);
    void RGB_set_brightness(uint8_t brightness);
    void RGB_show();

    void OLED_display(int clear_after = 0);
    void OLED_clear();
    size_t OLED_print(const __FlashStringHelper *);
    size_t OLED_print(const String &);
    size_t OLED_print(const char[]);
    size_t OLED_print(char);
    size_t OLED_print(const Printable&);
    size_t OLED_println(const __FlashStringHelper *);
    size_t OLED_println(const String &s);
    size_t OLED_println(const char[]);
    size_t OLED_println(char);
    size_t OLED_println(const Printable&);
    size_t OLED_println(void);

    float pot1_read() {return analogRead(PIN_POT1) * ANALOG_TO_VOLTAGE;}
    float pot2_read() {return analogRead(PIN_POT2) * ANALOG_TO_VOLTAGE;}
    float pot3_read() {return analogRead(PIN_POT3) * ANALOG_TO_VOLTAGE;}
    float photo_read() {return analogRead(PIN_PHOTO) * ANALOG_TO_VOLTAGE;}

    int32_t rotary_encoder_read() {return m_rotary_encoder.read();}

    void MPU_calculate_offsets(int wait);

    void shift_7seg_write(byte left, byte right);
    void shift_led_write(byte b);
    byte shift_7seg_left_read();
    byte shift_7seg_right_read();
    byte shift_led_read();
    void shift_clear();
    void shift_test_sequence(int wait);

    void take_temperature_reading();
    float get_temp_C() {return m_temperature_C;}
    float get_temp_F() {return m_temperature_F;}

    void MPU_update();
    float MPU_get_temp(){ return m_mpu.getTemp(); };
    float MPU_get_acc_x(){ return m_mpu.getAccX(); };
    float MPU_get_acc_y(){ return m_mpu.getAccY(); };
    float MPU_get_acc_z(){ return m_mpu.getAccZ(); };
    float MPU_get_gyro_x(){ return m_mpu.getGyroX(); };
    float MPU_get_gyro_y(){ return m_mpu.getGyroY(); };
    float MPU_get_gyro_z(){ return m_mpu.getGyroZ(); };
    float MPU_get_acc_angle_x(){ return m_mpu.getAccAngleX(); };
    float MPU_get_acc_angle_y(){ return m_mpu.getAccAngleY(); };//yup, no Z
    float MPU_get_angle_x(){ return m_mpu.getAngleX(); };
    float MPU_get_angle_y(){ return m_mpu.getAngleY(); };
    float MPU_get_angle_z(){ return m_mpu.getAngleZ(); };

    void interrupt();

    void clear_all_displays(DISPLAYS exception = DISPLAY_NONE);

    //TODO
    //Need functions for all four buttons, rotary encoder twists and buttons
    //Need objects, functions, and test mode for IR
    //Need init way to use a reduced number of pins so some can be used for alternate functions

    private:
    Encoder             m_rotary_encoder;
    OneWire             m_one_wire;
    DallasTemperature   m_temp_sensor;
    MPU6050             m_mpu;
    Adafruit_SSD1306    m_oled_display;
    Adafruit_NeoPixel   m_RGB;

    byte                m_shift_7seg_left;
    byte                m_shift_7seg_right;
    byte                m_shift_led;
    volatile uint8_t    m_interrupt;
    float               m_temperature_C;
    float               m_temperature_F;
    FEATURES            m_enabled_features;
};



#endif //#ifndef NANOPROTO_H