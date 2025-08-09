# Pico Keyboard Arcade Dual Boot Firmware

This firmware supports two modes selected by holding **Button 1 (GP6)** low during boot:

1. **Keyboard HID Mode (default)**  
   - Acts as a USB keyboard sending joystick and button presses.  
   - Enumerates as a standard HID keyboard, compatible with PSTV.

2. **Game Mode**  
   - USB is disabled (no enumeration).  
   - UART0 enabled on pins 0 (TX) and 1 (RX) at 115200 baud for debug output.  
   - Prints "Hello from Game Mode!" on UART and dots every 0.5s.  
   - Onboard LED (GPIO 25) blinks every 0.5s as a heartbeat.

## Wiring

| Pico Pin | Function        | Key Sent     |
|----------|-----------------|--------------|
| GP2      | Joystick Up     | W            |
| GP3      | Joystick Down   | S            |
| GP4      | Joystick Left   | A            |
| GP5      | Joystick Right  | D            |
| GP6      | Button 1        | Space (Mode select)  |
| GP7      | Button 2        | Enter        |
| GP8      | Button 3        | Left Arrow   |
| GP9      | Button 4        | Right Arrow  |
| GPIO0    | UART0 TX (Game Mode debug) |
| GPIO1    | UART0 RX (Game Mode debug) |
| GPIO25   | Onboard LED (blinks in game mode) |

All buttons/switches wired between Pico GPIO and GND (active low).

## Build & Flash

Build instructions are the same as the standard Pico SDK projects:

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
mkdir build
cd build
cmake ..
make
```

Then flash the generated UF2 by putting the Pico into bootloader mode (hold BOOTSEL) and copying the file.

## Notes

- Hold Button 1 (GP6) pressed at boot to enter game mode.
- In game mode, use a serial terminal connected to GPIO0/1 at 115200 baud to see debug output.
- The onboard LED will blink every 0.5 seconds in game mode.
- In keyboard HID mode, USB keyboard works as usual for joystick/buttons.
