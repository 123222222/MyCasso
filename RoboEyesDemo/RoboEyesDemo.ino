/***************************************************
 * RoboEyesDemo.ino - Demo for RoboEyes Library
 * ST7789 TFT Display 240x320
 ***************************************************/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "FluxGarage_RoboEyes.h"

// ST7789 Pin definitions
#define TFT_CS    5
#define TFT_DC    16
#define TFT_RST   17
#define TFT_SDA   23  // MOSI
#define TFT_SCK   18  // SCK

// Create display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Create RoboEyes object (FluxGarage port for ST7789)
RoboEyes<Adafruit_ST7789> eyes(tft);

void setup() {
  Serial.begin(115200);
  Serial.println("\n🤖 RoboEyes ST7789 Demo");
  
  // Initialize display
  tft.init(240, 320);
  tft.setRotation(2); // Portrait mode (240x320) - vertical
  tft.fillScreen(ST77XX_BLACK);
  
  // Initialize RoboEyes (width,height,maxFPS)
  // Note: for rotation=0 the logical width/height are 240x320
  eyes.begin(240, 320, 15);

  // Configure eyes (FluxGarage API names)
  eyes.setWidth(40, 40);
  eyes.setHeight(50, 50);
  eyes.setBorderradius(15, 15);
  eyes.setSpacebetween(40);

  // Set colors (background, main)
  eyes.setDisplayColors(ST77XX_BLACK, ST77XX_CYAN);

  // Enable auto behaviors (use FluxGarage API names)
  // Use slower defaults: auto-blink ~12s ±3s and idle every 12s (no variation)
  eyes.setAutoblinker(ON, 12, 3); // active, interval (s), variation (s)
  eyes.setIdleMode(ON, 12, 0); // idle every 12s exactly
  
  Serial.println("✅ Initialized!");
  printHelp();
}

void loop() {
  // Update eyes (handles auto blink and idle)
  eyes.update();
  
  // Check serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }
}

void handleCommand(char cmd) {
  switch (cmd) {
    // === MOODS ===
    // Use letter commands for moods to avoid colliding with numeric position keys
    case 'h':
      eyes.setMood(HAPPY);
      Serial.println("😊 HAPPY - Vui vẻ");
      break;

    case 's':
      eyes.setMood(DEFAULT); // FluxGarage has no SAD constant separate from DEFAULT; use DEFAULT or implement if needed
      Serial.println("😢 SAD - Buồn");
      break;

    case 'a':
      eyes.setMood(ANGRY);
      Serial.println("😠 ANGRY - Giận dữ");
      break;

    case 't':
      eyes.setMood(TIRED);
      Serial.println("😴 TIRED - Mệt mỏi");
      break;

    case 'n':
      eyes.setMood(DEFAULT);
      Serial.println("😐 DEFAULT - Bình thường");
      break;
    
    // === POSITIONS ===
    case '8':
      eyes.setPosition(N);
      Serial.println("⬆️ NORTH - Nhìn lên");
      break;
      
    case '9':
      eyes.setPosition(NE);
      Serial.println("↗️ NORTH-EAST");
      break;
      
    case '6':
      eyes.setPosition(E);
      Serial.println("➡️ EAST - Nhìn phải");
      break;
      
    case '3':
      eyes.setPosition(SE);
      Serial.println("↘️ SOUTH-EAST");
      break;
      
    case '2':
      eyes.setPosition(S);
      Serial.println("⬇️ SOUTH - Nhìn xuống");
      break;
      
    case '1':
      eyes.setPosition(SW);
      Serial.println("↙️ SOUTH-WEST");
      break;
      
    case '4':
      eyes.setPosition(W);
      Serial.println("⬅️ WEST - Nhìn trái");
      break;
      
    case '7':
      eyes.setPosition(NW);
      Serial.println("↖️ NORTH-WEST");
      break;
      
    case '5':
    case 'c':
      eyes.setPosition(DEFAULT);
      Serial.println("⏺️ CENTER - Giữa");
      break;
    
    // === ACTIONS ===
    case 'b':
      eyes.blink();
      Serial.println("👁️ BLINK - Nháy mắt");
      break;
      
    case 'o':
      eyes.open();
      Serial.println("👀 OPEN - Mở mắt");
      break;
      
    case 'x':
      eyes.close();
      Serial.println("😑 CLOSE - Nhắm mắt");
      break;
    
    // === ANIMATIONS ===
    case 'l':
      Serial.println("😂 LAUGH - Cười");
      eyes.anim_laugh();
      break;
      
    case 'f':
      Serial.println("😕 CONFUSED - Bối rối");
      eyes.anim_confused();
      break;
    
    // === FEATURES ===
    case 'w':
      eyes.setSweat(true);
      Serial.println("💦 SWEAT ON - Bật mồ hôi");
      delay(2000);
      eyes.setSweat(false);
      break;
      
    case 'u':
      eyes.setCuriosity(!eyes.curious);
      Serial.print("👁️ CURIOSITY: ");
      Serial.println(eyes.curious ? "ON" : "OFF");
      break;
      
    case 'y':
      eyes.setCyclops(!eyes.cyclops);
      Serial.print("👁️ CYCLOPS: ");
      Serial.println(eyes.cyclops ? "ON" : "OFF");
      break;
    
    // === AUTO MODES ===
    case 'A':
      // Keep the same slower defaults when toggling from serial (FluxGarage API)
  eyes.setAutoblinker(!eyes.autoblinker, 12, 3);
      Serial.print("🔄 AUTO BLINK: ");
      Serial.println(eyes.autoblinker ? "ON" : "OFF");
      break;
      
    case 'I':
      // Toggle idle mode and keep the default interval at 5s with no variation
  eyes.setIdleMode(!eyes.idle, 12, 0);
      Serial.print("🔄 IDLE MODE: ");
      Serial.println(eyes.idle ? "ON" : "OFF");
      break;
    
    // === HELP ===
    case '?':
      printHelp();
      break;
      
    default:
      if (cmd > 32) { // Printable character
        Serial.print("❓ Unknown command: ");
        Serial.println(cmd);
        Serial.println("Type '?' for help");
      }
      break;
  }
}

void printHelp() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║    🤖 ROBOEYES CONTROL COMMANDS       ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ MOODS:                                 ║");
  Serial.println("║  h    = 😊 Happy (Vui)                 ║");
  Serial.println("║  s    = 😢 Sad (Buồn)                  ║");
  Serial.println("║  a    = 😠 Angry (Giận)                ║");
  Serial.println("║  t    = 😴 Tired (Mệt)                 ║");
  Serial.println("║  n    = 😐 Default (Bình thường)       ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ POSITIONS (Numpad layout):             ║");
  Serial.println("║  7(NW)  8(N)  9(NE)                    ║");
  Serial.println("║  4(W)   5(C)  6(E)                     ║");
  Serial.println("║  1(SW)  2(S)  3(SE)                    ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ ACTIONS:                               ║");
  Serial.println("║  b = 👁️  Blink (Nháy mắt)            ║");
  Serial.println("║  o = 👀 Open (Mở)                     ║");
  Serial.println("║  x = 😑 Close (Nhắm)                  ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ ANIMATIONS:                            ║");
  Serial.println("║  l = 😂 Laugh (Cười)                  ║");
  Serial.println("║  f = 😕 Confused (Bối rối)            ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ FEATURES:                              ║");
  Serial.println("║  w = 💦 Sweat (Mồ hôi)                ║");
  Serial.println("║  u = 👁️  Toggle Curiosity             ║");
  Serial.println("║  y = 👁️  Toggle Cyclops               ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ AUTO MODES:                            ║");
  Serial.println("║  A = 🔄 Toggle Auto Blink              ║");
  Serial.println("║  I = 🔄 Toggle Idle Mode               ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║  ? = Show this help                    ║");
  Serial.println("╚════════════════════════════════════════╝\n");
}
