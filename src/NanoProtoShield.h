#ifndef NANOPROTOSHIELD_H
#define NANOPROTOSHIELD_H

#include "Arduino.h"
#include <SPI.h> //for talking to OLED display
#include <Wire.h> //for talking to OLED display
#include <Adafruit_GFX.h> //for talking to OLED display
#include <Adafruit_SSD1306.h> //for talking to OLED display
#include <Adafruit_NeoPixel.h> //for talking to RGB LEDs
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

enum BUTTON { BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_ROTARY, BUTTON_COUNT };

//Declare class object to abstract and hide many of the complexities of the board
class NanoProtoShield {
    public:

    NanoProtoShield();

    void begin();

    void oledDisplay(int clear_after = 0);
    void oledClear();
    void oledInvert(bool i) {m_oled.invertDisplay(i);}
    size_t oledPrint(const __FlashStringHelper *f)  { m_oled.print(f); }
    size_t oledPrint(const String &s)               { m_oled.print(s); }
    size_t oledPrint(const char c[])                { m_oled.print(c); }
    size_t oledPrint(char c)                        { m_oled.print(c); }
    size_t oledPrint(unsigned char c, int b = DEC)  { m_oled.print(c,b); }
    size_t oledPrint(int i, int b = DEC)            { m_oled.print(i,b); }
    size_t oledPrint(unsigned int i, int b= DEC)    { m_oled.print(i,b); }
    size_t oledPrint(long i, int b = DEC)           { m_oled.print(i,b); }
    size_t oledPrint(unsigned long i, int b = DEC)  { m_oled.print(i,b); }
    size_t oledPrint(double d, int digits = 2)      { m_oled.print(d,digits); }
    size_t oledPrint(const Printable& p)            { m_oled.print(p); }
    size_t oledPrintln(const __FlashStringHelper *f)    { m_oled.println(f); }
    size_t oledPrintln(const String &s)                 { m_oled.println(s); }
    size_t oledPrintln(const char c[])                  { m_oled.println(c); }
    size_t oledPrintln(char c)                          { m_oled.println(c); }
    size_t oledPrintln(unsigned char c, int b = DEC)    { m_oled.println(c,b); }
    size_t oledPrintln(int i, int b = DEC)              { m_oled.println(i,b); }
    size_t oledPrintln(unsigned int i, int b = DEC)     { m_oled.println(i,b); }
    size_t oledPrintln(long i, int b = DEC)             { m_oled.println(i,b); }
    size_t oledPrintln(unsigned long i, int b = DEC)    { m_oled.println(i,b); }
    size_t oledPrintln(double d, int digits = 2)        { m_oled.println(d,digits); }
    size_t oledPrintln(const Printable& p)              { m_oled.println(p); }
    size_t oledPrintln(void)                            { m_oled.println(); }

    void rgbColorWipe(uint8_t r, uint8_t g, uint8_t b, int wait);
    void rgbRainbow(int wait);
    void rgbClear(); //ZDE: Debating if this should be here or if it hides the underlying class too much
    void rgbSetPixelColor(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b)   {m_rgb.setPixelColor( pixel % RGB_LED_COUNT, m_rgb.Color(r,g,b));};
    void rgbSetPixelColor(uint8_t pixel, uint32_t color)                    {m_rgb.setPixelColor( pixel, color );};
    void rgbSetPixelsColor(uint8_t r, uint8_t g, uint8_t b);
    void rgbSetPixelsColor(uint32_t color);
    void rgbSetBrightness(uint8_t brightness)   {m_rgb.setBrightness(brightness);};
    void rgbShow() {m_rgb.show();};
    uint32_t rgbGetColorFromHsv(uint16_t hue, uint8_t sat=255, uint8_t val=255) {return Adafruit_NeoPixel::ColorHSV(hue,sat,val);}
    void rgbSetButtonInterrupt(bool interrupt) {m_rgbInterrupt = interrupt;};

    float pot1Read() {return analogRead(PIN_POT1) * ANALOG_TO_VOLTAGE;}
    float pot2Read() {return analogRead(PIN_POT2) * ANALOG_TO_VOLTAGE;}
    float pot3Read() {return analogRead(PIN_POT3) * ANALOG_TO_VOLTAGE;}
    float photoRead() {return analogRead(PIN_PHOTO) * ANALOG_TO_VOLTAGE;}

    int rotaryRead() {return m_rotary.read()/4;} //Don't know what is wrong, but the Encoder library always updates in increments of 4...
    void rotaryWrite(int value) {m_rotary.write(value*4);}

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
    float getTempF() {return ((m_temperatureC * 1.8f) + 32.0f) ;}

    void mpuCalculateOffsets(int wait);
    void mpuUpdate() {m_mpu.update();}
    float mpuGetTemp(){ return m_mpu.getTemp(); }
    float mpuGetAccX(){ return m_mpu.getAccX(); }
    float mpuGetAccY(){ return m_mpu.getAccY(); }
    float mpuGetAccZ(){ return m_mpu.getAccZ(); }
    float mpuGetGyroX(){ return m_mpu.getGyroX(); }
    float mpuGetGyroY(){ return m_mpu.getGyroY(); }
    float mpuGetGyroZ(){ return m_mpu.getGyroZ(); }
    float mpuGetAccAngleX(){ return m_mpu.getAccAngleX(); }
    float mpuGetAccAngleY(){ return m_mpu.getAccAngleY(); }//yup, no Z
    float mpuGetAngleX(){ return m_mpu.getAngleX(); }
    float mpuGetAngleY(){ return m_mpu.getAngleY(); }
    float mpuGetAngleZ(){ return m_mpu.getAngleZ(); }

    void interrupt();

    void clearAllDisplays(DISPLAYS exception = DISPLAY_NONE);

    //buttonCheckForEvent must be called regularly (e.g. in the loop function) for the event system to work. This will call
    //any appropriate event handlers if a state transition which has a handler is calculated. If there is not a event handler
    //for any calculated button event, it sets up the internal state for use by the buttonPressed() and buttonReleased()
    //functions
    void buttonCheckForEvent();

    //buttonPressed will return if a given button has been calculated to have been pressed (via a buttonCheckForEvent() call)
    //You may optionally clear the flag indicating the press.
    bool buttonPressed(BUTTON b, bool clear = true);

    //buttonReleased will return if a given button has been calculated to have been released (via a buttonCheckForEvent() call)
    //You may optionally clear the flag indicating the release.
    bool buttonReleased(BUTTON b, bool clear = true);

    //Set up event handlers for button presses and releases calculated by buttonCheckForEvent() calls.
    //Cannot be used with buttonPressed() or buttonReleased() as it clears the flag automatically
    //These events are only triggered when buttonCheckForEvent() calculates a change, so if the update loop is too slow,
    //they could miss events
    void buttonSetPressEvent(BUTTON b, void (*buttenEvent)(void));
    void buttonSetReleaseEvent(BUTTON b, void (*buttenEvent)(void));

    //Clear out event handlers that have been set already
    void buttonClearPressEvent(BUTTON b) {(b<BUTTON_COUNT)?m_buttonPressEvents[b] = NULL : NULL;};
    void buttonClearReleaseEvent(BUTTON b){(b<BUTTON_COUNT)?m_buttonReleaseEvents[b] = NULL : NULL;};

    //Simple functions to directly read the CURRENT state of a button. Pressed is true, unpressed is false. Have to poll on your own
    bool buttonUpPressed(){ return digitalRead(PIN_UP_BUTTON); }
    bool buttonDownPressed(){ return digitalRead(PIN_DOWN_BUTTON); }
    bool buttonRightPressed(){ return digitalRead(PIN_RIGHT_BUTTON); }
    bool buttonLeftPressed(){ return digitalRead(PIN_LEFT_BUTTON); }
    bool buttonRotaryPressed(){ return !digitalRead(PIN_ROT_ENC_BUTTON); }//Hardware has this input inverted (active high)

    //TODO
    //Need objects, functions, and test mode for IR
    //Need init way to use a reduced number of pins so some can be used for alternate functions
    //look at other types (float?!?) for oledPrint(ln)
    //make way for button check to interrupt rgb functions
    //RGB class??

    private:
    Adafruit_SSD1306        m_oled;
    Encoder                 m_rotary;
    OneWire                 m_oneWire;
    MPU6050                 m_mpu;
    Adafruit_NeoPixel       m_rgb;

    byte                    m_shift7segLeft;
    byte                    m_shift7segRight;
    byte                    m_shiftLed;

    volatile byte           m_interrupt;
    bool                    m_rgbInterrupt;

    float                   m_temperatureC;

    byte                    m_buttonState;
    byte                    m_buttonPressed;
    byte                    m_buttonReleased;

    void (*m_buttonPressEvents[BUTTON_COUNT])(void);
    void (*m_buttonReleaseEvents[BUTTON_COUNT])(void);
};

int incrementValueWithMaxRollover(int value, int max);
int decrementValueWithMaxRollover(int value, int max);

#endif //#ifndef NANOPROTO_H