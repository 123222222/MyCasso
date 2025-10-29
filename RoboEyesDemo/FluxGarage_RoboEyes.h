/*
 * FluxGarage RoboEyes for ST7789 Displays - ported version
 * Adapted from FluxGarage_RoboEyes (originally for OLED) to work with
 * Adafruit ST7789 displays (ST7789). Changes: use 16-bit color, immediate
 * draws (fillScreen) instead of buffered display->display(), and keep
 * original API/behaviour as much as possible.
 *
 * Original Copyright (C) 2024-2025 Dennis Hoelscher
 * This port preserves the original GPLv3 license.
 * See original: https://github.com/FluxGarage/RoboEyes
 */

#ifndef _FLUXGARAGE_ROBOEYES_ST7789_H
#define _FLUXGARAGE_ROBOEYES_ST7789_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// Display colors (16-bit for ST77xx)
uint16_t BGCOLOR = ST77XX_BLACK; // background and overlays
uint16_t MAINCOLOR = ST77XX_CYAN; // drawings

// For mood type switch
#define DEFAULT 0
#define TIRED 1
#define ANGRY 2
#define HAPPY 3

// For turning things on or off
#define ON 1
#define OFF 0

// For switch "predefined positions"
#define N 1
#define NE 2
#define E 3
#define SE 4
#define S 5
#define SW 6
#define W 7
#define NW 8

// Constructor: takes a reference to the active Adafruit display object
template<typename AdafruitDisplay>
class RoboEyes {
private:

public:

  AdafruitDisplay *display;

  // Screen / framerate
  int screenWidth = 240;
  int screenHeight = 320;
  int frameInterval = 1000/30;
  unsigned long fpsTimer = 0;

  // Moods & flags
  bool tired = 0;
  bool angry = 0;
  bool happy = 0;
  bool curious = 0;
  bool cyclops = 0;
  bool eyeL_open = 0;
  bool eyeR_open = 0;

  // EYE geometry defaults
  int eyeLwidthDefault = 36;
  int eyeLheightDefault = 36;
  int eyeLwidthCurrent = 36;
  int eyeLheightCurrent = 1;
  int eyeLwidthNext = 36;
  int eyeLheightNext = 36;
  int eyeLheightOffset = 0;
  byte eyeLborderRadiusDefault = 8;
  byte eyeLborderRadiusCurrent = 8;
  byte eyeLborderRadiusNext = 8;

  int eyeRwidthDefault = 36;
  int eyeRheightDefault = 36;
  int eyeRwidthCurrent = 36;
  int eyeRheightCurrent = 1;
  int eyeRwidthNext = 36;
  int eyeRheightNext = 36;
  int eyeRheightOffset = 0;
  byte eyeRborderRadiusDefault = 8;
  byte eyeRborderRadiusCurrent = 8;
  byte eyeRborderRadiusNext = 8;

  // Positions
  int spaceBetweenDefault = 10;
  int spaceBetweenCurrent = 10;
  int spaceBetweenNext = 10;

  int eyeLxDefault = 0;
  int eyeLyDefault = 0;
  int eyeLx = 0;
  int eyeLy = 0;
  int eyeLxNext = 0;
  int eyeLyNext = 0;

  int eyeRxDefault = 0;
  int eyeRyDefault = 0;
  int eyeRx = 0;
  int eyeRy = 0;
  int eyeRxNext = 0;
  int eyeRyNext = 0;

  // Eyelid values
  byte eyelidsHeightMax = 18;
  byte eyelidsTiredHeight = 0;
  byte eyelidsTiredHeightNext = 0;
  byte eyelidsAngryHeight = 0;
  byte eyelidsAngryHeightNext = 0;
  byte eyelidsHappyBottomOffsetMax = 21;
  byte eyelidsHappyBottomOffset = 0;
  byte eyelidsHappyBottomOffsetNext = 0;

  // Macro animations
  bool hFlicker = 0;
  bool hFlickerAlternate = 0;
  byte hFlickerAmplitude = 2;

  bool vFlicker = 0;
  bool vFlickerAlternate = 0;
  byte vFlickerAmplitude = 10;

  bool autoblinker = 0;
  int blinkInterval = 3;
  int blinkIntervalVariation = 2;
  unsigned long blinktimer = 0;

  bool idle = 0;
  int idleInterval = 2;
  int idleIntervalVariation = 2;
  unsigned long idleAnimationTimer = 0;

  bool confused = 0;
  unsigned long confusedAnimationTimer = 0;
  int confusedAnimationDuration = 500;
  bool confusedToggle = 1;

  bool laugh = 0;
  unsigned long laughAnimationTimer = 0;
  int laughAnimationDuration = 500;
  bool laughToggle = 1;

  bool sweat = 0;
  byte sweatBorderradius = 3;

  // sweat drops
  int sweat1XPosInitial = 2;
  int sweat1XPos;
  float sweat1YPos = 2;
  int sweat1YPosMax;
  float sweat1Height = 2;
  float sweat1Width = 1;

  int sweat2XPosInitial = 2;
  int sweat2XPos;
  float sweat2YPos = 2;
  int sweat2YPosMax;
  float sweat2Height = 2;
  float sweat2Width = 1;

  int sweat3XPosInitial = 2;
  int sweat3XPos;
  float sweat3YPos = 2;
  int sweat3YPosMax;
  float sweat3Height = 2;
  float sweat3Width = 1;

  // Constructor
  RoboEyes(AdafruitDisplay &disp) : display(&disp) {
    // nothing
  }

  void begin(int width, int height, byte frameRate) {
    screenWidth = width;
    screenHeight = height;
    // Initialize default positions based on screen size
    eyeLwidthDefault = eyeLwidthCurrent = eyeLwidthNext = 36;
    eyeLheightDefault = eyeLheightCurrent = eyeLheightNext = 36;
    spaceBetweenDefault = spaceBetweenCurrent = spaceBetweenNext = 10;
    // compute defaults
    eyeLxDefault = (screenWidth - (eyeLwidthDefault + spaceBetweenDefault + eyeRwidthDefault)) / 2;
    eyeLyDefault = (screenHeight - eyeLheightDefault) / 2;
    eyeLx = eyeLxDefault; eyeLy = eyeLyDefault; eyeLxNext = eyeLx;
    eyeRx = eyeLx + eyeLwidthCurrent + spaceBetweenCurrent;
    eyeRxNext = eyeRx; eyeRy = eyeLy; eyeRyNext = eyeRy;

    // clear screen once
    display->fillScreen(BGCOLOR);

    eyeLheightCurrent = 1;
    eyeRheightCurrent = 1;
    
    setFramerate(frameRate);
  }

  void update(){
    if(millis() - fpsTimer >= frameInterval){
      drawEyes();
      fpsTimer = millis();
    }
  }

  void setFramerate(byte fps){
    frameInterval = 1000 / fps;
  }

  void setDisplayColors(uint16_t background, uint16_t main) {
    BGCOLOR = background;
    MAINCOLOR = main;
  }

  void setWidth(byte leftEye, byte rightEye) {
    eyeLwidthNext = leftEye;
    eyeRwidthNext = rightEye;
    eyeLwidthDefault = leftEye;
    eyeRwidthDefault = rightEye;
  }

  void setHeight(byte leftEye, byte rightEye) {
    eyeLheightNext = leftEye;
    eyeRheightNext = rightEye;
    eyeLheightDefault = leftEye;
    eyeRheightDefault = rightEye;
  }

  void setBorderradius(byte leftEye, byte rightEye) {
    eyeLborderRadiusNext = leftEye;
    eyeRborderRadiusNext = rightEye;
    eyeLborderRadiusDefault = leftEye;
    eyeRborderRadiusDefault = rightEye;
  }

  void setSpacebetween(int space) {
    spaceBetweenNext = space;
    spaceBetweenDefault = space;
  }

  void setMood(unsigned char mood) {
    tired = angry = happy = 0;
    switch(mood){
      case TIRED: tired = 1; break;
      case ANGRY: angry = 1; break;
      case HAPPY: happy = 1; break;
      default: break;
    }
  }

  int getScreenConstraint_X(){
    return screenWidth - eyeLwidthCurrent - spaceBetweenCurrent - eyeRwidthCurrent;
  }

  int getScreenConstraint_Y(){
    return screenHeight - eyeLheightDefault;
  }

  void setPosition(unsigned char position){
    switch(position){
      case N: eyeLxNext = getScreenConstraint_X()/2; eyeLyNext = 0; break;
      case NE: eyeLxNext = getScreenConstraint_X(); eyeLyNext = 0; break;
      case E: eyeLxNext = getScreenConstraint_X(); eyeLyNext = getScreenConstraint_Y()/2; break;
      case SE: eyeLxNext = getScreenConstraint_X(); eyeLyNext = getScreenConstraint_Y(); break;
      case S: eyeLxNext = getScreenConstraint_X()/2; eyeLyNext = getScreenConstraint_Y(); break;
      case SW: eyeLxNext = 0; eyeLyNext = getScreenConstraint_Y(); break;
      case W: eyeLxNext = 0; eyeLyNext = getScreenConstraint_Y()/2; break;
      case NW: eyeLxNext = 0; eyeLyNext = 0; break;
      default: eyeLxNext = getScreenConstraint_X()/2; eyeLyNext = getScreenConstraint_Y()/2; break;
    }
  }

  void setAutoblinker(bool active, int interval, int variation){ autoblinker = active; blinkInterval = interval; blinkIntervalVariation = variation; }
  void setAutoblinker(bool active){ autoblinker = active; }
  void setIdleMode(bool active, int interval, int variation){ idle = active; idleInterval = interval; idleIntervalVariation = variation; }
  void setIdleMode(bool active){ idle = active; }
  void setCuriosity(bool curiousBit){ curious = curiousBit; }
  void setCyclops(bool cyclopsBit){ cyclops = cyclopsBit; }
  void setHFlicker(bool flickerBit, byte Amplitude){ hFlicker = flickerBit; hFlickerAmplitude = Amplitude; }
  void setHFlicker(bool flickerBit){ hFlicker = flickerBit; }
  void setVFlicker(bool flickerBit, byte Amplitude){ vFlicker = flickerBit; vFlickerAmplitude = Amplitude; }
  void setVFlicker(bool flickerBit){ vFlicker = flickerBit; }
  void setSweat(bool sweatBit){ sweat = sweatBit; }

  void close(){ eyeLheightNext = 1; eyeRheightNext = 1; eyeL_open = 0; eyeR_open = 0; }
  void open(){ eyeL_open = 1; eyeR_open = 1; }
  void blink(){ close(); open(); }
  void close(bool left, bool right){ if(left){ eyeLheightNext = 1; eyeL_open = 0; } if(right){ eyeRheightNext = 1; eyeR_open = 0; } }
  void open(bool left, bool right){ if(left) eyeL_open = 1; if(right) eyeR_open = 1; }
  void blink(bool left, bool right){ close(left,right); open(left,right); }
  void anim_confused(){ confused = 1; }
  void anim_laugh(){ laugh = 1; }

  void drawEyes(){
    // Pre-calculations
    if(curious){
      if(eyeLxNext<=10) eyeLheightOffset=8; else if (eyeLxNext>=(getScreenConstraint_X()-10) && cyclops) eyeLheightOffset=8; else eyeLheightOffset=0;
      if(eyeRxNext>=screenWidth-eyeRwidthCurrent-10) eyeRheightOffset=8; else eyeRheightOffset=0;
    } else { eyeLheightOffset=0; eyeRheightOffset=0; }

    eyeLheightCurrent = (eyeLheightCurrent + eyeLheightNext + eyeLheightOffset)/2;
    eyeLy += ((eyeLheightDefault-eyeLheightCurrent)/2);
    eyeLy -= eyeLheightOffset/2;
    eyeRheightCurrent = (eyeRheightCurrent + eyeRheightNext + eyeRheightOffset)/2;
    eyeRy += (eyeRheightDefault-eyeRheightCurrent)/2;
    eyeRy -= eyeRheightOffset/2;

    if(eyeL_open){ if(eyeLheightCurrent <= 1 + eyeLheightOffset) eyeLheightNext = eyeLheightDefault; }
    if(eyeR_open){ if(eyeRheightCurrent <= 1 + eyeRheightOffset) eyeRheightNext = eyeRheightDefault; }

    eyeLwidthCurrent = (eyeLwidthCurrent + eyeLwidthNext)/2;
    eyeRwidthCurrent = (eyeRwidthCurrent + eyeRwidthNext)/2;
    spaceBetweenCurrent = (spaceBetweenCurrent + spaceBetweenNext)/2;

    eyeLx = (eyeLx + eyeLxNext)/2;
    eyeLy = (eyeLy + eyeLyNext)/2;
    eyeRxNext = eyeLxNext + eyeLwidthCurrent + spaceBetweenCurrent;
    eyeRyNext = eyeLyNext;
    eyeRx = (eyeRx + eyeRxNext)/2;
    eyeRy = (eyeRy + eyeRyNext)/2;

    eyeLborderRadiusCurrent = (eyeLborderRadiusCurrent + eyeLborderRadiusNext)/2;
    eyeRborderRadiusCurrent = (eyeRborderRadiusCurrent + eyeRborderRadiusNext)/2;

    // Autoblink
    if(autoblinker){ if(millis() >= blinktimer){ blink(); blinktimer = millis() + (blinkInterval*1000) + (random(blinkIntervalVariation)*1000); } }

    // Laugh
    if(laugh){ if(laughToggle){ setVFlicker(1,5); laughAnimationTimer = millis(); laughToggle = 0; } else if(millis() >= laughAnimationTimer + laughAnimationDuration){ setVFlicker(0,0); laughToggle = 1; laugh = 0; } }

    // Confused
    if(confused){ if(confusedToggle){ setHFlicker(1,20); confusedAnimationTimer = millis(); confusedToggle = 0; } else if(millis() >= confusedAnimationTimer + confusedAnimationDuration){ setHFlicker(0,0); confusedToggle = 1; confused = 0; } }

    // Idle
    if(idle){ if(millis() >= idleAnimationTimer){ eyeLxNext = random(getScreenConstraint_X()); eyeLyNext = random(getScreenConstraint_Y()); idleAnimationTimer = millis() + (idleInterval*1000) + (random(idleIntervalVariation)*1000); } }

    // Flicker offsets
    if(hFlicker){ if(hFlickerAlternate){ eyeLx += hFlickerAmplitude; eyeRx += hFlickerAmplitude; } else { eyeLx -= hFlickerAmplitude; eyeRx -= hFlickerAmplitude; } hFlickerAlternate = !hFlickerAlternate; }
    if(vFlicker){ if(vFlickerAlternate){ eyeLy += vFlickerAmplitude; eyeRy += vFlickerAmplitude; } else { eyeLy -= vFlickerAmplitude; eyeRy -= vFlickerAmplitude; } vFlickerAlternate = !vFlickerAlternate; }

    if(cyclops){ eyeRwidthCurrent = 0; eyeRheightCurrent = 0; spaceBetweenCurrent = 0; }

    // Draw - clear minimal regions by simply drawing BG first for whole screen
    // (ST7789 is fast on ESP32; if you want further optimization we can use sprites)
    display->fillScreen(BGCOLOR);

    // Draw eyes
    display->fillRoundRect(eyeLx, eyeLy, eyeLwidthCurrent, eyeLheightCurrent, eyeLborderRadiusCurrent, MAINCOLOR);
    if(!cyclops) display->fillRoundRect(eyeRx, eyeRy, eyeRwidthCurrent, eyeRheightCurrent, eyeRborderRadiusCurrent, MAINCOLOR);

    // Mood transitions
    if(tired){ eyelidsTiredHeightNext = eyeLheightCurrent/2; eyelidsAngryHeightNext = 0; } else { eyelidsTiredHeightNext = 0; }
    if(angry){ eyelidsAngryHeightNext = eyeLheightCurrent/2; eyelidsTiredHeightNext = 0; } else { eyelidsAngryHeightNext = 0; }
    if(happy){ eyelidsHappyBottomOffsetNext = eyeLheightCurrent/2; } else { eyelidsHappyBottomOffsetNext = 0; }

    // Tired eyelids (top)
    eyelidsTiredHeight = (eyelidsTiredHeight + eyelidsTiredHeightNext)/2;
    if(!cyclops){
      display->fillTriangle(eyeLx, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx, eyeLy+eyelidsTiredHeight-1, BGCOLOR);
      display->fillTriangle(eyeRx, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy+eyelidsTiredHeight-1, BGCOLOR);
    } else {
      display->fillTriangle(eyeLx, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx, eyeLy+eyelidsTiredHeight-1, BGCOLOR);
      display->fillTriangle(eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy+eyelidsTiredHeight-1, BGCOLOR);
    }

    // Angry eyelids
    eyelidsAngryHeight = (eyelidsAngryHeight + eyelidsAngryHeightNext)/2;
    if(!cyclops){
      display->fillTriangle(eyeLx, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy+eyelidsAngryHeight-1, BGCOLOR);
      display->fillTriangle(eyeRx, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy-1, eyeRx, eyeRy+eyelidsAngryHeight-1, BGCOLOR);
    } else {
      display->fillTriangle(eyeLx, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy+eyelidsAngryHeight-1, BGCOLOR);
      display->fillTriangle(eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy+eyelidsAngryHeight-1, BGCOLOR);
    }

    // Happy bottom eyelids
    eyelidsHappyBottomOffset = (eyelidsHappyBottomOffset + eyelidsHappyBottomOffsetNext)/2;
    display->fillRoundRect(eyeLx-1, (eyeLy+eyeLheightCurrent)-eyelidsHappyBottomOffset+1, eyeLwidthCurrent+2, eyeLheightDefault, eyeLborderRadiusCurrent, BGCOLOR);
    if(!cyclops) display->fillRoundRect(eyeRx-1, (eyeRy+eyeRheightCurrent)-eyelidsHappyBottomOffset+1, eyeRwidthCurrent+2, eyeRheightDefault, eyeRborderRadiusCurrent, BGCOLOR);

    // Sweat
    if(sweat){
      if(sweat1YPos <= sweat1YPosMax) sweat1YPos += 0.5; else { sweat1XPosInitial = random(30); sweat1YPos = 2; sweat1YPosMax = (random(10)+10); sweat1Width = 1; sweat1Height = 2; }
      if(sweat1YPos <= sweat1YPosMax/2) { sweat1Width += 0.5; sweat1Height += 0.5; } else { sweat1Width -= 0.1; sweat1Height -= 0.5; }
      sweat1XPos = sweat1XPosInitial - (sweat1Width/2);
      display->fillRoundRect(sweat1XPos, sweat1YPos, (int)sweat1Width, (int)sweat1Height, sweatBorderradius, MAINCOLOR);

      if(sweat2YPos <= sweat2YPosMax) sweat2YPos += 0.5; else { sweat2XPosInitial = random((screenWidth-60))+30; sweat2YPos = 2; sweat2YPosMax = (random(10)+10); sweat2Width = 1; sweat2Height = 2; }
      if(sweat2YPos <= sweat2YPosMax/2) { sweat2Width += 0.5; sweat2Height += 0.5; } else { sweat2Width -= 0.1; sweat2Height -= 0.5; }
      sweat2XPos = sweat2XPosInitial - (sweat2Width/2);
      display->fillRoundRect(sweat2XPos, sweat2YPos, (int)sweat2Width, (int)sweat2Height, sweatBorderradius, MAINCOLOR);

      if(sweat3YPos <= sweat3YPosMax) sweat3YPos += 0.5; else { sweat3XPosInitial = (screenWidth-30)+(random(30)); sweat3YPos = 2; sweat3YPosMax = (random(10)+10); sweat3Width = 1; sweat3Height = 2; }
      if(sweat3YPos <= sweat3YPosMax/2) { sweat3Width += 0.5; sweat3Height += 0.5; } else { sweat3Width -= 0.1; sweat3Height -= 0.5; }
      sweat3XPos = sweat3XPosInitial - (sweat3Width/2);
      display->fillRoundRect(sweat3XPos, sweat3YPos, (int)sweat3Width, (int)sweat3Height, sweatBorderradius, MAINCOLOR);
    }  }

};

#endif
