
// --- Configurable Constants ---
const int LED_PINS[] = {9, 10, 11}; // Anode (220 res to ground)
const int NUM_LEDS = 3;
const int BUTTON_PIN = 2;
const int PIEZO_PIN = 8;

const int INITIAL_LIVES = 3;
const unsigned long DEBOUNCE_DELAY = 250;

const int LIFE_LOST_FREQ[] = {659, 523, 330};
const int GAME_OVER_FREQ[] = {523, 494, 466, 440, 415, 392};
const int GAME_OVER_DURATIONS[] = {300, 300, 300, 300, 400, 800};
const int GAME_START_FREQ[] = {659, 659, 659, 523, 784, 698, 659};
const int GAME_START_DURATIONS[] = {150, 150, 150, 200, 150, 150, 300};
const int POWER_UP_FREQ[] = {1047, 1175, 1319, 1397, 1568, 1760, 1976};

const int LIFE_LOST_TONE_DURATION = 200;
const int LIFE_LOST_LED_ON_DELAY = 120;
const int LIFE_LOST_LED_OFF_DELAY = 80;

const int GAME_OVER_WAIT_MS = 5000;
const int GAME_START_LED_COUNT = 3;

const int POWER_UP_TONE_DURATION = 80;
const int POWER_UP_TONE_DELAY = 90;

// --- State Variables ---
int lives = INITIAL_LIVES;
bool lastButtonState = HIGH;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0; 

void setup() {
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], HIGH); // Start with all LEDs on
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIEZO_PIN, OUTPUT);
  playGameStartSound();
  Serial.begin(9600);
  Serial.println("Game Started!");
}

void loop() {
  if (checkButtonPress()) {
    if (lives > 0) {
      loseLife();
      lives--;
      Serial.print("Life lost! Lives remaining: ");
      Serial.println(lives);
      
      if (lives == 0) {
        delay(800);
        gameOver();
      }
    }
  }
}

bool checkButtonPress() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    lastButtonState = currentButtonState;
    delay(DEBOUNCE_DELAY);
    return true;
  }
  
  lastButtonState = currentButtonState;
  return false;
}

void loseLife() {
  int ledIndex = NUM_LEDS - lives;
  for (int i = 0; i < NUM_LEDS; i++) {
    tone(PIEZO_PIN, LIFE_LOST_FREQ[i], LIFE_LOST_TONE_DURATION);
    digitalWrite(LED_PINS[ledIndex], LOW);
    delay(LIFE_LOST_LED_ON_DELAY);
    digitalWrite(LED_PINS[ledIndex], HIGH);
    delay(LIFE_LOST_LED_OFF_DELAY);
  }
  digitalWrite(LED_PINS[ledIndex], LOW);
  noTone(PIEZO_PIN);
}

void gameOver() {
  Serial.println("Game Over!");
  for (int i = 0; i < 6; i++) {
    tone(PIEZO_PIN, GAME_OVER_FREQ[i], GAME_OVER_DURATIONS[i]);
    delay(GAME_OVER_DURATIONS[i] + 50);
  }
  noTone(PIEZO_PIN);
  delay(GAME_OVER_WAIT_MS);
  resetGame();
}

void resetGame() {
  Serial.println("Game Reset!");
  lives = INITIAL_LIVES;
  for (int i = 0; i < GAME_START_LED_COUNT; i++) {
    digitalWrite(LED_PINS[i], HIGH);
    tone(PIEZO_PIN, GAME_START_FREQ[i], GAME_START_DURATIONS[i]);
    delay(GAME_START_DURATIONS[i] + 50);
  }
  for (int i = GAME_START_LED_COUNT; i < 7; i++) {
    tone(PIEZO_PIN, GAME_START_FREQ[i], GAME_START_DURATIONS[i]);
    delay(GAME_START_DURATIONS[i] + 50);
  }
  noTone(PIEZO_PIN);
  Serial.println("Ready to play again!");
}

void playGameStartSound() {
  for (int i = 0; i < 7; i++) {
    tone(PIEZO_PIN, POWER_UP_FREQ[i], POWER_UP_TONE_DURATION);
    delay(POWER_UP_TONE_DELAY);
  }
  noTone(PIEZO_PIN);
}