#include <TM1637Display.h>
#include <Adafruit_NeoPixel.h>

// ----------- TM1637 Display -----------
#define TM_CLK 9
#define TM_DIO 10
TM1637Display display(TM_CLK, TM_DIO);

// ----------- NeoPixel Setup -----------
#define NEOPIXEL_PIN 6
#define NUM_PIXELS 3
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ----------- Sounds -----------
#define PIEZO_PIN 8

// ----------- Game Inputs -----------
#define GAME_WAND_PIN 2
#define WIN_BUTTON_PIN 3
#define START_BUTTON_PIN 4

// ----------- Constants -----------
const int INITIAL_LIVES = 3;
const unsigned long DEBOUNCE_DELAY = 1000;

// ----------- Mario Theme -----------
const int MARIO_THEME_FREQ[] = {
  659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392
};
const int MARIO_THEME_DUR[] = {
  150, 150, 110, 150, 110, 150, 150, 110, 300, 200, 300
};

// ----------- Sounds -----------
const int LIFE_LOST_FREQ[] = {659, 523, 330};

const int GAME_OVER_FREQ[] = {523, 494, 466, 440, 415, 392};
const int GAME_OVER_DURATIONS[] = {300, 300, 300, 300, 400, 800};

const int GAME_START_FREQ[] = {659, 784, 1319, 1047, 1175, 1568, 1047, 1175, 1568};
const int GAME_START_DURATIONS[] = {100, 100, 100, 100, 100, 80, 50, 50,50};

const int LIFE_LOST_TONE_DURATION = 200;
const int LIFE_LOST_LED_ON_DELAY = 120;
const int LIFE_LOST_LED_OFF_DELAY = 80;

// ----------- Colors -----------
uint32_t COLOR_HEART   = 0xFF0000;
uint32_t COLOR_OFF     = 0x000000;
uint32_t COLOR_YELLOW  = 0xFFFF00;

// ----------- Game State -----------
int lives = INITIAL_LIVES;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

bool gameRunning = false;
bool gameOverState = false;
unsigned long gameStartTime = 0;
unsigned long gameEndTime = 0;

void setup() {
  Serial.begin(9600);

  pixels.begin();
  pixels.show();
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, COLOR_HEART);
  }
  pixels.show();

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
  if (!gameRunning) {
    if (gameOverState) {
      // Don't clear the display, just wait for the start sensor
      if (digitalRead(START_BUTTON_PIN) == LOW) {
        gameRunning = true;
        gameStartTime = millis();
        resetGame();
        Serial.println("Game Started!");
        gameOverState = false;
      }
    } else {
      display.showNumberDec(0, true, 4, 0);
      if (digitalRead(START_BUTTON_PIN) == LOW) {
        gameRunning = true;
        gameStartTime = millis();
        resetGame();
        Serial.println("Game Started!");
      }
    }
    return;
  }

  unsigned long now = millis();
  int elapsed = (now - gameStartTime) / 1000;

  // Update lives display
  for (int i = 0; i < NUM_PIXELS; i++) {
    if (i < lives)
      pixels.setPixelColor(i, COLOR_HEART);
    else
      pixels.setPixelColor(i, COLOR_OFF);
  }
  pixels.show();

  // Show elapsed time in seconds
  display.showNumberDecEx(elapsed, 0b01000000, true, 4, 0);

  // Lose life if wand touches buzz wire
  if (gameRunning && checkWandTouch()) {
    if (lives > 0) {
      loseLife();
      lives--;
      Serial.print("Life lost! Lives remaining: ");
      Serial.println(lives);

      if (lives == 0) {
        delay(800);
        gameEndTime = millis();
        showFinalTime();
        gameOver();
        
        gameRunning = false;
        gameOverState = true;
      }
    }
  }

  // Win sensor
  if (gameRunning && digitalRead(WIN_BUTTON_PIN) == LOW) {
    gameEndTime = millis();
    playMario();
    
    for (int i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, COLOR_YELLOW);
    }
    pixels.show();
    delay(3000);
  
    gameRunning = false;
    gameOverState = true;
  }
}

void showFinalTime() {
  unsigned long finalTime = (gameEndTime - gameStartTime) / 1000;
  Serial.print("Final Time: ");
  Serial.println(finalTime);
  display.showNumberDecEx(finalTime, 0b01000000, true, 4, 0);
}

void playMario() {
  Serial.println("Playing Mario theme...");
  int nNotes = sizeof(MARIO_THEME_FREQ) / sizeof(MARIO_THEME_FREQ[0]);

  for (int i = 0; i < nNotes; i++) {
    for (int j = 0; j < NUM_PIXELS; j++) {
      uint32_t color = pixels.Color(
        (uint8_t)(sin((i + j) * 2.0) * 127 + 128),
        (uint8_t)(sin((i + j) * 2.0 + 2) * 127 + 128),
        (uint8_t)(sin((i + j) * 2.0 + 4) * 127 + 128)
      );
      pixels.setPixelColor(j, color);
    }
    pixels.show();

    if (MARIO_THEME_FREQ[i] > 0)
      tone(PIEZO_PIN, MARIO_THEME_FREQ[i], MARIO_THEME_DUR[i]);
    delay(MARIO_THEME_DUR[i]);
    noTone(PIEZO_PIN);
  }
}

bool checkWandTouch() {
  bool currentButtonState = digitalRead(GAME_WAND_PIN);
  unsigned long currentTime = millis();

  if (currentButtonState == LOW && lastButtonState == HIGH &&
      (currentTime - lastDebounceTime > DEBOUNCE_DELAY)) {
    lastDebounceTime = currentTime;
    lastButtonState = currentButtonState;
    return true;
  }

  if (currentButtonState != lastButtonState)
    lastButtonState = currentButtonState;

  return false;
}

void loseLife() {
  int ledIndex = lives - 1;
  for (int i = 0; i < NUM_PIXELS; i++) {
    tone(PIEZO_PIN, LIFE_LOST_FREQ[i], LIFE_LOST_TONE_DURATION);
    pixels.setPixelColor(ledIndex, COLOR_OFF);
    pixels.show();
    delay(LIFE_LOST_LED_ON_DELAY);
    pixels.setPixelColor(ledIndex, COLOR_HEART);
    pixels.show();
    delay(LIFE_LOST_LED_OFF_DELAY);
  }

  int tinkle[] = {1047, 1319};
  for (int i = 0; i < 8; i++) { 
      tone(PIEZO_PIN, tinkle[i % 2], 150);
      pixels.setPixelColor(ledIndex, 0x4400FF);
      pixels.show();
      delay(50);
      pixels.setPixelColor(ledIndex, COLOR_OFF);
      pixels.show();
      delay(30);
  }

  pixels.setPixelColor(ledIndex, COLOR_OFF);
  pixels.show();
  noTone(PIEZO_PIN);
}

void gameOver() {
  Serial.println("Game Over!");
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < NUM_PIXELS; j++) {
      if ((i + j) % 2 == 0) pixels.setPixelColor(j, COLOR_HEART);
      else pixels.setPixelColor(j, COLOR_YELLOW);
    }
    pixels.show();
    tone(PIEZO_PIN, GAME_OVER_FREQ[i], GAME_OVER_DURATIONS[i]);
    delay(GAME_OVER_DURATIONS[i] / 2);

    for (int j = 0; j < NUM_PIXELS; j++) {
      pixels.setPixelColor(j, COLOR_OFF);
    }
    pixels.show();
    delay(GAME_OVER_DURATIONS[i] / 2 + 50);
  }
  noTone(PIEZO_PIN);
}

void resetGame() {
  Serial.println("Game Reset!");
  lives = INITIAL_LIVES;

  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, COLOR_HEART);
  }
  pixels.show();

  int nNotes = sizeof(GAME_START_FREQ) / sizeof(GAME_START_FREQ[0]);
  for (int i = 0; i < nNotes; i++) {
    tone(PIEZO_PIN, GAME_START_FREQ[i], GAME_START_DURATIONS[i]);
    delay(GAME_START_DURATIONS[i] + 50);
  }

  noTone(PIEZO_PIN);
  pixels.show();
  
  Serial.println("Ready to play again!");
}
