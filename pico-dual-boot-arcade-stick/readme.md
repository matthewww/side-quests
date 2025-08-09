# Pi Pico Arcade Stick  Dual Boot Project

---

## Project Overview

This project is about building a custom **Raspberry Pi Pico-based arcade joystick and button controller** with two main operating modes:

1. **USB HID Keyboard Mode:**

   * The Pico acts as a USB keyboard device.
   * Joystick directions and buttons are mapped to keyboard keys compatible with devices like the PSTV.
   * Enables use as a classic game controller for menus and gameplay.

2. **Standalone Game Mode:**

   * The Pico runs small games locally on a connected display (future goal).
   * In this mode, the USB interface is disabled to free resources and simplify operation.
   * Input is read directly from joystick/buttons.
   * A minimal output (initially a UART debug stream and onboard LED heartbeat) signals mode activity.
   * Allows the Pico to act as a self-contained gaming device without a host computer.

## Next Steps

1. **Integrate Display Support:**

   * Add a small OLED screen.
   * Implement minimal display driver support in game mode.

2. **Port/Integrate Games:**

   * Bring in simple games from [MegaGamesCompilation](https://github.com/tscha70/MegaGamesCompilation) or pimoroni.

3. **Stretch Goal:**
   * Keen for this to work on a jailbroken PSTV that we still use.

---