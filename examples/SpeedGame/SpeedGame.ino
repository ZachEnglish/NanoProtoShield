/*  SpeedGame.ino

    Purpose:
      Show how to use random time delays to make a game that tests reaction
      time. Uses lots of global state.

    What it does:
      Shows red RGB LEDs and a "Get ready" message. When the RGB LEDs go
      green you are supposed to press and hold the up button as fast as you
      can. Your reaction time is displayed and the highest score (fastest
      time) is saved as a reference. Get your best score!

    TODOs (Things that are broken/not yet working/could be improved):
      None

    Special Notes:
      None
*/

#include "NanoProtoShield.h"

NanoProtoShield g_nps;

int gameDelay; //how many milliseconds to wait before testing the reaction time
unsigned long startTime;
unsigned long reactionTime;
unsigned long fastestTime;
bool buttonAlreadyPressed;  //here to make sure we only handle the button press one time
bool hasButtonBeenUp;       //only here to check for cheaters
bool gameInProgress;        //used to know if a game is currently in progress. False when a game has finished.

void setup(){
    g_nps.begin();

    g_nps.useLightMeterToSeedRandom();
    gameInProgress = false;
    fastestTime = 10000;
}

void loop(){
    if(g_nps.buttonUpPressed()){
        //the button is pressed. See if it has even been up in this game (make sure they are not cheating)
        if(hasButtonBeenUp == false){
            //they are totally cheating
            buttonAlreadyPressed = true;
            displayCheater();

            gameInProgress = false;
        }
        if(buttonAlreadyPressed == false){
            //the button IS pressed and it hasn't been handled yet
            buttonAlreadyPressed = true;

            reactionTime = millis() - startTime;
            if(reactionTime < fastestTime){
                fastestTime = reactionTime;
            }
            
            displayEndGame();
            
            gameInProgress = false;
        }
    } else { //Up button is, uh, well, up
        hasButtonBeenUp = true; //not cheating

        if(gameInProgress == false){ //if we don't have a game in progress, start one!
            gameInProgress = true;
            buttonAlreadyPressed = false;
            hasButtonBeenUp = false;

            playGame();
        }
    }
}

void getNewGameDelay(int lowSec, int highSec){
    gameDelay = random(lowSec*1000,highSec*1000); //one second to ten seconds
}

void displayGetReady(){
    g_nps.rgbSetPixelsColor(255,0,0);
    g_nps.rgbShow();

    g_nps.oledClear();
    g_nps.oledPrintln("Get ready...");
    //g_nps.oledPrint("Your delay is ");
    //g_nps.oledPrintln(gameDelay);
    g_nps.oledDisplay();
}

void displayGo(){
    g_nps.rgbSetPixelsColor(0,255,0);
    g_nps.rgbShow();

    g_nps.oledPrintln("Go!!!");
    g_nps.oledDisplay();
}

void displayCheater(){
    g_nps.rgbSetPixelsColor(255,255,0);
    g_nps.rgbShow();

    g_nps.oledClear();
    g_nps.oledPrintln("You cheater!!");
    g_nps.oledDisplay();
}

void displayEndGame(){
    g_nps.rgbSetPixelsColor(0,0,255);
    g_nps.rgbShow();

    g_nps.oledPrintln("Your time:");
    g_nps.oledPrint(reactionTime);
    g_nps.oledPrintln("ms");
    g_nps.oledPrintln("Fastest time:");
    g_nps.oledPrint(fastestTime);
    g_nps.oledPrintln("ms");
    if(reactionTime == fastestTime){
        g_nps.oledPrintln("High Score!!!");
    }
    g_nps.oledDisplay();
}

void playGame(){
    getNewGameDelay(1,10);

    displayGetReady();

    delay(gameDelay);

    displayGo();

    startTime = millis();
    reactionTime = 0;
}