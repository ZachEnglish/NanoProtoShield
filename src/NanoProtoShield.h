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
#define VOLTAGE_TO_RGB (255.0 / 5.0)

enum DISPLAYS { 
    DISPLAY_RGB_LEDS    = bit(0),
    DISPLAY_SHIFT_LEDS  = bit(1),
    DISPLAY_SHIFT_7SEG  = bit(2),
    DISPLAY_OLED        = bit(3)
    };
#define DISPLAY_NONE 0

enum BUTTONS { BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT };

//Declare class object to abstract and hide many of the complexities of the board
class NanoProtoShield {
    public:

    NanoProtoShield();

    void begin();

    void rgbColorWipe(uint8_t r, uint8_t g, uint8_t b, int wait);
    void rgbRainbow(int wait);
    void rgbClear(); //ZDE: Debating if this should be here or if it hides the underlying class too much
    void rgbSetPixelColor(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b);
    void rgbSetPixelsColor(uint8_t r, uint8_t g, uint8_t b);
    void rgbSetPixelColor(uint8_t pixel, uint32_t color);
    void rgbSetPixelsColor(uint32_t color);
    void rgbSetBrightness(uint8_t brightness);
    void rgbShow();
    uint32_t rgbGetColorFromHsv(uint16_t hue, uint8_t sat=255, uint8_t val=255) {return Adafruit_NeoPixel::ColorHSV(hue,sat,val);}

    void oledDisplay(int clear_after = 0);
    void oledClear();
    size_t oledPrint(const __FlashStringHelper *);
    size_t oledPrint(const String &);
    size_t oledPrint(const char[]);
    size_t oledPrint(char);
    size_t oledPrint(const Printable&);
    size_t oledPrintln(const __FlashStringHelper *);
    size_t oledPrintln(const String &s);
    size_t oledPrintln(const char[]);
    size_t oledPrintln(char);
    size_t oledPrintln(const Printable&);
    size_t oledPrintln(void);

    float pot1Read() {return analogRead(PIN_POT1) * ANALOG_TO_VOLTAGE;}
    float pot2Read() {return analogRead(PIN_POT2) * ANALOG_TO_VOLTAGE;}
    float pot3Read() {return analogRead(PIN_POT3) * ANALOG_TO_VOLTAGE;}
    float photoRead() {return analogRead(PIN_PHOTO) * ANALOG_TO_VOLTAGE;}

    int32_t rotaryEncoderRead() {return -m_rotaryEncoder.read()/4;}

    void mpuCalculateOffsets(int wait);

    void shift7segWrite(byte left, byte right);
    void shift7segWrite(uint8_t num);
    void shift7segWriteHex(uint8_t num);
    void shiftLedWrite(byte b);
    byte shift7segLeftRead();
    byte shift7segRightRead();
    byte shiftLedRead();
    void shiftClear();
    void shiftTestSequence(int wait);

    void takeTemperatureReading();
    float getTempC() {return m_temperatureC;}
    float getTempF() {return m_temperatureF;}

    void mpuUpdate();
    float mpuGetTemp(){ return m_mpu.getTemp(); };
    float mpuGetAccX(){ return m_mpu.getAccX(); };
    float mpuGetAccY(){ return m_mpu.getAccY(); };
    float mpuGetAccZ(){ return m_mpu.getAccZ(); };
    float mpuGetGyroX(){ return m_mpu.getGyroX(); };
    float mpuGetGyroY(){ return m_mpu.getGyroY(); };
    float mpuGetGyroZ(){ return m_mpu.getGyroZ(); };
    float mpuGetAccAngleX(){ return m_mpu.getAccAngleX(); };
    float mpuGetAccAngleY(){ return m_mpu.getAccAngleY(); };//yup, no Z
    float mpuGetAngleX(){ return m_mpu.getAngleX(); };
    float mpuGetAngleY(){ return m_mpu.getAngleY(); };
    float mpuGetAngleZ(){ return m_mpu.getAngleZ(); };

    void interrupt();

    void clearAllDisplays(DISPLAYS exception = DISPLAY_NONE);

    void buttonCheck();
    bool buttonPressed(BUTTONS b, bool clear = true);
    void buttonDownEventSet(void (*butten_event)(void));

    //TODO
    //Need functions for all four buttons, rotary encoder twists and buttons
    //Need objects, functions, and test mode for IR
    //Need init way to use a reduced number of pins so some can be used for alternate functions
    //loot at other types (float?!?) for oledPrint(ln)
    //RGB internal array... actually use the array
    //Make print commands for 7 segment display so integers or floats can be displayed
    

    private:
    Encoder                 m_rotaryEncoder;
    OneWire                 m_oneWire;
    MPU6050                 m_mpu;
    Adafruit_SSD1306        m_oled_display;
    Adafruit_NeoPixel       m_rgb;

    byte                    m_shift7segLeft;
    byte                    m_shift7segRight;
    byte                    m_shiftLed;

    volatile byte           m_interrupt;

    float                   m_temperatureC; //could use uint_16's for these and save a few bytes
    float                   m_temperatureF;

    byte                    m_buttonState;
    byte                    m_buttonPressed;

    void (*m_buttonDownEvent)();

};



#endif //#ifndef NANOPROTO_H