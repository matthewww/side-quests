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

// GPIO for mode select
#define MODE_SELECT_PIN 6 // Button 1

// Forward declarations
void run_keyboard_mode(void);
void run_game_mode(void);

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

uint8_t const desc_hid_report[] = {
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07,
    0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01,
    0x75, 0x08, 0x81, 0x03, 0x95, 0x06, 0x75, 0x08,
    0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, 0x00,
    0x29, 0x65, 0x81, 0x00, 0xC0
};

int main(void) {
    // Init stdio (USB or UART depending on mode)
    // Init board and GPIO for inputs
    board_init();

    for (int i = 0; i < 8; i++) {
        gpio_init(pin_map[i]);
        gpio_pull_up(pin_map[i]); // default high (not pressed)
        gpio_set_dir(pin_map[i], GPIO_IN);
    }

    // Read mode select pin at boot
    bool game_mode = (gpio_get(MODE_SELECT_PIN) == 0); // pressed = low

    if (game_mode) {
        run_game_mode();
    } else {
        run_keyboard_mode();
    }

    // Should never reach here
    while (1) {
        tight_loop_contents();
    }
}

// --- Keyboard HID Mode ---

void run_keyboard_mode(void) {
    // Enable USB stdio, disable UART stdio
    stdio_init_all();
    pico_enable_stdio_usb(true);
    pico_enable_stdio_uart(false);

    tusb_init();

    while (1) {
        tud_task();
        send_keys();
        sleep_ms(10);
    }
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    return desc_hid_report;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t* buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
}

// --- Game Mode ---

void run_game_mode(void) {
    // Disable USB, enable UART0 (GPIO0=TX, GPIO1=RX)
    // For debugging, connect UART0 pins to serial monitor at 115200 baud

    // Initialize UART0
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART); // TX
    gpio_set_function(1, GPIO_FUNC_UART); // RX

    // Init onboard LED
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Print "Hello from Game Mode!"
    printf("Hello from Game Mode!\r\n");

    bool led_on = false;

    while (1) {
        printf(".");
        fflush(stdout);
        led_on = !led_on;
        gpio_put(LED_PIN, led_on);
        sleep_ms(500);
    }
}
