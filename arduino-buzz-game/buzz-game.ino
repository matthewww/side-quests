const int LED_PINS[] = {9, 10, 11}; // Anode (220 res to ground)
const int BUTTON_PIN = 2;
const int PIEZO_PIN = 8;

int lives = 3;
bool lastButtonState = HIGH;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0; 
const unsigned long DEBOUNCE_DELAY = 50;

const int LIFE_LOST_FREQ[] = {659, 523, 330};
const int GAME_OVER_FREQ[] = {523, 494, 466, 440, 415, 392};
const int GAME_START_FREQ[] = {659, 659, 659, 523, 784, 698, 659};
const int POWER_UP_FREQ[] = {1047, 1175, 1319, 1397, 1568, 1760, 1976};

void setup() {
  for (int i = 0; i < 3; i++) {
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
        gameOver();
      }
    }
  }
}

bool checkButtonPress() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    lastButtonState = currentButtonState;
    delay(50); //  debounce delay
    return true;
  }
  
  lastButtonState = currentButtonState;
  return false;
}

void loseLife() {
  int ledIndex = 3 - lives;
  
  for (int i = 0; i < 3; i++) {
    tone(PIEZO_PIN, LIFE_LOST_FREQ[i], 200);
    digitalWrite(LED_PINS[ledIndex], LOW);
    delay(120);
    digitalWrite(LED_PINS[ledIndex], HIGH);
    delay(80);
  }
  
  for (int i = 0; i < 6; i++) {
    digitalWrite(LED_PINS[ledIndex], i % 2);
    delay(100);
  }
  
  digitalWrite(LED_PINS[ledIndex], LOW);
  noTone(PIEZO_PIN);
}

void gameOver() {
  Serial.println("Game Over!");
  
  int gameOverDurations[] = {300, 300, 300, 300, 400, 800};
  
  for (int i = 0; i < 6; i++) {
    tone(PIEZO_PIN, GAME_OVER_FREQ[i], gameOverDurations[i]);
    delay(gameOverDurations[i] + 50);
  }
  noTone(PIEZO_PIN);
  
  // Wait 5 seconds
  delay(5000);
  resetGame();
}

void resetGame() {
  Serial.println("Game Reset!");
  lives = 3;
  
  int startDurations[] = {150, 150, 150, 200, 150, 150, 300};
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PINS[i], HIGH);
    tone(PIEZO_PIN, GAME_START_FREQ[i], startDurations[i]);
    delay(startDurations[i] + 50);
  }
  
  for (int i = 3; i < 7; i++) {
    tone(PIEZO_PIN, GAME_START_FREQ[i], startDurations[i]);
    delay(startDurations[i] + 50);
  }
  
  noTone(PIEZO_PIN);
  Serial.println("Ready to play again!");
}

void playGameStartSound() {
  for (int i = 0; i < 7; i++) {
    tone(PIEZO_PIN, POWER_UP_FREQ[i], 80);
    delay(90);
  }
  noTone(PIEZO_PIN);
}
