#include <TM1637Display.h>

// Define connection pins
#define CLK 2
#define DIO 3

TM1637Display display(CLK, DIO);

// Character segment map for A-Z and space
const uint8_t CHAR_MAP[] = {
  0x77, // A
  0x7C, // B
  0x39, // C
  0x5E, // D
  0x79, // E
  0x71, // F
  0x3D, // G
  0x76, // H
  0x06, // I
  0x1E, // J
  0x00, // K (not representable)
  0x38, // L
  0x36, // M (improved: F | E | B | C)
  0x54, // N (approximate)
  0x3F, // O
  0x73, // P
  0x67, // Q (approximate)
  0x50, // R (approximate)
  0x6D, // S
  0x78, // T
  0x3E, // U
  0x1C, // V (approximate)
  0x2A, // W (approximate)
  0x00, // X (not representable)
  0x6E, // Y
  0x00, // Z (not representable)
  0x00  // space
};

uint8_t getCharSegments(char c) {
  if (c >= 'A' && c <= 'Z') return CHAR_MAP[c - 'A'];
  if (c == ' ') return 0x00;
  return 0x00; // fallback for unsupported chars
}

void scrollText(const char* msg, int delayMs = 300) {
  size_t len = strlen(msg);
  for (size_t i = 0; i < len + 4; i++) {
    uint8_t segs[4];
    for (int j = 0; j < 4; j++) {
      size_t charIndex = (i + j < 4) ? 0 : i + j - 4;
      char c = (charIndex < len) ? msg[charIndex] : ' ';
      segs[j] = getCharSegments(toupper(c));
    }
    display.setSegments(segs);
    delay(delayMs);
  }
}

void setup() {
  display.setBrightness(0x0f); // Max brightness
}

void loop() {
  scrollText(" BOOMSLANG IS THE BEST ");
  delay(1000);
}