//This example is used to verify that the HW is fully functional. Goes through several test modes
//by using the 'up' button to transition between them. Uses the OLED to give a brief description
//of the current test.

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

enum TESTS {
    TEST_RGB_LEDS,
    TEST_7SEG,
    TEST_7SEG_FAST,
    TEST_SHIFT_LEDS,
    TEST_SHIFT_LEDS_FAST,
    TEST_BUTTONS,
    TEST_ROTARY,
    TEST_POTS,
    TEST_LIGHT_SENSE,
    TEST_TEMPERATURE,
    TEST_IR,
    TEST_MPU,
    TEST_COUNT_OF_TESTS
};
volatile TESTS g_currentTest = TEST_RGB_LEDS;
TESTS g_lastTest = TEST_COUNT_OF_TESTS;
int g_counter; //incremented every loop cycle. Used by some tests to show change
long g_timer = 0; //timer used for gyro calculations

//declare these functions before they are used so the compiler is happy
//they are defined at the end of the file (after the loop)
void isrIncrementTest();
void setUpTest(const __FlashStringHelper *title, const __FlashStringHelper *desciption = NULL, bool doDisplay = true, DISPLAYS extra = DISPLAY_NONE);

void setup(){
    g_nps.begin();
    g_counter = 0;
    g_nps.clearAllDisplays();
    g_nps.oledPrint(F("Booting and calculating MPU6050 offsets, please leave level"));
    g_nps.oledDisplay();
    g_nps.mpuCalculateOffsets(1000);
    g_nps.clearAllDisplays();
    attachInterrupt( digitalPinToInterrupt(PIN_UP_BUTTON), isrIncrementTest, FALLING ); //make the UP button run the function that advances the test
}

void loop(){
    float lightLevel = 0.0f;
    byte whichBit = 0;
    bool skipOled = (g_currentTest == TEST_SHIFT_LEDS_FAST || g_currentTest == TEST_7SEG_FAST || g_currentTest == TEST_ROTARY );
    bool isFirstTime = g_lastTest != g_currentTest;

    g_lastTest = g_currentTest;

    g_counter++;
    
    //print "'Up' for next(#/#)"
    if(isFirstTime || !skipOled){
        g_nps.oledClear();
        g_nps.oledPrint(F("'Up' for next ("));
        g_nps.oledPrint(g_currentTest + 1);
        g_nps.oledPrint(F("/"));
        g_nps.oledPrint(TEST_COUNT_OF_TESTS);
        g_nps.oledPrintln(F(")"));
    }

    switch(g_currentTest){
        case TEST_RGB_LEDS:
            setUpTest( F("RGB LEDs"), F("Rainbow colors cycling"), true, DISPLAY_RGB_LEDS );
            g_nps.rgbRainbow(1);
            break;

        case TEST_7SEG:
            setUpTest( F("7 Segment LEDs"), F("Counting up in hex."), true, DISPLAY_SHIFT_7SEG );
            g_nps.shift7segPrintHex(g_counter);
            break;

        case TEST_7SEG_FAST:
            if(isFirstTime) {
                setUpTest(F("7 Segment LEDs"), F("Counting up in hex without redrawing the OLED."), true, DISPLAY_SHIFT_7SEG );
            }
            g_nps.shift7segPrintHex(g_counter);
            break;

        case TEST_SHIFT_LEDS:
            setUpTest( F("Shift LEDs"), F("Filling up via binary counting."), true, DISPLAY_SHIFT_LEDS );
            g_nps.shiftLedWrite(g_counter);
            break;

        case TEST_SHIFT_LEDS_FAST:
            if(isFirstTime) {
                setUpTest( F("Shift LEDs"), F("Filling up via binary counting without redrawing the OLED."), true, DISPLAY_SHIFT_LEDS );
            }
            g_nps.shiftLedWrite(g_counter);
            break;

        case TEST_BUTTONS:
            setUpTest(F("Button test"),NULL,false);
            printButtonState(F("LEFT :"),  g_nps.buttonLeftPressed());
            printButtonState(F("RIGHT :"), g_nps.buttonRightPressed());
            printButtonState(F("DOWN :"),  g_nps.buttonDownPressed());
            g_nps.oledDisplay();
            break;

        case TEST_ROTARY:
            if(isFirstTime) {
                setUpTest( F("Rotary Encoder"), F("Encoder state on 7 segment. Press to zero."), true, DISPLAY_SHIFT_7SEG );
            }
            if(g_nps.buttonRotaryPressed())
                g_nps.rotaryWrite(0);
            g_nps.shift7segPrintHex(g_nps.rotaryRead());
            //TODO Not working well... why?!? Too much BUS I/O the Encoder is missing twists? Much better skipping OLED drawing
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
            lightLevel =  g_nps.lightMeterRead();
            whichBit = lightLevel * 8.0f / 5.0f;
            g_nps.shiftLedWrite(1 << whichBit);
            break;

        case TEST_TEMPERATURE:
            setUpTest(F("Temperature"), NULL, false);
            g_nps.takeTemperatureReading();
            g_nps.oledPrintln(g_nps.getTempF());
            g_nps.oledDisplay();
            break;

        case TEST_IR:
            setUpTest(F("IR Tx/Rx"));
            break;

        case TEST_MPU:
            setUpTest(F("Gyro/Accel."), NULL, false);
            g_nps.mpuUpdate();

            if (millis() - g_timer > 1000) { // print data every second
                g_nps.oledPrint(F("TEMP (C): ")); g_nps.oledPrintln(g_nps.mpuGetTemp(),0);
                g_nps.oledPrint(F("ANGLE X: ")); g_nps.oledPrintln(g_nps.mpuGetAngleX(),0);
                g_nps.oledPrint(F("ANGLE Y: ")); g_nps.oledPrintln(g_nps.mpuGetAngleY(),0);
                g_nps.oledPrint(F("ANGLE Z: ")); g_nps.oledPrintln(g_nps.mpuGetAngleZ(),0);
                g_nps.oledDisplay();
                g_timer = millis();
            }
            break;
    } //end switch
}//end loop()

// Interrupt Service Routine (ISR) to walk through the tests
void isrIncrementTest() {
  g_currentTest = incrementValueWithMaxRollover(g_currentTest, TEST_COUNT_OF_TESTS);
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

void printButtonState(const __FlashStringHelper *name, bool pressed) {
    g_nps.oledPrint(name);
    g_nps.oledPrintln(pressed?F("Pressed"):F("Released"));
}