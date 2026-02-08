#include <TM1637Display.h>
#include <Adafruit_NeoPixel.h>

// ----------- TM1637 Display ----------
#define TM_CLK 9
#define TM_DIO 10
TM1637Display display(TM_CLK, TM_DIO);

// ----------- NeoPixel Setup ----------
#define NEOPIXEL_PIN 6
#define NUM_PIXELS 3
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ----------- Sounds ----------
#define PIEZO_PIN 8

// ----------- Game Inputs ----------
#define GAME_WAND_PIN 2
#define WIN_BUTTON_PIN 3
#define START_BUTTON_PIN 4

// ----------- Constants ----------
const int INITIAL_LIVES = 3;
const unsigned long DEBOUNCE_DELAY = 500;

// ----------- Mario Theme ----------
const int MARIO_THEME_FREQ[] = {
  659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392
};
const int MARIO_THEME_DUR[] = {
  150, 150, 110, 150, 110, 150, 150, 110, 300, 200, 300
};
const int MARIO_THEME_LENGTH = sizeof(MARIO_THEME_FREQ) / sizeof(MARIO_THEME_FREQ[0]);

// ----------- Sounds ----------
const int LIFE_LOST_FREQ[] = {659, 523, 330};
const int GAME_OVER_FREQ[] = {523, 494, 466, 440, 415, 392};
const int GAME_OVER_DURATIONS[] = {300, 300, 300, 300, 400, 800};
const int GAME_START_FREQ[] = {659, 659, 659, 523, 784, 698, 659};
const int GAME_START_DURATIONS[] = {150, 150, 150, 200, 150, 150, 300};

const int LIFE_LOST_TONE_DURATION = 200;
const int LIFE_LOST_LED_ON_DELAY = 120;
const int LIFE_LOST_LED_OFF_DELAY = 80;

// ----------- Colors ----------
uint32_t COLOR_HEART   = 0xFF0000;
uint32_t COLOR_OFF     = 0x000000;
uint32_t COLOR_YELLOW  = 0xFFFF00;

// ----------- Game State ----------
enum GameState {
  WAITING_TO_START,
  STARTING,
  PLAYING,
  LIFE_LOST,
  GAME_OVER,
  WIN
};
GameState gameState = WAITING_TO_START;

int lives = INITIAL_LIVES;
unsigned long gameStartTime = 0;
unsigned long gameEndTime = 0;

// ----------- Input State ----------
bool lastWandState = HIGH;
unsigned long lastWandTouchTime = 0;

// ----------- Animation/Sound State ----------
unsigned long animationStartTime = 0;
int animationStep = 0;


void setup() {
  Serial.begin(9600);

  pixels.begin();
  pixels.show();
  updateLivesDisplay();

  pinMode(GAME_WAND_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(WIN_BUTTON_PIN, INPUT_PULLUP);

  pinMode(PIEZO_PIN, OUTPUT);

  display.setBrightness(0x0f);
  display.clear();

  tone(PIEZO_PIN, 800, 40);
  delay(60);
  noTone(PIEZO_PIN);
  
  Serial.println("Ready to start!");
}

void loop() {
  unsigned long currentTime = millis();

  switch (gameState) {
    case WAITING_TO_START:
      handleWaitingToStart(currentTime);
      break;
    case STARTING:
      handleStarting(currentTime);
      break;
    case PLAYING:
      handlePlaying(currentTime);
      break;
    case LIFE_LOST:
      handleLifeLost(currentTime);
      break;
    case GAME_OVER:
      handleGameOver(currentTime);
      break;
    case WIN:
      handleWin(currentTime);
      break;
  }
}

// =================================================================
// State Handlers
// =================================================================

void handleWaitingToStart(unsigned long currentTime) {
  display.showNumberDec(0, true, 4, 0);
  if (digitalRead(START_BUTTON_PIN) == LOW) {
    Serial.println("Game Starting!");
    gameState = STARTING;
    animationStep = 0;
    animationStartTime = currentTime;
    resetGame();
  }
}

void handleStarting(unsigned long currentTime) {
  // This state plays the Mario theme at the beginning of the game.
  bool finished = playMarioTheme(currentTime);
  if (finished) {
    Serial.println("Game Started!");
    gameState = PLAYING;
    gameStartTime = currentTime;
  }
}

void handlePlaying(unsigned long currentTime) {
  // Update clock display
  int elapsed = (currentTime - gameStartTime) / 1000;
  int minutes = elapsed / 60;
  int seconds = elapsed % 60;
  int time_display = minutes * 100 + seconds;
  display.showNumberDecEx(time_display, 0b01000000, true, 4, 0);

  // Check for wand touch
  if (checkWandTouch(currentTime)) {
    lives--;
    Serial.print("Life lost! Lives remaining: ");
    Serial.println(lives);
    
    gameState = LIFE_LOST;
    animationStep = 0;
    animationStartTime = currentTime;
    
    if (lives == 0) {
      gameEndTime = lastWandTouchTime;
    }
  }

  // Check for win
  if (digitalRead(WIN_BUTTON_PIN) == LOW) {
    gameEndTime = currentTime;
    Serial.println("You Win!");
    gameState = WIN;
    animationStep = 0;
    animationStartTime = currentTime;
  }
}

void handleLifeLost(unsigned long currentTime) {
  bool finished = playLoseLifeAnimation(currentTime);
  if (finished) {
    updateLivesDisplay();
    if (lives > 0) {
      gameState = PLAYING;
    } else {
      Serial.println("Game Over!");
      gameState = GAME_OVER;
      animationStep = 0;
      animationStartTime = currentTime;
      showFinalTime();
    }
  }
}

void handleGameOver(unsigned long currentTime) {
  bool finished = playGameOverAnimation(currentTime);
  if (finished) {
    gameState = WAITING_TO_START;
  }
}

void handleWin(unsigned long currentTime) {
  bool finished = playMarioTheme(currentTime); // Play Mario on win
  
  // Also do a rainbow pixel effect
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.ColorHSV((currentTime / 10 + i * 256 / NUM_PIXELS) % 65535));
  }
  pixels.show();

  // After 3 seconds, go back to waiting
  if (currentTime - animationStartTime > 3000) {
    showFinalTime();
    gameState = WAITING_TO_START;
  }
}


// =================================================================
// Game Logic & Animations (Non-Blocking)
// =================================================================

void resetGame() {
  lives = INITIAL_LIVES;
  updateLivesDisplay();
}

void showFinalTime() {
  unsigned long finalTime = (gameEndTime - gameStartTime) / 1000;
  Serial.print("Final Time: ");
  Serial.println(finalTime);
  int minutes = finalTime / 60;
  int seconds = finalTime % 60;
  int time_display = minutes * 100 + seconds;
  display.showNumberDecEx(time_display, 0b01000000, true, 4, 0);
}

void updateLivesDisplay() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    if (i < lives)
      pixels.setPixelColor(i, COLOR_HEART);
    else
      pixels.setPixelColor(i, COLOR_OFF);
  }
  pixels.show();
}

bool checkWandTouch(unsigned long currentTime) {
  bool wandTouched = (digitalRead(GAME_WAND_PIN) == LOW);
  if (wandTouched && !lastWandState && (currentTime - lastWandTouchTime > DEBOUNCE_DELAY)) {
    lastWandTouchTime = currentTime;
    lastWandState = true;
    return true;
  }
  if (!wandTouched) {
    lastWandState = false;
  }
  return false;
}

bool playMarioTheme(unsigned long currentTime) {
  if (animationStep >= MARIO_THEME_LENGTH) {
    noTone(PIEZO_PIN);
    return true; // Finished
  }

  // Animate pixels
  for (int j = 0; j < NUM_PIXELS; j++) {
    uint32_t color = pixels.Color(
      (uint8_t)(sin((animationStep + j) * 2.0) * 127 + 128),
      (uint8_t)(sin((animationStep + j) * 2.0 + 2) * 127 + 128),
      (uint8_t)(sin((animationStep + j) * 2.0 + 4) * 127 + 128)
    );
    pixels.setPixelColor(j, color);
  }
  pixels.show();

  if (currentTime - animationStartTime >= MARIO_THEME_DUR[animationStep]) {
    noTone(PIEZO_PIN);
    animationStep++;
    animationStartTime = currentTime;
    if (animationStep < MARIO_THEME_LENGTH && MARIO_THEME_FREQ[animationStep] > 0) {
      tone(PIEZO_PIN, MARIO_THEME_FREQ[animationStep]);
    }
  }
  return false; // Not finished
}

bool playLoseLifeAnimation(unsigned long currentTime) {
  int ledIndex = lives; // `lives` is already decremented, so it's the index of the light to flash
  
  // This animation has 3 tones and 8 "tinkles"
  const int totalSteps = 3 + 8;
  if (animationStep >= totalSteps) {
    noTone(PIEZO_PIN);
    return true; // Finished
  }

  unsigned long timeInAnimation = currentTime - animationStartTime;

  if (animationStep < 3) { // The first 3 tones
    unsigned long stepDuration = LIFE_LOST_TONE_DURATION + LIFE_LOST_LED_ON_DELAY + LIFE_LOST_LED_OFF_DELAY;
    if (timeInAnimation < LIFE_LOST_TONE_DURATION) {
      if (animationStep == 0 && timeInAnimation == 0) tone(PIEZO_PIN, LIFE_LOST_FREQ[animationStep]);
    } else if (timeInAnimation < stepDuration - LIFE_LOST_LED_OFF_DELAY) {
      pixels.setPixelColor(ledIndex, COLOR_OFF);
      pixels.show();
    } else {
      pixels.setPixelColor(ledIndex, COLOR_HEART);
      pixels.show();
    }
    
    if (timeInAnimation >= stepDuration) {
      noTone(PIEZO_PIN);
      animationStep++;
      animationStartTime = currentTime;
      if (animationStep < 3) tone(PIEZO_PIN, LIFE_LOST_FREQ[animationStep]);
    }
  } else { // The 8 tinkles
    int tinkleStep = animationStep - 3;
    int tinkle[] = {1047, 1319};
    unsigned long stepDuration = 150 + 50 + 30;

    if (timeInAnimation < 150) {
       if (tinkleStep == 0 && timeInAnimation == 0) tone(PIEZO_PIN, tinkle[tinkleStep % 2]);
    } else if (timeInAnimation < 150 + 50) {
      pixels.setPixelColor(ledIndex, 0x4400FF);
      pixels.show();
    } else {
      pixels.setPixelColor(ledIndex, COLOR_OFF);
      pixels.show();
    }

    if (timeInAnimation >= stepDuration) {
      noTone(PIEZO_PIN);
      animationStep++;
      animationStartTime = currentTime;
      if (animationStep < totalSteps) tone(PIEZO_PIN, tinkle[(animationStep - 3) % 2]);
    }
  }
  
  return false; // Not finished
}

bool playGameOverAnimation(unsigned long currentTime) {
  const int totalSteps = 6;
  if (animationStep >= totalSteps) {
    noTone(PIEZO_PIN);
    return true; // Finished
  }

  unsigned long stepDuration = GAME_OVER_DURATIONS[animationStep];
  unsigned long timeInAnimation = currentTime - animationStartTime;

  if (timeInAnimation < stepDuration / 2) {
    // Set pixel colors
    for (int j = 0; j < NUM_PIXELS; j++) {
      if ((animationStep + j) % 2 == 0) pixels.setPixelColor(j, COLOR_HEART);
      else pixels.setPixelColor(j, COLOR_YELLOW);
    }
    pixels.show();
    // Start tone only at the beginning of the step
    if (timeInAnimation == 0) {
       tone(PIEZO_PIN, GAME_OVER_FREQ[animationStep]);
    }
  } else {
    // Turn pixels off
    for (int j = 0; j < NUM_PIXELS; j++) {
      pixels.setPixelColor(j, COLOR_OFF);
    }
    pixels.show();
  }

  if (timeInAnimation >= stepDuration) {
    noTone(PIEZO_PIN);
    animationStep++;
    animationStartTime = currentTime;
  }

  return false; // Not finished
}