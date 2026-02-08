#include <Adafruit_NeoPixel.h>

#define PIN        6      // Pin where NeoPixel is connected
#define NUMPIXELS  1      // Number of NeoPixels

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // Rainbow breathing effect
  for (int j = 0; j < 256; j++) {
    uint32_t color = strip.Color(
      (sin(j * 0.024 + 0) * 127 + 128),
      (sin(j * 0.024 + 2) * 127 + 128),
      (sin(j * 0.024 + 4) * 127 + 128)
    );
    strip.setPixelColor(0, color);
    strip.show();
    delay(10);
  }
}