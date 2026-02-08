#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ball properties
float ballX, ballY;
float velX, velY;
const int ballRadius = 4;

void setup() {
  Serial.begin(115200);

  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Initialize ball position and velocity
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  velX = 2.5; // Initial horizontal velocity
  velY = 1.5; // Initial vertical velocity
  
  display.clearDisplay();
}

void loop() {
  // 1. Update State
  // Move the ball
  ballX += velX;
  ballY += velY;

  // Check for collisions with the screen edges and reverse velocity if a collision occurs.
  // Left or right edge
  if (ballX - ballRadius <= 0 || ballX + ballRadius >= SCREEN_WIDTH) {
    velX = -velX;
  }
  // Top or bottom edge
  if (ballY - ballRadius <= 0 || ballY + ballRadius >= SCREEN_HEIGHT) {
    velY = -velY;
  }

  // 2. Draw Frame
  // Clear the buffer
  display.clearDisplay();
  
  // Draw the ball at its new position
  display.fillCircle(ballX, ballY, ballRadius, SSD1306_WHITE);
  
  // 3. Display Frame
  // Push the frame to the screen
  display.display();

  // 4. Delay
  // Wait for a short period to control the animation speed
  delay(10); 
}
