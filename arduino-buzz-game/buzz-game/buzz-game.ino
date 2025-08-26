// Arduino Buzz Game

const int LED_PINS[] = {9, 10, 11}; // Anode (Cathode 220 res to ground)
const int NUM_LEDS = 3;
const int BUTTON_PIN = 2;
const int PIEZO_PIN = 8;

const int INITIAL_LIVES = 3;
const unsigned long DEBOUNCE_DELAY = 1000; // Gives the player a moment to react and avoid accidental double touches

// Music and sound effects (frequencies in Hz)
const int POWER_UP_FREQ[] = {1047, 1175, 1319, 1397, 1568, 1760, 1976};
const int LIFE_LOST_FREQ[] = {659, 523, 330};
const int GAME_OVER_FREQ[] = {523, 494, 466, 440, 415, 392};
const int GAME_OVER_DURATIONS[] = {300, 300, 300, 300, 400, 800};
const int GAME_START_FREQ[] = {659, 659, 659, 523, 784, 698, 659};
const int GAME_START_DURATIONS[] = {150, 150, 150, 200, 150, 150, 300};

// Timing constants
const int LIFE_LOST_TONE_DURATION = 200;
const int LIFE_LOST_LED_ON_DELAY = 120;
const int LIFE_LOST_LED_OFF_DELAY = 80;
const int GAME_OVER_WAIT_MS = 5000;
const int GAME_START_LED_COUNT = 3;
const int POWER_UP_TONE_DURATION = 80;
const int POWER_UP_TONE_DELAY = 90;

// State
int lives = INITIAL_LIVES;
bool lastButtonState = HIGH;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;


// Initializes hardware, sets pin modes, starts serial, and plays the game start sound.
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


// Main game loop: checks for button press, handles life loss, and triggers game over.
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


// Checks if the button was pressed with debounce logic.
bool checkButtonPress() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  unsigned long currentTime = millis();

  // Button pressed (LOW) and was not pressed before, and debounce time has passed
  if (currentButtonState == LOW && lastButtonState == HIGH && (currentTime - lastDebounceTime > DEBOUNCE_DELAY)) {
    lastDebounceTime = currentTime;
    lastButtonState = currentButtonState;
    return true;
  }

  lastButtonState = currentButtonState;
  return false;
}


// Handles losing a life: plays sound, blinks LED, and updates state.
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


// Handles game over: plays sound sequence, waits, and resets the game.
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


// Resets the game state and LEDs, and plays the game start sound sequence.
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


// Plays the power-up sound sequence at the start of the game.
void playGameStartSound() {
  for (int i = 0; i < 7; i++) {
    tone(PIEZO_PIN, POWER_UP_FREQ[i], POWER_UP_TONE_DURATION);
    delay(POWER_UP_TONE_DELAY);
  }
  noTone(PIEZO_PIN);
}


// ------------------------------------------------------------
// Hi Tom! Here are some notes to help you understand this script:
// ------------------------------------------------------------
//
// - setup() runs once when the Arduino is powered on or reset. It sets up the pins and starts the game.
// - loop() runs over and over, checking if the button is pressed and handling the game logic.
// - pinMode(pin, mode) tells the Arduino if a pin is for input (like a button) or output (like an LED).
// - digitalWrite(pin, value) turns an LED on (HIGH) or off (LOW).
// - digitalRead(pin) checks if a button is pressed (LOW) or not (HIGH).
// - tone(pin, frequency, duration) makes a sound on the buzzer at a certain pitch for a certain time.
// - delay(ms) pauses the program for a number of milliseconds (1000 ms = 1 second).
// - Serial.print/Serial.println lets you see messages if you connect the Arduino to a computer and open the Serial Monitor.
//
// Some ideas to try next:
// - Change the number of lives (INITIAL_LIVES) to make the game easier or harder.
// - Change the LED pins or add more LEDs for extra effects.
// - Change the sounds by editing the numbers in the *_FREQ or *_DURATIONS arrays.
// - Add a new button to reset the game or do something special.
// - Make the game faster or slower by changing the delay values.
// - Add a "win" condition or a score counter.