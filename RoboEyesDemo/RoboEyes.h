/***************************************************
 * RoboEyes.h - Animated Robot Eyes for ST7789
 * Based on FluxGarage RoboEyes Library
 * Adapted for Adafruit ST7789 displays
 ***************************************************/

#ifndef ROBOEYES_H
#define ROBOEYES_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Mood types
enum Mood {
  MOOD_DEFAULT,
  MOOD_HAPPY,
  MOOD_SAD,
  MOOD_ANGRY,
  MOOD_TIRED
};

// Position types (8 directions + center)
enum Position {
  POS_DEFAULT,  // Center
  POS_N,        // North (up)
  POS_NE,       // North-East
  POS_E,        // East (right)
  POS_SE,       // South-East
  POS_S,        // South (down)
  POS_SW,       // South-West
  POS_W,        // West (left)
  POS_NW        // North-West
};

class RoboEyes {
public:
  RoboEyes(Adafruit_ST7789 &display);
  
  // Initialization
  void begin(uint16_t screenWidth, uint16_t screenHeight, uint8_t maxFPS = 30);
  
  // Update & Draw
  void update();
  void drawEyes();
  
  // Eye shape configuration
  void setWidth(uint8_t leftEye, uint8_t rightEye);
  void setHeight(uint8_t leftEye, uint8_t rightEye);
  void setBorderRadius(uint8_t leftEye, uint8_t rightEye);
  void setSpaceBetween(int16_t space);
  void setCyclops(bool enable);
  
  // Face expressions
  void setMood(Mood mood);
  void setPosition(Position pos);
  void setCuriosity(bool enable);
  void setSweat(bool enable);
  
  // Open/Close
  void open(bool leftEye = true, bool rightEye = true);
  void close(bool leftEye = true, bool rightEye = true);
  
  // Animations
  void blink(bool leftEye = true, bool rightEye = true);
  void anim_confused();
  void anim_laugh();
  
  // Auto behaviors
  void setAutoBlinker(bool enable, uint16_t interval = 3, uint16_t variation = 2);
  void setIdleMode(bool enable, uint16_t interval = 5, uint16_t variation = 3);
  
  // Display colors
  void setColors(uint16_t background, uint16_t main);
  
  // Public access to state (for demo purposes)
  bool _curiosity;
  bool _isCyclops;
  bool _autoBlinkEnabled;
  bool _idleModeEnabled;

private:
  Adafruit_ST7789 &_display;
  
  // Screen properties
  uint16_t _screenWidth;
  uint16_t _screenHeight;
  uint8_t _maxFPS;
  unsigned long _lastFrameTime;
  
  // Eye properties
  struct Eye {
    uint8_t width;
    uint8_t height;
    uint8_t borderRadius;
    int16_t x;            // Current position
    int16_t y;
    int16_t targetX;      // Target position for smooth movement
    int16_t targetY;
    float openAmount;     // 0.0 = closed, 1.0 = fully open
    float targetOpen;
  // Previous frame values (used to compute minimal redraw region)
  int16_t prevX;
  int16_t prevY;
  float prevOpenAmount;
    
    // Mood modifiers
    float upperLidAngle;  // For mood expressions
    float lowerLidAngle;
  };
  
  Eye _leftEye;
  Eye _rightEye;
  int16_t _spaceBetween;
  
  // Current state
  Mood _currentMood;
  Position _currentPosition;
  bool _sweat;
  
  // Colors
  uint16_t _colorBg;
  uint16_t _colorMain;
  
  // Auto behaviors (moved private variables here)
  uint16_t _blinkInterval;
  uint16_t _blinkVariation;
  unsigned long _lastBlinkTime;
  
  // Idle mode (moved private variables here)
  uint16_t _idleInterval;
  uint16_t _idleVariation;
  unsigned long _lastIdleTime;
  
  // Animation state
  bool _isBlinking;
  unsigned long _blinkStartTime;
  uint16_t _blinkDuration;
  
  // Internal methods
  void updateEyePositions();
  void updateEyeOpenAmount();
  void updateAutoBehaviors();
  void applyMoodToEye(Eye &eye);
  void drawSingleEye(Eye &eye);
  void drawEyeShape(int16_t centerX, int16_t centerY, Eye &eye);
  void calculateTargetPosition(Position pos, int16_t &leftX, int16_t &leftY, int16_t &rightX, int16_t &rightY);
  float easeInOutCubic(float t);
};

#endif // ROBOEYES_H
