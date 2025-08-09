#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"

// HID keycodes from USB HID Usage Tables
#define KC_A        0x04
#define KC_D        0x07
#define KC_S        0x16
#define KC_W        0x1A
#define KC_SPACE    0x2C
#define KC_ENTER    0x28
#define KC_LEFT     0x50
#define KC_RIGHT    0x4F

// Pin mapping
const uint8_t pin_map[] = { 2, 3, 4, 5, 6, 7, 8, 9 };
const uint8_t key_map[] = { KC_W, KC_S, KC_A, KC_D, KC_SPACE, KC_ENTER, KC_LEFT, KC_RIGHT };

void send_keys(void) {
    uint8_t keycode[6] = {0};
    int idx = 0;

    for (int i = 0; i < 8 && idx < 6; i++) {
        if (!gpio_get(pin_map[i])) { // pressed = low
            keycode[idx++] = key_map[i];
        }
    }

    tud_hid_keyboard_report(0, 0, keycode);
}

void release_keys(void) {
    uint8_t empty[6] = {0};
    tud_hid_keyboard_report(0, 0, empty);
}

uint8_t const desc_hid_report[] = {
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07,
    0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01,
    0x75, 0x08, 0x81, 0x03, 0x95, 0x06, 0x75, 0x08,
    0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, 0x00,
    0x29, 0x65, 0x81, 0x00, 0xC0
};

int main(void) {
    board_init();
    tusb_init();

    for (int i = 0; i < 8; i++) {
        gpio_init(pin_map[i]);
        gpio_pull_up(pin_map[i]); // default high
        gpio_set_dir(pin_map[i], GPIO_IN);
    }

    while (1) {
        tud_task();
        send_keys();
        sleep_ms(10); // simple debounce
    }
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    return desc_hid_report;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t* buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer, uint16_t bufsize) {
}
