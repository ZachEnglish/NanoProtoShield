/*  MemoryRGB.ino

    Purpose:
      Memory game.

    What it does:
      Shows colors on the RGB pixels for a short time and then hides then. Goes into the guessing
      mode where the user/player selects the color for each RGB LED one at a time using the rotary
      encoder. Clicking it moves on to the next RGB LED.

      The right button let's the user/player "peek" at the answer.
      The left button goes into a config mode where the number of RGB LEDs in use, the number of
      colors, and the length of preview time are all adjustable. These are stored in non-volatile
      memory so that they persist over resets.
      The down button starts a new game.

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      None
*/

#include <NanoProtoShield.h>
#include <EEPROM.h>

NanoProtoShield g_nps; //Object to interact with and control all things on the NanoProtoShield

//These will be the user configurable settings saved in EEPROM
struct GameSettings_t {
  byte numberOfLEDs;
  byte numberOfColorOptions;
  byte secondsOfPreview;
  byte secondsOfGame;
};
GameSettings_t g_gameSettings;

//Structure to keep RGB values together
struct PixelColor_t {
  byte r;
  byte g;
  byte b;
};

//Arrays for the goal and current user guess. When they are the same, the user won!
//Using bytes that represent the colorOption chosen for that pixel. This is easier
//to compare against each other than RGB values. Also takes up less memory.
#define NUMBER_OF_PIXELS 8
byte g_goal[NUMBER_OF_PIXELS];
byte g_guess[NUMBER_OF_PIXELS];

volatile int g_userPixel = 0; //Current pixel the user is changing the color of
volatile int g_currentConfigSetting = 0; //When in config mode this tracks which config setting is being set by the user
volatile unsigned long g_cheatTotalTime = 0; //How long did the user peek at the answer?
volatile unsigned long g_cheatStartTime = 0; //Used to calculate the above...
volatile unsigned long g_currentGameStarted = 0; //Used to show the game countdown timer
volatile unsigned long g_currentPreviewStarted = 0; //Used to show the preview countdown timer

//All the available options for different colors. Order chosen to avoid similar colors
//as long as possible.
PixelColor_t g_colorOptions[] = {
  {0, 0, 0}, //off
  {255, 0, 0,}, //red
  {0, 255, 0}, //green
  {0, 0, 255}, //blue
  {255, 255, 255}, // white
  {255, 0, 255}, //purple
  {255, 255, 0}, //yellow
  {0, 255, 255} //cyan
};

//State machine states
enum States_t {
  STATE_NEW_GAME, //Start up a new game
  STATE_MEMORIZE, //User is trying to memorize the right answer
  STATE_GUESSING, //User is trying to guess
  STATE_WIN,      //User has won
  STATE_LOST,     //User lost due to being too slow
  STATE_PEEKING,  //User is peeking/cheating
  STATE_CONFIG    //User is configuring future games
};

States_t g_currentState = STATE_NEW_GAME; //State transitions happen by changing this

//Setup only does things that need to happen once. Anything related to creating a new game happens
//in the NEW GAME state in the main loop.
void setup() {
  g_nps.begin();

  g_nps.useLightMeterToSeedRandom();

  g_nps.setRotaryEncoderButtonInterrupt(isr_nextPixel, BUTTON_PRESSED);
  g_nps.setDownButtonInterrupt(isr_newGame, BUTTON_PRESSED);
  g_nps.setLeftButtonInterrupt(isr_configMode, BUTTON_PRESSED);
  g_nps.setRightButtonInterrupt(isr_peekGoal, BUTTON_CHANGED);

  g_nps.oledSetTextSize(2);
}

void loop() {
  switch (g_currentState) {
    case STATE_NEW_GAME:
      showMode(F("Memorize!"));
      startNewGame();
      g_currentState = STATE_MEMORIZE;
      g_currentPreviewStarted = millis();
      break;
    
    case STATE_MEMORIZE:
      showMode(F("Memorize!"));
      if(millis() - g_currentPreviewStarted > g_gameSettings.secondsOfPreview * 1000){
        g_nps.rgbClear();
        g_currentState = STATE_GUESSING;
        g_currentGameStarted = millis();
      }
      showPreviewTimer();
      break;

    case STATE_GUESSING:
      showMode(F("Guess!"));
      g_guess[g_userPixel] = abs(g_nps.rotaryRead()) % (g_gameSettings.numberOfColorOptions) + 1;
      indicateCurrentPixel();
      showGuess();
      showGameTimer();
      if (didTheyWin()) {
        g_currentState = STATE_WIN;
      } else if (didTheyLose()) {
        g_currentState = STATE_LOST;
      }
      break;

    case STATE_WIN:
      g_nps.oledClear();
      g_nps.oledPrintln(F("UR Winner!"));
      if (g_cheatTotalTime > 0) {
        g_nps.oledPrintln(F("Cheated 4:"));
        g_nps.oledPrint(g_cheatTotalTime);
        g_nps.oledPrintln(F("ms"));
      }
      g_nps.oledPrintln(F("Down 4 New"));
      g_nps.oledDisplay();
      setRgbBrightness(true);
      g_nps.rgbShow();
      g_nps.shiftLedWrite(0);
      break;

    case STATE_LOST:
      g_nps.oledClear();
      g_nps.oledPrintln(F("TOO SLOW!"));
      g_nps.oledPrintln(F("Down 4 New"));
      g_nps.oledDisplay();
      g_nps.rgbClear();
      g_nps.shiftLedWrite(0);
      break;

    case STATE_PEEKING:
      update_cheating_time();
      g_nps.oledClear();
      g_nps.oledPrintln(F("Cheater!"));
      g_nps.oledPrint(g_cheatTotalTime);
      g_nps.oledPrintln(F("ms"));
      g_nps.oledDisplay();
      showGoal();
      showGameTimer();
      break;

    case STATE_CONFIG:
      g_nps.setRotaryEncoderButtonInterrupt(isr_nextSetting, BUTTON_PRESSED); //change the ISR for the rotary encoder button. Must be put back!
      g_nps.rotaryWrite(1);
      g_currentConfigSetting = 0;
      g_nps.clearAllDisplays();
      while (g_currentConfigSetting < 4) { //We don't leave this config mode until all settings are set!
        switch (g_currentConfigSetting) {
          case 0:
            showMode(F("Number of LEDS?"));
            g_gameSettings.numberOfLEDs = max(min(g_nps.rotaryRead(), NUMBER_OF_PIXELS), 1);
            g_nps.rotaryWrite(g_gameSettings.numberOfLEDs);
            g_nps.shift7segPrint(g_gameSettings.numberOfLEDs);
            break;
          case 1:
            showMode(F("Number of colors?"));
            g_gameSettings.numberOfColorOptions = max(min(g_nps.rotaryRead(), 7), 2);
            g_nps.rotaryWrite(g_gameSettings.numberOfColorOptions);
            g_nps.shift7segPrint(g_gameSettings.numberOfColorOptions);
            break;
          case 2:
            showMode(F("Seconds of preview?"));
            g_gameSettings.secondsOfPreview = max(min(g_nps.rotaryRead(), 10), 1);
            g_nps.rotaryWrite(g_gameSettings.secondsOfPreview);
            g_nps.shift7segPrint(g_gameSettings.secondsOfPreview);
            break;
          case 3:
            showMode(F("Seconds to guess?"));
            g_gameSettings.secondsOfGame = 5 * max(min(g_nps.rotaryRead(), 12), 1); //1*5=5 and 12*5=60
            g_nps.rotaryWrite(g_gameSettings.secondsOfGame/5);
            g_nps.shift7segPrint(g_gameSettings.secondsOfGame);
            break;
        }
      }
      writeSettingsToEEPROM();
      g_nps.setRotaryEncoderButtonInterrupt(isr_nextPixel, BUTTON_PRESSED); //putting the rotary encoder's ISR back to normal.
      g_currentState = STATE_NEW_GAME;
      break;
  }
}

void showMode(const __FlashStringHelper *modeText) {
  g_nps.oledClear();
  g_nps.oledPrintln(modeText);
  g_nps.oledDisplay();
}

void startNewGame() {
  getSettingsFromEEPROM();
  createNewGoal();
  resetGuess();
  setRgbBrightness(false);
  showGoal();
}

void createNewGoal() {
  byte goalIndex;
  int i;

  for (i = 0; i < g_gameSettings.numberOfLEDs; i++ ) {
    goalIndex = random(g_gameSettings.numberOfColorOptions) + 1;
    g_goal[i] = goalIndex;
  }
  for ( ; i < NUMBER_OF_PIXELS; i++) {
    g_goal[i] = 0;
  }

  g_cheatTotalTime = 0;
}

void resetGuess() {
  for (int i = 0; i < NUMBER_OF_PIXELS; i++ ) {
    g_guess[i] = 0;
  }
  g_userPixel = 0;
}

void setRgbBrightness(bool flash) {
  if (flash) {
    if (trueFourTimesASecond()) {
      g_nps.rgbSetBrightness(255);
    } else {
      g_nps.rgbSetBrightness(50);
    }
  } else { //don't flash
    g_nps.rgbSetBrightness(50);
  }
}

bool trueFourTimesASecond() {
  return ( ( millis() / 250 ) % 2 );
}

void showGoal() {
  for (int i = 0; i < NUMBER_OF_PIXELS; i++) {
    g_nps.rgbSetPixelColor( i, g_colorOptions[g_goal[i]].r, g_colorOptions[g_goal[i]].g, g_colorOptions[g_goal[i]].b);
  }
  g_nps.rgbShow();
}

void showPreviewTimer() {
  g_nps.shift7segPrint(g_gameSettings.secondsOfPreview - (millis() - g_currentPreviewStarted)/1000);
}

void showGameTimer() {
  g_nps.shift7segPrint(g_gameSettings.secondsOfGame - (millis() - g_currentGameStarted)/1000);
}

void indicateCurrentPixel() {
  g_nps.shiftLedWrite(1 << g_userPixel);
}

void showGuess() {
  for (int i = 0; i < NUMBER_OF_PIXELS; i++) {
    g_nps.rgbSetPixelColor( i, g_colorOptions[g_guess[i]].r, g_colorOptions[g_guess[i]].g, g_colorOptions[g_guess[i]].b);
  }
  g_nps.rgbShow();
}

bool didTheyWin() {
  bool theyWon = true;
  for (int i = 0; i < NUMBER_OF_PIXELS; i++) {
    theyWon = theyWon && ( g_goal[i] == g_guess[i] );
  }

  return theyWon;
}

bool didTheyLose() {
  return ( (millis() - g_currentGameStarted) > (g_gameSettings.secondsOfGame * 1000) );
}

void writeSettingsToEEPROM() {
  EEPROM.put(0, g_gameSettings);
}

void getSettingsFromEEPROM() {
  EEPROM.get(0, g_gameSettings);
}

void update_cheating_time(){
  g_cheatTotalTime += (millis() - g_cheatStartTime);
  g_cheatStartTime = millis();
}

void isr_nextPixel() {
  g_userPixel = (g_userPixel + 1) % g_gameSettings.numberOfLEDs;
  if (g_guess[g_userPixel] != 0) {
    g_nps.rotaryWrite(g_guess[g_userPixel] - 1);
  }
}

void isr_newGame() {
  g_currentState = STATE_NEW_GAME;
}

void isr_peekGoal() {
  if (g_currentState == STATE_GUESSING) {
    g_currentState = STATE_PEEKING;
    g_cheatStartTime = millis();
  } else if (g_currentState == STATE_PEEKING) {
    g_currentState = STATE_GUESSING;
    update_cheating_time();
  }
}

void isr_configMode() {
  g_currentState = STATE_CONFIG;
}

void isr_nextSetting() {
  g_currentConfigSetting++;
}
