#include <Adafruit_NeoPixel.h>


#define NEOPIXEL_PIN 6
#define NUM_PIXELS 3
#define PIEZO_PIN 8


Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Helper: Theater chase (Cylon eye) effect
void theaterChase(int cycles = 3, int delayMs = 250) {
  // Forward
  for (int c = 0; c < cycles; c++) {
    for (int i = 0; i < NUM_PIXELS; i++) {


      for (int j = 0; j < NUM_PIXELS; j++) {
        if (j == i) {
          strip.setPixelColor(j, strip.Color(0, 255, 0));
        } else {
          strip.setPixelColor(j, strip.Color(10, 0, 0)); // Dim/off
        }
      }
      strip.show();
      delay(delayMs);
    }
    // Backward
    for (int i = NUM_PIXELS - 2; i > 0; i--) {
      for (int j = 0; j < NUM_PIXELS; j++) {
        if (j == i) {
          strip.setPixelColor(j, strip.Color(255, 0, 0));
        } else {
          strip.setPixelColor(j, strip.Color(10, 0, 0));
        }
      }
      strip.show();
      delay(delayMs);
    }
  }
}

// Helper: Fire effect (flickering red/orange/yellow)
void fireEffect(int durationMs = 5000) {
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    for (int i = 0; i < NUM_PIXELS; i++) {
      // Random red/orange/yellow
      int r = random(180, 255);
      int g = random(20, 80);
      int b = random(0, 10);
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
    delay(random(30, 80));
  }
}

// Helper: Wheel function to get rainbow colors
uint32_t wheel(byte pos) {
  if (pos < 85) {
    return strip.Color(pos * 3, 255 - pos * 3, 0);
  } else if (pos < 170) {
    pos -= 85;
    return strip.Color(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return strip.Color(0, pos * 3, 255 - pos * 3);
  }
}

// Simple melody (notes in Hz and durations in ms)
int melody[] = { 262, 294, 330, 349, 392, 440, 494, 523 }; // C D E F G A B C
int noteDurations[] = { 200, 200, 200, 200, 200, 200, 200, 400 };

void runShow() {
  // Play a simple tune and LED show
  for (int i = 0; i < 8; i++) {
    // Play note
    tone(PIEZO_PIN, melody[i], noteDurations[i]);
    // Show color on NeoPixels
    uint32_t color = strip.Color(
      (i % 3 == 0) ? 255 : 0,
      (i % 3 == 1) ? 255 : 0,
      (i % 3 == 2) ? 255 : 0
    );
    for (int j = 0; j < NUM_PIXELS; j++) {
      strip.setPixelColor(j, color);
    }
    strip.show();
    delay(noteDurations[i]);
    noTone(PIEZO_PIN);
    delay(50);
  }

  // Independent rainbow effect for each pixel
  for (int k = 0; k < 100; k++) {
    for (int i = 0; i < NUM_PIXELS; i++) {
      // Each pixel cycles at a different speed/offset
      strip.setPixelColor(i, wheel((k * 5 + i * 85) & 255));
    }
    strip.show();
    delay(30);
  }


  // Fire effect after rainbow
  fireEffect();

  // Theater chase (Cylon eye) effect after fire
  theaterChase();

  // Turn off LEDs at end of show
  strip.clear();
  strip.show();
}

void setup() {
  strip.begin();
  strip.show();
  runShow();
}

void loop() {
  delay(4000);
  runShow();
}