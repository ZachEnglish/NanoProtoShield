# NanoProtoSheld
## This is just the beginning of a library for interacting with the NanoProtoSheld Rev. 3.0. Its purpose is to be an educational tool for young students of computer science and electronics.

# API (Application Programing Interface)
## class NanoProtoShield
### NanoProtoShield(FEATURES features = FEATURE_ALL)
Class constructor. Initializes all the member classes identified by the features list passed in
except for the temperature one because we can save the RAM from having that one always allocated by
just putting it on the stack when we want to take a reading.

### void begin(INDEX_PINS pinout[] = NULL)
begin() should be called in the setup() part of any script that uses a global NanoProtoShield object.
Sets up all the I/O pins and begins the member classes and prepares them for use.

### void interrupt()
Calling interrupt let's the NanoProtoShield know that any function which may take a while (e.g. RGB sequences) should
check to see if they need to do an early abort. This allows for a faster UI response when using buttons for navigation

### void clearAllDisplays(DISPLAYS exception = DISPLAY_NONE)
Clears all the displays on the NanoProtoShield. Can set an exception
so a display does not flash when it is in use yet it is desired to
ensure the rest of the displays are cleared out.

### byte getPin(INDEX_PINS pin)

## OLED Display Functions
### void oledClear()
Should be called each refresh of the OLED display so new things will be drawn without the old still
hanging around. Wipes the display buffer clean and sets the cursor (for text) at the top of the
display. Still requires oledDisplay() to be called to actually see the change on the physical OLED
as this just updates the buffer.

### void oledDisplay(int clear_after = 0)

### void oledInvert(bool i)

### void oledSetTextSize(uint8_t size)

### void oledSetTextSize(uint8_t xSize, uint8_t ySize)

### size_t oledPrint(<many types supported>)

### size_t oledPrintln(<many types supported>)

### void oledDrawPixel(int16_t x, int16_t y, uint16_t color = OLED_COLOR_WHITE)
color is OLED_COLOR_BLACK, OLED_COLOR_WHITE, or OLED_COLOR_INVERT

### void oledDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawRect(int16_t x0, int16_t y0, int16_t width, int16_t height, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawFilledCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color = OLED_COLOR_WHITE)

### void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color = OLED_COLOR_WHITE)

### void drawFilledTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawRoundRect(int16_t x0, int16_t y0, int16_t width, int16_t height, int16_t radius, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawFilledRoundRect(int16_t x0, int16_t y0, int16_t width, int16_t height, int16_t radius, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t width, int16_t height, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t width, int16_t height, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t width, int16_t height, uint16_t color = OLED_COLOR_WHITE)

### void oledDrawBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t width, int16_t height, uint16_t color = OLED_COLOR_WHITE)

## RGB Pixel Strip Functions
### void rgbColorWipe(uint8_t r, uint8_t g, uint8_t b, int wait)
Fill strip pixels one after another with a color. Strip is NOT cleared
first; anything there will be covered pixel by pixel. Pause for 'wait'
amount of time between each pixel. Can be interrupted by a
NanoProtoShield::interrupt() call or a button event if m_rgbInterrupt
is set to true (via NanoProtoShield::rgbSetButtonInterrupt)

### void rgbRainbow(int wait)
Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
Can be interrupted by a NanoProtoShield::interrupt() call or a button
event if m_rgbInterrupt is set to true (via NanoProtoShield::rgbSetButtonInterrupt)

### void rgbClear()
Clears the RGB LED strip and calls show so that the changes are seen

### void rgbSetPixelColor(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b)

### void rgbSetPixelColor(uint8_t pixel, uint32_t color)

### void rgbSetPixelsColor(uint8_t r, uint8_t g, uint8_t b)
Set the color of all the pixels

### void rgbSetPixelsColor(uint32_t color)

### void rgbSetBrightness(uint8_t brightness)

### void rgbShow()

### uint32_t rgbGetColorFromHsv(uint16_t hue, uint8_t sat=255, uint8_t val=255)

### void rgbSetButtonInterrupt(bool interrupt)
Set to true if you want the RGB color wipe and rainbow functions to check for button events
Not doing this makes non-interrupt driven (anything other than the UP button) button presses
very easy to miss.

## Analog Input Functions
These functions return a float value that has been converted to the range 0-5.0 to represent the voltage on the pin.

### float pot1Read()

### float pot2Read()

### float pot3Read()

### float lightMeterRead()

## Buttons and Rotary Encoder Functions
### int rotaryRead()

### void rotaryWrite(int value)

### void buttonCheckForEvent()
buttonCheckForEvent must be called regularly (e.g. in the loop function) for the event system to work. This will call
any appropriate event handlers if a state transition which has a handler is calculated. If there is not a event handler
for any calculated button event, it sets up the internal state for use by the buttonPressed() and buttonReleased()
functions. See ShowButtonPressEvent example sketch to see this in use.

### void buttonSetPressEvent(BUTTON b, void (*buttenEvent)(void))
Set up event handlers for button presses and releases calculated by buttonCheckForEvent() calls.
Cannot be used with buttonPressed() or buttonReleased() as it clears the flag automatically
These events are only triggered when buttonCheckForEvent() calculates a change, so if the update loop is too slow,
they could miss events. See ShowButtonPressEvent example sketch to see this in use.

### void buttonSetReleaseEvent(BUTTON b, void (*buttenEvent)(void))
Set up event handlers for button presses and releases calculated by buttonCheckForEvent() calls.
Cannot be used with buttonPressed() or buttonReleased() as it clears the flag automatically
These events are only triggered when buttonCheckForEvent() calculates a change, so if the update loop is too slow,
they could miss events. See ShowButtonPressEvent example sketch to see this in use.

### bool buttonPressed(BUTTON b, bool clear = true)
buttonPressed will return if a given button has been calculated to have been pressed (via a buttonCheckForEvent() call)
You may optionally clear the flag indicating the press. See ShowButtonPressEvent example sketch to see this in use.

### bool buttonReleased(BUTTON b, bool clear = true)
buttonReleased will return if a given button has been calculated to have been released (via a buttonCheckForEvent() call)
You may optionally clear the flag indicating the release. See ShowButtonPressEvent example sketch to see this in use.

### void buttonClearPressEvent(BUTTON b)
Clear out event handlers that have been set already

### void buttonClearReleaseEvent(BUTTON b)
Clear out event handlers that have been set already

### bool buttonUpPressed()
Simple function to directly read the CURRENT state of a button. Pressed is true, unpressed is false. Have to poll on your own

### bool buttonDownPressed()
Simple function to directly read the CURRENT state of a button. Pressed is true, unpressed is false. Have to poll on your own

### bool buttonRightPressed()
Simple function to directly read the CURRENT state of a button. Pressed is true, unpressed is false. Have to poll on your own

### bool buttonLeftPressed()
Simple function to directly read the CURRENT state of a button. Pressed is true, unpressed is false. Have to poll on your own

### bool buttonRotaryPressed()
Simple function to directly read the CURRENT state of a button. Pressed is true, unpressed is false. Have to poll on your own

## Shift-Driven Displays - 7 Segment & the 8 LEDs
### void shift7segWrite(byte left, byte right)
Write left and right out to the 7 segment display digits
These are raw bytes, so they follow the formatting described where
g_map_7seg is defined in NanoProtoShield.cpp

### void shift7segPrint(uint8_t num)
Prints a number to the 7 segment display. Can handle 0-255 by
using the decimal places to indicate 100 or 200

### void shift7segPrintHex(uint8_t num)
Prints a two digit hex number to the 7 segment display

### byte shift7segLeftRead()

### byte shift7segRightRead()

### void shiftLedWrite(byte b)
Shift out b to the single color LEDs. Stores what was shifted
so when the 7 segmenet display is also used it does not get
lost or erased (since both of those are in series in HW).

### byte shiftLedRead()
### void shiftClear()
Clear out everything shift register related, the 7 segment display
and the 8 single color LEDs

### void shiftTestSequence(int wait)
Go through a test pattern on the shift register displays

## Temperature Sensor Functions
### void takeTemperatureReading()
Takes a temperature reading and stores it for retrieval later.
Puts the DallasTemperature object on the stack to save RAM
Not a super fast function because of that initialization that
takes place, so don't call it in any tight loop

### float getTempC()
Must call takeTemperatureReading() before this will have meaningful data.

### float getTempF()
Must call takeTemperatureReading() before this will have meaningful data.

## Gyroscope & Accelerometer Functions
### void mpuCalculateOffsets(int wait)
### void mpuUpdate()
### float mpuGetTemp()
### float mpuGetAccX()
### float mpuGetAccY()
### float mpuGetAccZ()
### float mpuGetGyroX()
### float mpuGetGyroY()
### float mpuGetGyroZ()
### float mpuGetAccAngleX()
### float mpuGetAccAngleY()
### float mpuGetAngleX()
### float mpuGetAngleY()
### float mpuGetAngleZ()