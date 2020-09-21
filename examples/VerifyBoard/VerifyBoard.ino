#include "NanoProtoShield.h"

NanoProtoShield g_nps;

enum TESTS {
    TEST_RGB_LEDS,
    TEST_7SEG,
    TEST_SHIFT_LEDS,
    TEST_BUTTONS,
    TEST_ROTARY,
    TEST_POTS,
    TEST_LIGHT_SENSE,
    TEST_TEMPERATURE,
    TEST_IR,
    TEST_MPU,
    TEST_COUNT_OF_TESTS
};
volatile TESTS g_whichTest = TEST_RGB_LEDS;
int g_counter;

void isrIncrementTest();
void setUpTest(const __FlashStringHelper *title, const __FlashStringHelper *desciption = NULL, bool doDisplay = true, DISPLAYS extra = DISPLAY_NONE);

void setup(){
    g_nps.begin();
    g_counter = 0;
    attachInterrupt( digitalPinToInterrupt(PIN_UP_BUTTON), isrIncrementTest, FALLING );
}

void loop(){
    float lightLevel = 0.0f;
    byte whichBit = 0;

    g_counter++;
    //print "'Up' for next(#/#)"
    g_nps.oledClear();
    g_nps.oledPrint(F("'Up' for next ("));
    g_nps.oledPrint(g_whichTest + 1);
    g_nps.oledPrint(F("/"));
    g_nps.oledPrint(TEST_COUNT_OF_TESTS);
    g_nps.oledPrintln(F(")"));

    switch(g_whichTest){
        case TEST_RGB_LEDS:
            setUpTest( F("RGB LEDs"), F("Rainbow colors cycling"), true, DISPLAY_RGB_LEDS );
            g_nps.rgbRainbow(1);
            break;

        case TEST_7SEG:
            setUpTest( F("7 Segment LEDs"), F("Counting up in hex"), true, DISPLAY_SHIFT_7SEG );
            g_nps.shift7segWriteHex(g_counter);
            delay(50);
            break;

        case TEST_SHIFT_LEDS:
            setUpTest( F("Shift LEDs"), F("Filling up via binary counting"), true, DISPLAY_SHIFT_LEDS );
            g_nps.shiftLedWrite(g_counter);
            delay(50);
            break;

        case TEST_BUTTONS:
            setUpTest(F("Button test"),NULL,false);
            g_nps.oledPrint(F("LEFT :"));
            g_nps.oledPrintln(g_nps.buttonLeftPressed()?F("Pressed"):F("Released"));
            g_nps.oledPrint(F("RIGHT:"));
            g_nps.oledPrintln(g_nps.buttonRightPressed()?F("Pressed"):F("Released"));
            g_nps.oledPrint(F("DOWN :"));
            g_nps.oledPrintln(g_nps.buttonDownPressed()?F("Pressed"):F("Released"));
            g_nps.oledDisplay();
            break;

        case TEST_ROTARY:
            setUpTest( F("Rotary Encoder"), F("Encoder state on 7 segment"), true, DISPLAY_SHIFT_7SEG );
            g_nps.shift7segWriteHex(g_nps.rotaryRead());
            delay(250); //TODO Not working well... why?!? Too much BUS I/O the Encoder is missing twists?
            break;

        case TEST_POTS:
            setUpTest(F("Analog Pots"), NULL, false);
            g_nps.oledPrint(F("POT1(V): ")); g_nps.oledPrintln(g_nps.pot1Read(),3);
            g_nps.oledPrint(F("POT2(V): ")); g_nps.oledPrintln(g_nps.pot2Read(),3);
            g_nps.oledPrint(F("POT3(V): ")); g_nps.oledPrintln(g_nps.pot3Read(),3);
            g_nps.oledDisplay();
            break;

        case TEST_LIGHT_SENSE:
            setUpTest(F("Light sensor"), F("LEDs serve as light level indicator"), true, DISPLAY_SHIFT_LEDS);
            lightLevel =  g_nps.photoRead();
            whichBit = lightLevel * 8.0f / 5.0f;
            g_nps.shiftLedWrite(1 << whichBit);
            break;

        case TEST_TEMPERATURE:
            setUpTest(F("Temperature"), NULL, false);
            g_nps.oledPrintln(g_nps.getTempF());
            g_nps.oledDisplay();
            break;

        case TEST_IR:
            setUpTest(F("IR Tx/Rx"));
            break;

        case TEST_MPU:
            setUpTest(F("Gyro/Accel."), NULL, false);

            g_nps.oledDisplay();
            break;
    }
}

// Interrupt Service Routine (ISR) to walk through the tests
void isrIncrementTest() {
  g_whichTest = incrementValueWithMaxRollover(g_whichTest, TEST_COUNT_OF_TESTS);
  g_nps.interrupt();
}

void setUpTest(const __FlashStringHelper *title, const __FlashStringHelper *desciption = NULL, bool doDisplay = true, DISPLAYS extra = DISPLAY_NONE) {
    g_nps.clearAllDisplays( DISPLAY_OLED | extra );
    g_nps.oledPrintln(title);
    if(desciption)
        g_nps.oledPrintln(desciption);
    if(doDisplay)
        g_nps.oledDisplay();
}