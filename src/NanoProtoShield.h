#ifndef NANOPROTOSHIELD_H
#define NANOPROTOSHIELD_H

#include "Arduino.h"
#include <SPI.h> //for talking to OLED display
#include <Wire.h> //for talking to OLED display
#include <Adafruit_GFX.h> //for talking to OLED display
#include <Adafruit_SSD1306.h> //for talking to OLED display
#include <Adafruit_NeoPixel.h> //for talking to RGB LEDs
#include <Encoder.h> //for talking to the rotary encoded. Try ClickEncoder?
#include "OneWireWithPullup.h" //for talking to one wire devices like the temperature sensor.
#define REQUIRESALARMS false //saves bytes in the DallasTemperature object by not enabling functionality we don't need/use
#include "DallasTemperatureWithPullup.h" //for talking to the temperature sensor
#include <MPU6050_light.h> //for talking to the gyro/accelerometer


// ------------- BOARD SPECIFIC DEFINES -------------
#ifdef USE_VERSION_2_0_PINOUT

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
#define PIN_TEMPERATURE     9
#define PIN_RGB_LED         12
#define PIN_POT1            A0
#define PIN_POT2            A1
#define PIN_POT3            A2
#define PIN_PHOTO           A3
#define PIN_IR_RX           A7
#define PIN_IR_TX           A6

#else //no "if" here, default to the latest board design

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

#endif //finish off the #ifdef/else for pinout version.


// ------------- RGB LED DEFINES -------------
// How many NeoPixels are attached to the Arduino?
#define RGB_LED_COUNT 8


// ------------- OLED DISPLAY DEFINES -------------
#define OLED_SCREEN_WIDTH 128 // OLED display width, in pixels
#define OLED_SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_ADDRESS 0x3C


#define ANALOG_TO_VOLTAGE (5.0 / 1023.0) //Multiply by to convert an analog reading to voltage level
#define VOLTAGE_TO_RGB (255.0 / 5.0) //Multiply by to convert a voltage to an RGB value (0-255)


enum DISPLAYS { 
    DISPLAY_NONE        = 0,
    DISPLAY_RGB_LEDS    = bit(0),
    DISPLAY_SHIFT_LEDS  = bit(1),
    DISPLAY_SHIFT_7SEG  = bit(2),
    DISPLAY_OLED        = bit(3)
    };

enum BUTTON { BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_ROTARY, BUTTON_COUNT };

enum FEATURES {
    FEATURE_NONE            = 0, //please tell me why you are using this
    FEATURE_OLED            = bit(0),
    FEATURE_ROTARY_TWIST    = bit(1),
    FEATURE_ROT_ENC_BUTTON  = bit(2),
    FEATURE_MPU             = bit(3),
    FEATURE_RGB             = bit(4),
    FEATURE_SHIFTS          = bit(5),
    FEATURE_BUTTON_UP       = bit(6),
    FEATURE_BUTTON_DOWN     = bit(7),
    FEATURE_BUTTON_LEFT     = bit(8),
    FEATURE_BUTTON_RIGHT    = bit(9),
    FEATURE_IR_TX           = bit(10),
    FEATURE_IR_RX           = bit(11)
    };
#define FEATURE_ALL (FEATURE_OLED           | \
                     FEATURE_ROTARY_TWIST   | \
                     FEATURE_ROT_ENC_BUTTON | \
                     FEATURE_MPU            | \
                     FEATURE_RGB            | \
                     FEATURE_SHIFTS         | \
                     FEATURE_BUTTON_UP      | \
                     FEATURE_BUTTON_DOWN    | \
                     FEATURE_BUTTON_LEFT    | \
                     FEATURE_BUTTON_RIGHT   | \
                     FEATURE_IR_TX          | \
                     FEATURE_IR_RX)


//Declare class object to abstract and hide many of the complexities of the board
class NanoProtoShield {
    public:
    
    //Class constructor. Initializes all the member classes identified by the features list passed in
    //except for the temperature one because we can save the RAM from having that one always allocated by
    //just putting it on the stack when we want to take a reading.
    NanoProtoShield(FEATURES features = FEATURE_ALL);
    ~NanoProtoShield();

    //begin() should be called in the setup() part of any script that uses a global NanoProtoShield object.
    //Sets up all the I/O pins and begins the member classes and prepares them for use.
    void begin();

    void oledDisplay(int clear_after = 0);
    void oledClear();
    void oledInvert(bool i) {if(m_oled) m_oled->invertDisplay(i);}
    size_t oledPrint(const __FlashStringHelper *f)  { return m_oled? m_oled->print(f) : 0; }
    size_t oledPrint(const String &s)               { return m_oled? m_oled->print(s) : 0; }
    size_t oledPrint(const char c[])                { return m_oled? m_oled->print(c) : 0; }
    size_t oledPrint(char c)                        { return m_oled? m_oled->print(c) : 0; }
    size_t oledPrint(unsigned char c, int b = DEC)  { return m_oled? m_oled->print(c,b) : 0; }
    size_t oledPrint(int i, int b = DEC)            { return m_oled? m_oled->print(i,b) : 0; }
    size_t oledPrint(unsigned int i, int b= DEC)    { return m_oled? m_oled->print(i,b) : 0; }
    size_t oledPrint(long i, int b = DEC)           { return m_oled? m_oled->print(i,b) : 0; }
    size_t oledPrint(unsigned long i, int b = DEC)  { return m_oled? m_oled->print(i,b) : 0; }
    size_t oledPrint(double d, int digits = 2)      { return m_oled? m_oled->print(d,digits) : 0; }
    size_t oledPrint(const Printable& p)            { return m_oled? m_oled->print(p) : 0; }
    size_t oledPrintln(const __FlashStringHelper *f)    { return m_oled? m_oled->println(f) : 0; }
    size_t oledPrintln(const String &s)                 { return m_oled? m_oled->println(s) : 0; }
    size_t oledPrintln(const char c[])                  { return m_oled? m_oled->println(c) : 0; }
    size_t oledPrintln(char c)                          { return m_oled? m_oled->println(c) : 0; }
    size_t oledPrintln(unsigned char c, int b = DEC)    { return m_oled? m_oled->println(c,b) : 0; }
    size_t oledPrintln(int i, int b = DEC)              { return m_oled? m_oled->println(i,b) : 0; }
    size_t oledPrintln(unsigned int i, int b = DEC)     { return m_oled? m_oled->println(i,b) : 0; }
    size_t oledPrintln(long i, int b = DEC)             { return m_oled? m_oled->println(i,b) : 0; }
    size_t oledPrintln(unsigned long i, int b = DEC)    { return m_oled? m_oled->println(i,b) : 0; }
    size_t oledPrintln(double d, int digits = 2)        { return m_oled? m_oled->println(d,digits) : 0; }
    size_t oledPrintln(const Printable& p)              { return m_oled? m_oled->println(p) : 0; }
    size_t oledPrintln(void)                            { return m_oled? m_oled->println() : 0; }

    // Fill strip pixels one after another with a color. Strip is NOT cleared
    // first; anything there will be covered pixel by pixel. Pause for 'wait'
    // amount of time between each pixel. Can be interrupted by a
    // NanoProtoShield::interrupt() call or a button event if m_rgbInterrupt
    // is set to true (via NanoProtoShield::rgbSetButtonInterrupt)
    void rgbColorWipe(uint8_t r, uint8_t g, uint8_t b, int wait);
    // Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
    // Can be interrupted by a NanoProtoShield::interrupt() call or a button
    // event if m_rgbInterrupt is set to true (via NanoProtoShield::rgbSetButtonInterrupt)
    void rgbRainbow(int wait);
    //Clears the RGB LED strip and calls show so that the changes are seen
    void rgbClear();
    void rgbSetPixelColor(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b)   {if(m_rgb) m_rgb->setPixelColor( pixel % RGB_LED_COUNT, m_rgb->Color(r,g,b));};
    void rgbSetPixelColor(uint8_t pixel, uint32_t color)                    {if(m_rgb) m_rgb->setPixelColor( pixel, color );};
    //Set the color of all the pixels
    void rgbSetPixelsColor(uint8_t r, uint8_t g, uint8_t b);
    void rgbSetPixelsColor(uint32_t color);
    void rgbSetBrightness(uint8_t brightness)   {if(m_rgb) m_rgb->setBrightness(brightness);};
    void rgbShow() {m_rgb->show();};
    uint32_t rgbGetColorFromHsv(uint16_t hue, uint8_t sat=255, uint8_t val=255) {return Adafruit_NeoPixel::ColorHSV(hue,sat,val);}
    //Set to true if you want the RGB color wipe and rainbow functions to check for button events
    //Not doing this makes non-interrupt driven (anything other than the UP button) button presses
    //very easy to miss.
    void rgbSetButtonInterrupt(bool interrupt) {m_rgbInterrupt = interrupt;};

    float pot1Read() {return analogRead(PIN_POT1) * ANALOG_TO_VOLTAGE;}
    float pot2Read() {return analogRead(PIN_POT2) * ANALOG_TO_VOLTAGE;}
    float pot3Read() {return analogRead(PIN_POT3) * ANALOG_TO_VOLTAGE;}
    float lightMeterRead() {return analogRead(PIN_PHOTO) * ANALOG_TO_VOLTAGE;}

    int rotaryRead() {return (m_rotary)? m_rotary->read()/4 : 0;} //Don't know what is wrong, but the Encoder library always updates in increments of 4...
    void rotaryWrite(int value) {if(m_rotary) m_rotary->write(value*4);}

    //Wrtine left and right out to the 7 segment display digits
    //These are raw bytes, so they follow the formatting described where
    //g_map_7seg is defined in NanoProtoShield.cpp
    void shift7segWrite(byte left, byte right);
    //Prints a number to the 7 segment display. Can handle 0-255 by
    //using the decimal places to indicate 100 or 200
    void shift7segPrint(uint8_t num);
    //Prints a two digit hex number to the 7 segment display
    void shift7segPrintHex(uint8_t num);
    byte shift7segLeftRead() {return ~m_shift7segLeft;}
    byte shift7segRightRead() {return ~m_shift7segRight;}
    //Shift out b to the single color LEDs. Stores what was shifted
    //so when the 7 segmenet display is also used it does not get
    //lost or erased (since both of those are in series in HW).
    void shiftLedWrite(byte b);
    byte shiftLedRead() {return m_shiftLed;}
    //Clear out everything shift register related, the 7 segment display
    //and the 8 single color LEDs
    void shiftClear();
    //Go through a test pattern on the shift register displays
    void shiftTestSequence(int wait);

    //Takes a temperature reading and stores it for retrieval later.
    //Puts the DallasTemperature object on the stack to save RAM
    //Not a super fast function because of that initialization that
    //takes place, so don't call it in any tight loop
    void takeTemperatureReading();
    float getTempC() {return m_temperatureC;}
    float getTempF() {return ((m_temperatureC * 1.8f) + 32.0f) ;}

    void mpuCalculateOffsets(int wait);
    void mpuUpdate() {if(m_mpu) m_mpu->update();}
    float mpuGetTemp(){ return (m_mpu)? m_mpu->getTemp() : 0; }
    float mpuGetAccX(){ return (m_mpu)? m_mpu->getAccX() : 0; }
    float mpuGetAccY(){ return (m_mpu)? m_mpu->getAccY() : 0; }
    float mpuGetAccZ(){ return (m_mpu)? m_mpu->getAccZ() : 0; }
    float mpuGetGyroX(){ return (m_mpu)? m_mpu->getGyroX() : 0; }
    float mpuGetGyroY(){ return (m_mpu)? m_mpu->getGyroY() : 0; }
    float mpuGetGyroZ(){ return (m_mpu)? m_mpu->getGyroZ() : 0; }
    float mpuGetAccAngleX(){ return (m_mpu)? m_mpu->getAccAngleX() : 0; }
    float mpuGetAccAngleY(){ return (m_mpu)? m_mpu->getAccAngleY() : 0; }//yup, no Z
    float mpuGetAngleX(){ return (m_mpu)? m_mpu->getAngleX() : 0; }
    float mpuGetAngleY(){ return (m_mpu)? m_mpu->getAngleY() : 0; }
    float mpuGetAngleZ(){ return (m_mpu)? m_mpu->getAngleZ() : 0; }

    //Calling interrupt let's the NanoProtoShield know that any function which may take a while (e.g. RGB sequences) should
    //check to see if they need to do an early abort. This allows for a faster UI response when using buttons for navigation
    void interrupt() {m_interrupt++;}

    //Clears all the displays on the NanoProtoShield. Can set an exception
    //so a display does not flash when it is in use yet it is desired to
    //ensure the rest of the displays are cleared out.
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
    void buttonClearPressEvent(BUTTON b) {if(b < BUTTON_COUNT) m_buttonPressEvents[b] = NULL;};
    void buttonClearReleaseEvent(BUTTON b){if(b < BUTTON_COUNT) m_buttonReleaseEvents[b] = NULL;};

    //Simple functions to directly read the CURRENT state of a button. Pressed is true, unpressed is false. Have to poll on your own
    bool buttonUpPressed(){ return (m_features & FEATURE_BUTTON_UP)? digitalRead(PIN_UP_BUTTON) : 0; }
    bool buttonDownPressed(){ return (m_features & FEATURE_BUTTON_DOWN)? digitalRead(PIN_DOWN_BUTTON) : 0; }
    bool buttonRightPressed(){ return (m_features & FEATURE_BUTTON_RIGHT)? digitalRead(PIN_RIGHT_BUTTON) : 0; }
    bool buttonLeftPressed(){ return (m_features & FEATURE_BUTTON_LEFT)? digitalRead(PIN_LEFT_BUTTON) : 0; }
    bool buttonRotaryPressed(){ return (m_features & FEATURE_ROT_ENC_BUTTON)? !digitalRead(PIN_ROT_ENC_BUTTON) : 0; }//Hardware has this input inverted (active high)

    //TODO
    //Need objects, functions, and test mode for IR
    //RGB class??

    private:
    FEATURES                m_features;

    Adafruit_SSD1306        *m_oled;
    Encoder                 *m_rotary;
    OneWireWithPullup       *m_oneWire;
    DallasTemperatureWithPullup
                            *m_tempSensor;
    MPU6050                 *m_mpu;
    Adafruit_NeoPixel       *m_rgb;

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