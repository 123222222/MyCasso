/*
  Simple RoboEyes Example for ST7789 with Adafruit Library
  Không cần thư viện RoboEyes, code trực tiếp!
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// ST7789 Pin definitions for ESP32
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  17
#define TFT_MOSI 23  // SDA
#define TFT_SCLK 18  // SCK

// Create display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Eye parameters
int screenWidth = 240;
int screenHeight = 240;

// Eye position and size
int eyeWidth = 40;
int eyeHeight = 50;
int eyeRadius = 15;
int spaceBetween = 20;

// Eye state
float leftEyeOpen = 1.0;   // 0.0 = closed, 1.0 = open
float rightEyeOpen = 1.0;
int eyePosX = 0;  // -15 to 15
int eyePosY = 0;  // -15 to 15

// Auto blink
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkDelay = 4000;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing ST7789...");
  
  // Initialize display - thử các cấu hình khác nhau
  tft.init(240, 320);  // Bỏ SPI_MODE2
  
  // Thử các rotation khác nhau nếu không hiển thị
  tft.setRotation(1);  // Thử 0, 1, 2, 3
  
//   // Test màn hình
//   Serial.println("Testing screen colors...");
//   tft.fillScreen(ST77XX_RED);
//   delay(500);
//   tft.fillScreen(ST77XX_GREEN);
//   delay(500);
//   tft.fillScreen(ST77XX_BLUE);
//   delay(500);
//   tft.fillScreen(ST77XX_BLACK);
  
//   Serial.println("RoboEyes Simple Demo - Screen OK!");
}


void loop() {
  // Demo sequence - uncomment để xem các animation
  demoSequence();
  
  
  // Hoặc dùng auto mode đơn giản:
/*
  if (millis() - lastBlinkTime > nextBlinkDelay) {
    blink();
    lastBlinkTime = millis();
    nextBlinkDelay = random(3000, 6000);
  }
  drawEyes();
  delay(50);
 */ 
}

void demoSequence() {
  // 1. Normal blinking
  lookCenter();
  for(int i=0; i<30; i++) {
    if (random(100) < 5) blink();
    drawEyes();
    delay(100);
  }
  
  // 2. Look around
  lookLeft();
  for(int i=0; i<20; i++) { drawEyes(); delay(50); }
  
  lookRight();
  for(int i=0; i<20; i++) { drawEyes(); delay(50); }
  
  lookUp();
  for(int i=0; i<20; i++) { drawEyes(); delay(50); }
  
  lookDown();
  for(int i=0; i<20; i++) { drawEyes(); delay(50); }
  
  lookCenter();
  
  // 3. Happy eyes
  animHappy();
  delay(1000);
  
  // 4. Angry eyes
  animAngry();
  delay(1000);
  
  // 5. Sleepy
  animSleepy();
  delay(1000);
  
  // 6. Surprised
  animSurprised();
  delay(1000);
  
  // 7. Confused (shake)
  animConfused();
  delay(1000);
  
  // 8. Wink
  winkLeft();
  delay(500);
  winkRight();
  delay(500);
}

void drawEyes() {
  tft.fillScreen(ST77XX_BLACK);
  
  // Calculate eye positions
  int leftX = screenWidth/2 - spaceBetween/2 - eyeWidth/2 + eyePosX;
  int rightX = screenWidth/2 + spaceBetween/2 + eyeWidth/2 + eyePosX;
  int centerY = screenHeight/2 + eyePosY;
  
  // Draw left eye
  drawEye(leftX, centerY, eyeWidth, eyeHeight, eyeRadius, leftEyeOpen);
  
  // Draw right eye
  drawEye(rightX, centerY, eyeWidth, eyeHeight, eyeRadius, rightEyeOpen);
}

void drawEye(int centerX, int centerY, int w, int h, int r, float openness) {
  int effectiveHeight = h * openness;
  
  if (effectiveHeight < 2) return; // Eye closed
  
  int x = centerX - w/2;
  int y = centerY - effectiveHeight/2;
  
  // Draw eye outline
  tft.fillRoundRect(x, y, w, effectiveHeight, r, ST77XX_WHITE);
  
  // Draw pupil if eye is open enough
  if (openness > 0.3) {
    int pupilSize = min(w, effectiveHeight) / 3;
    tft.fillCircle(centerX, centerY, pupilSize, ST77XX_BLACK);
  }
}

void blink() {
  // Close eyes
  for (int i = 10; i >= 0; i--) {
    leftEyeOpen = i / 10.0;
    rightEyeOpen = i / 10.0;
    drawEyes();
    delay(20);
  }
  
  delay(100);
  
  // Open eyes
  for (int i = 0; i <= 10; i++) {
    leftEyeOpen = i / 10.0;
    rightEyeOpen = i / 10.0;
    drawEyes();
    delay(20);
  }
}

// Example functions to control eyes:
void lookLeft() {
  eyePosX = -15;
}

void lookRight() {
  eyePosX = 15;
}

void lookUp() {
  eyePosY = -15;
}

void lookDown() {
  eyePosY = 15;
}

void lookCenter() {
  eyePosX = 0;
  eyePosY = 0;
}

// ========== ANIMATIONS ==========

void animHappy() {
  // Wide eyes with curved bottom
  eyeHeight = 60;
  eyeRadius = 20;
  for(int i=0; i<30; i++) {
    drawEyes();
    delay(50);
  }
  eyeHeight = 50;
  eyeRadius = 15;
}

void animAngry() {
  // Narrow sharp eyes
  eyeHeight = 30;
  eyeRadius = 5;
  eyePosY = -10;
  for(int i=0; i<30; i++) {
    drawEyes();
    delay(50);
  }
  eyeHeight = 50;
  eyeRadius = 15;
  eyePosY = 0;
}

void animSleepy() {
  // Half-closed eyes
  leftEyeOpen = 0.4;
  rightEyeOpen = 0.4;
  lookDown();
  for(int i=0; i<30; i++) {
    drawEyes();
    delay(100);
  }
  leftEyeOpen = 1.0;
  rightEyeOpen = 1.0;
  lookCenter();
}

void animSurprised() {
  // Very wide eyes
  eyeWidth = 50;
  eyeHeight = 70;
  spaceBetween = 30;
  for(int i=0; i<30; i++) {
    drawEyes();
    delay(50);
  }
  eyeWidth = 40;
  eyeHeight = 50;
  spaceBetween = 20;
}

void animConfused() {
  // Shake left and right
  for(int i=0; i<5; i++) {
    lookLeft();
    drawEyes();
    delay(100);
    lookRight();
    drawEyes();
    delay(100);
  }
  lookCenter();
}

void winkLeft() {
  // Close left eye only
  for (int i = 10; i >= 0; i--) {
    leftEyeOpen = i / 10.0;
    drawEyes();
    delay(20);
  }
  delay(200);
  for (int i = 0; i <= 10; i++) {
    leftEyeOpen = i / 10.0;
    drawEyes();
    delay(20);
  }
}

void winkRight() {
  // Close right eye only
  for (int i = 10; i >= 0; i--) {
    rightEyeOpen = i / 10.0;
    drawEyes();
    delay(20);
  }
  delay(200);
  for (int i = 0; i <= 10; i++) {
    rightEyeOpen = i / 10.0;
    drawEyes();
    delay(20);
  }
}
