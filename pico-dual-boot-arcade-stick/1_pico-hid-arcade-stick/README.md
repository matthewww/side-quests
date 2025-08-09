# Pico Keyboard Arcade Firmware

This project implements a USB HID keyboard firmware for Raspberry Pi Pico to emulate an arcade joystick (4 directions) and 4 buttons.

---

## Build & Flash

1. Make sure you have the Raspberry Pi Pico SDK installed and
   environment variable `PICO_SDK_PATH` set, e.g.:
   ```bash
   export PICO_SDK_PATH=/path/to/pico-sdk
   ```
2. Create and enter build directory:
   ```bash
   mkdir build
   cd build
   ```
3. Run CMake and build:

   ```bash
   cmake ..
   make
   ```
4. Put your Pico into bootloader mode (hold BOOTSEL and plug USB).
5. Copy the generated UF2 file `pico-keyboard-arcade.uf2` from `build/` to the Pico mass storage device.
6. Remove BOOTSEL, replug, and it should enumerate as a keyboard.

---

## Wiring

| Pico Pin | Function        | Key Sent     |
|----------|-----------------|--------------|
| GP2      | Joystick Up     | W            |
| GP3      | Joystick Down   | S            |
| GP4      | Joystick Left   | A            |
| GP5      | Joystick Right  | D            |
| GP6      | Button 1        | Space        |
| GP7      | Button 2        | Enter        |
| GP8      | Button 3        | Left Arrow   |
| GP9      | Button 4        | Right Arrow  |

All buttons/switches wired between Pico GPIO and GND.

---

## Notes

- Pressing a switch pulls the pin low (active low).
- Firmware sends up to 6 keys pressed simultaneously.
- Uses TinyUSB stack for USB HID.