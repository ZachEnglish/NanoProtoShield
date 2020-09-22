#include "NanoProtoShield.h"

#define QUICK_BOOT true //if true, this skips all the testing/demo at the beginning and goes right to the first actual mode

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
              MODE_SHIFT_7SEG_HEX,
              MODE_SHIFT_7SEG_DEC,
              MODE_ROT_ENC,
              MODE_OLED_HELLO_WORLD,
              MODE_OLED_LOVE_MY_WIFE,
              MODE_TEMPERATURE_PRINT,
              MODE_6050_PRINT,
              MODE_ANALOG_PRINT,
              MODE_COUNT
           };
volatile MODES g_mode = MODE_RGB_COLOR_CHASE;


//Used to keep track of what bit is lit/unlit on the 7seg displays while manipulating them
uint8_t g_current7segBit;

uint8_t g_currentRgbBit;

uint8_t g_7segAlpha;

void setup() {
  Serial.begin(112500);
  //Start out at the beginning
  g_current7segBit = 0;
  g_currentRgbBit = 0;
  g_7segAlpha = 0;
  int mpuOffsetTime = 10;

  //Initialize the NanoProtoShield object
  g_nps.begin();

  if (!QUICK_BOOT)
  {
    //There will be the adafruit logo in memory on start up, this shows it briefly
    g_nps.oledDisplay(250);

    g_nps.shiftTestSequence(250);

    //Set up the MPU6050 Gyro/Accel - Show DO NOT MOVE message on OLED dispaly
    g_nps.clearAllDisplays();
    g_nps.oledPrint(F("Calculating gyro offset, do not move MPU6050"));
    g_nps.oledDisplay();
    mpuOffsetTime = 1000;
  }

  g_nps.mpuCalculateOffsets(mpuOffsetTime);
  g_nps.clearAllDisplays();

  //attach the ISR to the UP button.
  attachInterrupt( digitalPinToInterrupt(PIN_UP_BUTTON), isrIncrementMode, FALLING );
  g_nps.buttonSetPressEvent(BUTTON_DOWN, notIsrDecrementMode);
  g_nps.rgbSetButtonInterrupt(true);
} //end setup()


void loop() {
  byte b;
  byte bn;
  MODES startingMode = g_mode;

  g_nps.buttonCheckForEvent();

  // if(g_nps.buttonPressed(BUTTON_DOWN))
  //   g_mode = decrementValueWithMaxRollover(g_mode, MODE_COUNT);

  switch (g_mode)
  {
    case MODE_RGB_COLOR_CHASE:
      g_nps.clearAllDisplays(DISPLAY_RGB_LEDS);

      if (startingMode != g_mode)
        break;
      g_nps.rgbColorWipe(255,   0,   0, 50); // Red
      g_nps.buttonCheckForEvent();
      if (startingMode != g_mode)
        break;
      g_nps.rgbColorWipe(  0, 255,   0, 50); // Green
      g_nps.buttonCheckForEvent();
      if (startingMode != g_mode)
        break;
      g_nps.rgbColorWipe(  0,   0, 255, 50); // Blue
      g_nps.buttonCheckForEvent();
      break;

    case MODE_RGB_RAINBOW:
      g_nps.clearAllDisplays(DISPLAY_RGB_LEDS);

      g_nps.rgbRainbow(1);
      break;

    case MODE_RGB_PIXEL_SET:
      g_nps.clearAllDisplays(DISPLAY_RGB_LEDS | DISPLAY_SHIFT_LEDS);

      g_nps.buttonCheckForEvent();
      if (g_nps.buttonPressed(BUTTON_RIGHT))
        g_currentRgbBit = incrementValueWithMaxRollover(g_currentRgbBit, RGB_LED_COUNT);
      if (g_nps.buttonPressed(BUTTON_LEFT))
        g_currentRgbBit = decrementValueWithMaxRollover(g_currentRgbBit, RGB_LED_COUNT);

      g_nps.shiftLedWrite(1 << g_currentRgbBit);

      g_nps.rgbSetPixelColor(g_currentRgbBit, g_nps.pot1Read()*VOLTAGE_TO_RGB, g_nps.pot2Read()*VOLTAGE_TO_RGB, g_nps.pot3Read()*VOLTAGE_TO_RGB);
      g_nps.rgbShow();
      break;

    case MODE_SHIFT_LEDS:
      g_nps.clearAllDisplays(DISPLAY_SHIFT_LEDS);

      //Play with the shift registers
      g_nps.shiftLedWrite(0b10101010);
      delay(250);

      g_nps.shiftLedWrite(0b01010101);
      delay(250);
      break;

    case MODE_SHIFT_7SEG:
      g_nps.clearAllDisplays(DISPLAY_SHIFT_7SEG);

      g_current7segBit = (g_current7segBit + 1) % 8;
      bitSet(b, g_current7segBit);
      bn = ~b;

      g_nps.shift7segWrite(b, bn);
      delay(150);
      break;

    case MODE_SHIFT_7SEG_HEX:
      g_nps.clearAllDisplays(DISPLAY_SHIFT_7SEG | DISPLAY_OLED);

      g_nps.buttonCheckForEvent();
      if (g_nps.buttonPressed(BUTTON_RIGHT))
        g_7segAlpha = incrementValueWithMaxRollover(g_7segAlpha, 256);
      if (g_nps.buttonPressed(BUTTON_LEFT))
        g_7segAlpha = decrementValueWithMaxRollover(g_7segAlpha, 256);

      g_nps.shift7segPrintHex(g_7segAlpha);
      g_nps.oledClear();
      g_nps.oledPrint(g_7segAlpha + 0.0f);
      g_nps.oledDisplay();
      break;

    case MODE_SHIFT_7SEG_DEC:
      g_nps.clearAllDisplays(DISPLAY_SHIFT_7SEG | DISPLAY_OLED);

      g_nps.buttonCheckForEvent();
      if (g_nps.buttonPressed(BUTTON_RIGHT))
        g_7segAlpha = incrementValueWithMaxRollover(g_7segAlpha, 100);
      if (g_nps.buttonPressed(BUTTON_LEFT))
        g_7segAlpha = decrementValueWithMaxRollover(g_7segAlpha, 100);

      g_nps.shift7segPrint(g_7segAlpha);
      g_nps.oledClear();
      g_nps.oledPrint(g_7segAlpha + 0.0f);
      g_nps.oledDisplay();
      break;

    case MODE_ROT_ENC:
      g_nps.clearAllDisplays(DISPLAY_SHIFT_7SEG);
      g_current7segBit = g_nps.rotaryRead() % 8;
      bitSet(b, abs(g_current7segBit));
      bn = ~b;

      g_nps.shift7segWrite(b, bn);
      break;

    case MODE_OLED_HELLO_WORLD:
      g_nps.clearAllDisplays(DISPLAY_OLED);

      g_nps.oledClear();
      g_nps.oledPrint(F("Hello world!"));
      g_nps.oledDisplay();
      break;

    case MODE_OLED_LOVE_MY_WIFE:
      g_nps.clearAllDisplays(DISPLAY_OLED);

      g_nps.oledClear();
      g_nps.oledPrintln(F("I enjoy this!"));
      g_nps.oledPrintln(F("I love my wife!"));
      g_nps.oledDisplay();
      break;

    case MODE_TEMPERATURE_PRINT:
      g_nps.clearAllDisplays(DISPLAY_OLED);

      g_nps.takeTemperatureReading();

      g_nps.oledClear();
      g_nps.oledPrintln(F("Temperature is:"));
      g_nps.oledPrint(g_nps.getTempC(),2);
      g_nps.oledPrintln(F("C"));
      g_nps.oledPrint(g_nps.getTempF(),2);
      g_nps.oledPrintln(F("F"));
      g_nps.oledDisplay();
      break;

    case MODE_6050_PRINT:
      g_nps.clearAllDisplays(DISPLAY_OLED);
      g_nps.mpuUpdate();

      if (millis() - g_timer > 1000) { // print data every second
        g_nps.oledClear();
        g_nps.oledPrint(F("TEMP: ")); g_nps.oledPrintln(g_nps.mpuGetTemp(),0);
        g_nps.oledPrint(F("ACC X: ")); g_nps.oledPrint(g_nps.mpuGetAccX(),0);
        g_nps.oledPrint(F(" Y: ")); g_nps.oledPrint(g_nps.mpuGetAccY(),0);
        g_nps.oledPrint(F(" Z: ")); g_nps.oledPrintln(g_nps.mpuGetAccZ(),0);

        g_nps.oledPrint(F("GYRO X: ")); g_nps.oledPrint(g_nps.mpuGetGyroX(),0);
        g_nps.oledPrint(F(" Y: ")); g_nps.oledPrint(g_nps.mpuGetGyroY(),0);
        g_nps.oledPrint(F(" Z: ")); g_nps.oledPrintln(g_nps.mpuGetGyroZ(),0);

        g_nps.oledPrint(F("X:")); g_nps.oledPrint(g_nps.mpuGetAccAngleX(),0);
        g_nps.oledPrint(F(" Y:")); g_nps.oledPrintln(g_nps.mpuGetAccAngleY(),0);

        g_nps.oledPrint(F("ANGLE X: ")); g_nps.oledPrint(g_nps.mpuGetAngleX(),0);
        g_nps.oledPrint(F(" Y: ")); g_nps.oledPrint(g_nps.mpuGetAngleY(),0);
        g_nps.oledPrint(F(" Z: ")); g_nps.oledPrintln(g_nps.mpuGetAngleZ(),0);
        g_nps.oledDisplay();
        g_timer = millis();
      }
      break;

    case MODE_ANALOG_PRINT:
      g_nps.clearAllDisplays(DISPLAY_OLED);

      g_nps.oledClear();
      g_nps.oledPrint(F("POT1(V): ")); g_nps.oledPrintln(g_nps.pot1Read(),2);
      g_nps.oledPrint(F("POT2(V): ")); g_nps.oledPrintln(g_nps.pot2Read(),2);
      g_nps.oledPrint(F("POT3(V): ")); g_nps.oledPrintln(g_nps.pot3Read(),2);
      g_nps.oledPrint(F("PHOTO(V): ")); g_nps.oledPrintln(g_nps.lightMeterRead(),2);
      g_nps.oledDisplay();
      break;
  }//end switch
}//end loop()


// Interrupt Service Routine (ISR) to walk through the modes
void isrIncrementMode() {
  g_mode = incrementValueWithMaxRollover(g_mode, MODE_COUNT);
  g_nps.interrupt();
}

void notIsrDecrementMode() {
  g_mode = decrementValueWithMaxRollover(g_mode, MODE_COUNT);
  g_nps.interrupt();
}
