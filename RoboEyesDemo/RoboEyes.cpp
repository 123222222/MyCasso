/***************************************************
 * RoboEyes.cpp - Implementation
 ***************************************************/

#include "RoboEyes.h"

RoboEyes::RoboEyes(Adafruit_ST7789 &display) 
  : _display(display),
    _screenWidth(240),
    _screenHeight(320),
    _maxFPS(30),
    _lastFrameTime(0),
    _spaceBetween(20),
    _isCyclops(false),
    _currentMood(MOOD_DEFAULT),
    _currentPosition(POS_DEFAULT),
    _curiosity(false),
    _sweat(false),
    _colorBg(ST77XX_BLACK),
    _colorMain(ST77XX_CYAN),
    _autoBlinkEnabled(true),
    _blinkInterval(3000),
    _blinkVariation(2000),
    _lastBlinkTime(0),
    _idleModeEnabled(true),
    _idleInterval(5000),
    _idleVariation(3000),
    _lastIdleTime(0),
    _isBlinking(false),
    _blinkStartTime(0),
    _blinkDuration(200)
{
  // Initialize left eye
  _leftEye.width = 40;
  _leftEye.height = 50;
  _leftEye.borderRadius = 15;
  _leftEye.x = 0;
  _leftEye.y = 0;
  _leftEye.targetX = 0;
  _leftEye.targetY = 0;
  _leftEye.openAmount = 1.0;
  _leftEye.targetOpen = 1.0;
  _leftEye.upperLidAngle = 0.0;
  _leftEye.lowerLidAngle = 0.0;
  _leftEye.prevX = _leftEye.x;
  _leftEye.prevY = _leftEye.y;
  _leftEye.prevOpenAmount = _leftEye.openAmount;
  
  // Initialize right eye
  _rightEye = _leftEye;
  _rightEye.prevX = _rightEye.x;
  _rightEye.prevY = _rightEye.y;
  _rightEye.prevOpenAmount = _rightEye.openAmount;
}

void RoboEyes::begin(uint16_t screenWidth, uint16_t screenHeight, uint8_t maxFPS) {
  _screenWidth = screenWidth;
  _screenHeight = screenHeight;
  _maxFPS = maxFPS;
  
  // Set initial positions
  _leftEye.x = _screenWidth / 2 - _spaceBetween / 2 - _leftEye.width / 2;
  _leftEye.y = _screenHeight / 2;
  _leftEye.targetX = _leftEye.x;
  _leftEye.targetY = _leftEye.y;
  
  _rightEye.x = _screenWidth / 2 + _spaceBetween / 2 + _rightEye.width / 2;
  _rightEye.y = _screenHeight / 2;
  _rightEye.targetX = _rightEye.x;
  _rightEye.targetY = _rightEye.y;
  
  _lastBlinkTime = millis();
  _lastIdleTime = millis();

  // Ensure initial screen is cleared to the background color to avoid
  // leftover pixels from previous runs or bootloader output.
  _display.fillScreen(_colorBg);
}

void RoboEyes::update() {
  // Frame rate limiting
  unsigned long currentTime = millis();
  unsigned long frameDelay = 1000 / _maxFPS;
  
  if (currentTime - _lastFrameTime < frameDelay) {
    return;
  }
  _lastFrameTime = currentTime;
  
  // Update behaviors
  updateAutoBehaviors();
  
  // Update eye states
  updateEyePositions();
  updateEyeOpenAmount();
  
  // Draw
  drawEyes();
}

void RoboEyes::drawEyes() {
  // Draw each eye using minimal dirty rectangles computed from previous
  // and current positions. drawSingleEye will clear only the union box
  // and then draw the new eye.

  if (!_isCyclops) {
    drawSingleEye(_leftEye);
    drawSingleEye(_rightEye);
  } else {
    // Cyclops mode - temporarily draw left eye centered and update its prev
    int16_t origX = _leftEye.x;
    _leftEye.x = _screenWidth / 2;
    drawSingleEye(_leftEye);
    // Keep logical position unchanged
    _leftEye.x = origX;
  }

  // Sweat: clear only a small area and draw
  if (_sweat) {
    int16_t sweatX = _screenWidth / 2 - 30;
    int16_t sweatY = 20 + ((millis() / 100) % 10);
    _display.fillRect(sweatX - 6, sweatY - 6, 24, 24, _colorBg);
    _display.fillCircle(sweatX, sweatY, 3, _colorMain);
    _display.fillCircle(sweatX, sweatY + 8, 4, _colorMain);
  }
}

void RoboEyes::setWidth(uint8_t leftEye, uint8_t rightEye) {
  _leftEye.width = leftEye;
  _rightEye.width = rightEye;
}

void RoboEyes::setHeight(uint8_t leftEye, uint8_t rightEye) {
  _leftEye.height = leftEye;
  _rightEye.height = rightEye;
}

void RoboEyes::setBorderRadius(uint8_t leftEye, uint8_t rightEye) {
  _leftEye.borderRadius = leftEye;
  _rightEye.borderRadius = rightEye;
}

void RoboEyes::setSpaceBetween(int16_t space) {
  _spaceBetween = space;
  // Recalculate positions
  _leftEye.targetX = _screenWidth / 2 - _spaceBetween / 2 - _leftEye.width / 2;
  _rightEye.targetX = _screenWidth / 2 + _spaceBetween / 2 + _rightEye.width / 2;
}

void RoboEyes::setCyclops(bool enable) {
  _isCyclops = enable;
}

void RoboEyes::setMood(Mood mood) {
  _currentMood = mood;
  applyMoodToEye(_leftEye);
  applyMoodToEye(_rightEye);
}

void RoboEyes::setPosition(Position pos) {
  _currentPosition = pos;
  
  int16_t leftX, leftY, rightX, rightY;
  calculateTargetPosition(pos, leftX, leftY, rightX, rightY);
  
  _leftEye.targetX = leftX;
  _leftEye.targetY = leftY;
  _rightEye.targetX = rightX;
  _rightEye.targetY = rightY;
}

void RoboEyes::setCuriosity(bool enable) {
  _curiosity = enable;
}

void RoboEyes::setSweat(bool enable) {
  _sweat = enable;
}

void RoboEyes::open(bool leftEye, bool rightEye) {
  if (leftEye) _leftEye.targetOpen = 1.0;
  if (rightEye) _rightEye.targetOpen = 1.0;
}

void RoboEyes::close(bool leftEye, bool rightEye) {
  if (leftEye) _leftEye.targetOpen = 0.0;
  if (rightEye) _rightEye.targetOpen = 0.0;
}

void RoboEyes::blink(bool leftEye, bool rightEye) {
  if (!_isBlinking) {
    _isBlinking = true;
    _blinkStartTime = millis();
    
    if (leftEye) _leftEye.targetOpen = 0.0;
    if (rightEye) _rightEye.targetOpen = 0.0;
  }
}

void RoboEyes::anim_confused() {
  // Shake left and right quickly
  for (int i = 0; i < 3; i++) {
    setPosition(POS_E);
    delay(100);
    drawEyes();
    setPosition(POS_W);
    delay(100);
    drawEyes();
  }
  setPosition(POS_DEFAULT);
}

void RoboEyes::anim_laugh() {
  // Shake up and down
  for (int i = 0; i < 3; i++) {
    setPosition(POS_N);
    delay(100);
    drawEyes();
    setPosition(POS_S);
    delay(100);
    drawEyes();
  }
  setPosition(POS_DEFAULT);
}

void RoboEyes::setAutoBlinker(bool enable, uint16_t interval, uint16_t variation) {
  _autoBlinkEnabled = enable;
  _blinkInterval = interval * 1000;
  _blinkVariation = variation * 1000;
}

void RoboEyes::setIdleMode(bool enable, uint16_t interval, uint16_t variation) {
  _idleModeEnabled = enable;
  _idleInterval = interval * 1000;
  _idleVariation = variation * 1000;
}

void RoboEyes::setColors(uint16_t background, uint16_t main) {
  _colorBg = background;
  _colorMain = main;
}

// ========== Internal Methods ==========

void RoboEyes::updateEyePositions() {
  // Smooth movement using easing
  float speed = 0.15; // Movement speed (0.0 - 1.0)
  
  _leftEye.x += (_leftEye.targetX - _leftEye.x) * speed;
  _leftEye.y += (_leftEye.targetY - _leftEye.y) * speed;
  
  _rightEye.x += (_rightEye.targetX - _rightEye.x) * speed;
  _rightEye.y += (_rightEye.targetY - _rightEye.y) * speed;
}

void RoboEyes::updateEyeOpenAmount() {
  float speed = 0.2;
  
  _leftEye.openAmount += (_leftEye.targetOpen - _leftEye.openAmount) * speed;
  _rightEye.openAmount += (_rightEye.targetOpen - _rightEye.openAmount) * speed;
  
  // Handle blink animation
  if (_isBlinking) {
    unsigned long elapsed = millis() - _blinkStartTime;
    
    if (elapsed < _blinkDuration / 2) {
      // Closing
      float progress = (float)elapsed / (_blinkDuration / 2);
      _leftEye.openAmount = 1.0 - progress;
      _rightEye.openAmount = 1.0 - progress;
    } else if (elapsed < _blinkDuration) {
      // Opening
      float progress = (float)(elapsed - _blinkDuration / 2) / (_blinkDuration / 2);
      _leftEye.openAmount = progress;
      _rightEye.openAmount = progress;
    } else {
      // Blink finished
      _isBlinking = false;
      _leftEye.openAmount = 1.0;
      _rightEye.openAmount = 1.0;
      _leftEye.targetOpen = 1.0;
      _rightEye.targetOpen = 1.0;
    }
  }
}

void RoboEyes::updateAutoBehaviors() {
  unsigned long currentTime = millis();
  
  // Auto blink
  if (_autoBlinkEnabled && !_isBlinking) {
    unsigned long nextBlink = _lastBlinkTime + _blinkInterval + random(_blinkVariation);
    if (currentTime > nextBlink) {
      // Debug: indicate auto-blink triggered (useful to diagnose rapid blinking)
      Serial.print("[RoboEyes] Auto blink triggered at ");
      Serial.println(currentTime);

      blink(true, true);
      // Set last blink time to include blink duration as a short refractory period
      // This avoids immediate re-triggering in edge cases where timing math or
      // random() could allow another blink right after the previous one finishes.
      _lastBlinkTime = currentTime + _blinkDuration;
    }
  }
  
  // Idle mode - random eye movement
  if (_idleModeEnabled) {
    unsigned long nextIdle = _lastIdleTime + _idleInterval + random(_idleVariation);
    if (currentTime > nextIdle) {
      // Random position
      Position randomPos = (Position)random(POS_DEFAULT, POS_NW + 1);
      setPosition(randomPos);
      _lastIdleTime = currentTime;
    }
  }
}

void RoboEyes::applyMoodToEye(Eye &eye) {
  switch (_currentMood) {
    case MOOD_HAPPY:
      eye.upperLidAngle = -0.28;
      eye.lowerLidAngle = 0.28;
      eye.height = 36;
      eye.width = 40;
      eye.borderRadius = 12;
      break;
    
    case MOOD_SAD:
      eye.upperLidAngle = 0.32;
      eye.lowerLidAngle = -0.28;
      eye.height = 38;
      eye.width = 40;
      eye.borderRadius = 10;
      break;
    
    case MOOD_ANGRY:
      eye.upperLidAngle = -0.48;
      eye.lowerLidAngle = -0.18;
      eye.height = 34;
      eye.width = 38;
      eye.borderRadius = 8;
      break;
    
    case MOOD_TIRED:
      eye.upperLidAngle = 0.08;
      eye.lowerLidAngle = -0.1;
      eye.height = 20;
      eye.width = 40;
      eye.borderRadius = 6;
      break;
    
    case MOOD_DEFAULT:
    default:
      eye.upperLidAngle = 0.0;
      eye.lowerLidAngle = 0.0;
      eye.height = 50;
      eye.width = 40;
      eye.borderRadius = 15;
      break;
  }
}

void RoboEyes::drawSingleEye(Eye &eye) {
  // Compute current and previous bounding rects and clear only their union.
  // Compute a margin that safely covers rounded corners and lid lines.
  // Use the eye borderRadius as a basis and add a small safety padding.
  int16_t margin = eye.borderRadius + 6;

  int16_t wCurr = eye.width;
  int16_t hCurr = (int16_t)(eye.height * eye.openAmount);
  int16_t xCurr = eye.x - wCurr / 2 - margin;
  int16_t yCurr = eye.y - hCurr / 2 - margin;
  int16_t wCurrBox = wCurr + margin * 2;
  int16_t hCurrBox = (hCurr < 2 ? 2 : hCurr) + margin * 2;

  int16_t wPrev = eye.width;
  int16_t hPrev = (int16_t)(eye.height * eye.prevOpenAmount);
  int16_t xPrev = eye.prevX - wPrev / 2 - margin;
  int16_t yPrev = eye.prevY - hPrev / 2 - margin;
  int16_t wPrevBox = wPrev + margin * 2;
  int16_t hPrevBox = (hPrev < 2 ? 2 : hPrev) + margin * 2;

  // Union rectangle
  int16_t ux = xCurr < xPrev ? xCurr : xPrev;
  int16_t uy = yCurr < yPrev ? yCurr : yPrev;
  int16_t ux2 = (xCurr + wCurrBox) > (xPrev + wPrevBox) ? (xCurr + wCurrBox) : (xPrev + wPrevBox);
  int16_t uy2 = (yCurr + hCurrBox) > (yPrev + hPrevBox) ? (yCurr + hCurrBox) : (yPrev + hPrevBox);
  int16_t uw = ux2 - ux;
  int16_t uh = uy2 - uy;

  // Clamp to screen
  if (ux < 0) { uw += ux; ux = 0; }
  if (uy < 0) { uh += uy; uy = 0; }
  if (ux + uw > _screenWidth) uw = _screenWidth - ux;
  if (uy + uh > _screenHeight) uh = _screenHeight - uy;

  if (uw > 0 && uh > 0) {
    _display.fillRect(ux, uy, uw, uh, _colorBg);
  }

  // Draw the eye into the cleared area
  drawEyeShape(eye.x, eye.y, eye);

  // Update previous values for next frame
  eye.prevX = eye.x;
  eye.prevY = eye.y;
  eye.prevOpenAmount = eye.openAmount;
}

void RoboEyes::drawEyeShape(int16_t centerX, int16_t centerY, Eye &eye) {
  uint8_t w = eye.width;
  uint8_t h = eye.height * eye.openAmount;
  uint8_t r = eye.borderRadius;
  
  if (h < 2) return; // Eye closed
  
  // Apply mood angle to lid
  float upperAngle = eye.upperLidAngle;
  float lowerAngle = eye.lowerLidAngle;
  
  // Draw filled rounded rectangle
  int16_t x = centerX - w / 2;
  int16_t y = centerY - h / 2;
  
  // Adjust for lid angles
  int16_t deltaTopLeft = upperAngle * w / 2;
  int16_t deltaTopRight = -upperAngle * w / 2;
  int16_t deltaBottomLeft = lowerAngle * w / 2;
  int16_t deltaBottomRight = -lowerAngle * w / 2;
  
  // Simple filled rounded rect (approximation)
  _display.fillRoundRect(x, y, w, h, r, _colorMain);
  
  // Add lid lines for mood
  if (abs(upperAngle) > 0.1 || abs(lowerAngle) > 0.1) {
    // Top lid
    _display.drawLine(
      x, y + deltaTopLeft,
      x + w, y + deltaTopRight,
      _colorMain
    );
    
    // Bottom lid
    _display.drawLine(
      x, y + h + deltaBottomLeft,
      x + w, y + h + deltaBottomRight,
      _colorMain
    );
  }
}

void RoboEyes::calculateTargetPosition(Position pos, int16_t &leftX, int16_t &leftY, int16_t &rightX, int16_t &rightY) {
  int16_t centerX = _screenWidth / 2;
  int16_t centerY = _screenHeight / 2;
  int16_t moveRange = 15; // Pixels to move in each direction
  
  int16_t dx = 0;
  int16_t dy = 0;
  
  switch (pos) {
    case POS_N:   dx = 0;  dy = -moveRange; break;
    case POS_NE:  dx = moveRange;  dy = -moveRange; break;
    case POS_E:   dx = moveRange;  dy = 0; break;
    case POS_SE:  dx = moveRange;  dy = moveRange; break;
    case POS_S:   dx = 0;  dy = moveRange; break;
    case POS_SW:  dx = -moveRange; dy = moveRange; break;
    case POS_W:   dx = -moveRange; dy = 0; break;
    case POS_NW:  dx = -moveRange; dy = -moveRange; break;
    case POS_DEFAULT:
    default:      dx = 0;  dy = 0; break;
  }
  
  leftX = centerX - _spaceBetween / 2 - _leftEye.width / 2 + dx;
  leftY = centerY + dy;
  rightX = centerX + _spaceBetween / 2 + _rightEye.width / 2 + dx;
  rightY = centerY + dy;
  
  // Apply curiosity (widen eyes when looking far left/right)
  if (_curiosity && (pos == POS_E || pos == POS_W)) {
    _leftEye.height = 60;
    _rightEye.height = 60;
  }
}

float RoboEyes::easeInOutCubic(float t) {
  return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
}
