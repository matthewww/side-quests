cmake_minimum_required(VERSION 3.13)

project(pico-keyboard-arcade-dualboot C CXX ASM)

include(pico_sdk_import.cmake)

pico_sdk_init()

add_executable(pico-keyboard-arcade-dualboot
    src/keyboard_arcade.c
)

target_link_libraries(pico-keyboard-arcade-dualboot
    pico_stdlib
    tinyusb_device
)

pico_enable_stdio_usb(pico-keyboard-arcade-dualboot 1)
pico_enable_stdio_uart(pico-keyboard-arcade-dualboot 1)

pico_add_extra_outputs(pico-keyboard-arcade-dualboot)
