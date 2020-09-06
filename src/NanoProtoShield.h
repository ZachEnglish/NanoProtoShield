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

//Declare class object to abstract and hide many of the complexities of the board
class NanoProtoShield {
    public:

    NanoProtoShield();

    void begin();

    Encoder             m_rotary_encoder;
    OneWire             m_one_wire;
    DallasTemperature   m_temp_sensor;
    MPU6050             m_mpu;
    Adafruit_SSD1306    m_oled_display;
    Adafruit_NeoPixel   m_RGB_Strip;

    void RGB_strip_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait);
    void RGB_strip_rainbow(int wait);
    void RGB_strip_clear(); //ZDE: Debating if this should be here or if it hides the underlying class too much

    void OLED_display(int clear_after);
    void OLED_clear();
    void OLED_print(const __FlashStringHelper *);

    float read_pot1();
    float read_pot2();
    float read_pot3();
    float read_photo();

    void MPU_calculate_offsets(int wait);

    void shift_7seg_write(byte left, byte right);
    void shift_led_write(byte b);
    byte shift_7seg_left_read();
    byte shift_7seg_right_read();
    byte shift_led_read();
    void shift_clear();
    void shift_test_sequence(int wait);

    void interrupt();

    private:
    byte                m_shift_7seg_left;
    byte                m_shift_7seg_right;
    byte                m_shift_led;
    volatile uint8_t    m_interrupt;
};



#endif //#ifndef NANOPROTO_H