cmake_minimum_required(VERSION 3.13)

# Set the project name and language
project(pico-keyboard-arcade C CXX ASM)

# Initialize the Raspberry Pi Pico SDK
# Assumes PICO_SDK_PATH is set externally or pass -DPICO_SDK_PATH=path when calling cmake
include(pico_sdk_import.cmake)

pico_sdk_init()

add_executable(pico-keyboard-arcade
    src/keyboard_arcade.c
)

# Link Pico standard libraries + TinyUSB device library for HID
target_link_libraries(pico-keyboard-arcade
    pico_stdlib
    tinyusb_device
)

# Enable USB output and disable UART
pico_enable_stdio_usb(pico-keyboard-arcade 1)
pico_enable_stdio_uart(pico-keyboard-arcade 0)

# Create UF2 file for flashing
pico_add_extra_outputs(pico-keyboard-arcade)
