#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// SWITCH COUNT SELECTION
// Uncomment exactly one. Relay count, RGB LED count and touch-sensor count
// are always equal to SWITCH_COUNT - there is no separate/fixed sensor count.
// ============================================================================
// #define SWITCH_COUNT 1
// #define SWITCH_COUNT 2
#define SWITCH_COUNT 3
// #define SWITCH_COUNT 4

// ============================================================================
// PER-SWITCH-COUNT HARDWARE MAPPING
// Extracted from the legacy UniversalSwitchControl DeviceConfig.h.
//
// IMPORTANT: two unrelated numbering spaces appear below. Do not compare
// them to each other:
//   - SR_DATA_PIN / SR_CLOCK_PIN / SR_LATCH_PIN / TOUCH_PINS are real GPIO
//     pin numbers (electrical connections).
//   - RELAY_BITS / RED_BITS / GREEN_BITS / BLUE_BITS are bit indices (0-15)
//     inside the 16-bit word shifted out to the two 74HC595s.
// ============================================================================

#if SWITCH_COUNT == 1
    #define SR_DATA_PIN  13
    #define SR_CLOCK_PIN 16
    #define SR_LATCH_PIN 14
    static const uint8_t TOUCH_PINS[SWITCH_COUNT] = {5};
    static const uint8_t RELAY_BITS[SWITCH_COUNT] = {10};
    static const uint8_t RED_BITS[SWITCH_COUNT]   = {4};
    static const uint8_t GREEN_BITS[SWITCH_COUNT] = {6};
    static const uint8_t BLUE_BITS[SWITCH_COUNT]  = {5};

#elif SWITCH_COUNT == 2
    #define SR_DATA_PIN  13
    #define SR_CLOCK_PIN 16
    #define SR_LATCH_PIN 14
    static const uint8_t TOUCH_PINS[SWITCH_COUNT] = {4, 12};
    static const uint8_t RELAY_BITS[SWITCH_COUNT] = {11, 9};
    static const uint8_t RED_BITS[SWITCH_COUNT]   = {1, 12};
    static const uint8_t GREEN_BITS[SWITCH_COUNT] = {3, 14};
    static const uint8_t BLUE_BITS[SWITCH_COUNT]  = {2, 13};

#elif SWITCH_COUNT == 3
    #define SR_DATA_PIN  13
    #define SR_CLOCK_PIN 16
    #define SR_LATCH_PIN 14
    static const uint8_t TOUCH_PINS[SWITCH_COUNT] = {4, 5, 12};
    static const uint8_t RELAY_BITS[SWITCH_COUNT] = {11, 10, 9};
    static const uint8_t RED_BITS[SWITCH_COUNT]   = {1, 4, 12};
    static const uint8_t GREEN_BITS[SWITCH_COUNT] = {3, 6, 14};
    static const uint8_t BLUE_BITS[SWITCH_COUNT]  = {2, 5, 13};

#elif SWITCH_COUNT == 4
    // NOTE: on this variant SR_DATA_PIN is GPIO1 (hardware UART TX), because
    // GPIO13 is used for a touch sensor instead. This is a fixed PCB trace,
    // not a firmware choice - Serial logging must stay off (or be moved via
    // Serial.swap()) on this variant, or boot-time UART output will corrupt
    // the shift-register data line. Flag any issue seen on real hardware.
    #define SR_DATA_PIN  1
    #define SR_CLOCK_PIN 16
    #define SR_LATCH_PIN 14
    static const uint8_t TOUCH_PINS[SWITCH_COUNT] = {4, 5, 13, 12};
    static const uint8_t RELAY_BITS[SWITCH_COUNT] = {10, 9, 12, 11};
    static const uint8_t RED_BITS[SWITCH_COUNT]   = {8, 5, 0, 15};
    static const uint8_t GREEN_BITS[SWITCH_COUNT] = {3, 7, 1, 14};
    static const uint8_t BLUE_BITS[SWITCH_COUNT]  = {4, 6, 2, 13};

#else
    #error "SWITCH_COUNT must be 1, 2, 3 or 4"
#endif

// ============================================================================
// COLORS
// Each channel is ON/OFF only -> 8 possible combinations. COLOR_RGB_BITS
// gives bit0=Red, bit1=Green, bit2=Blue for each enum value, so
// ShiftRegister::setColor() is a table lookup instead of a branch chain.
// ============================================================================
enum RGBColor : uint8_t {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_WHITE,
    COLOR_COUNT
};

static const uint8_t COLOR_RGB_BITS[COLOR_COUNT] = {
    0b000, // OFF
    0b001, // RED
    0b010, // GREEN
    0b100, // BLUE
    0b011, // YELLOW  = R+G
    0b110, // CYAN    = G+B
    0b101, // MAGENTA = R+B
    0b111  // WHITE   = R+G+B
};

// Applied on first boot / invalid EEPROM: green while on, red while off.
#define DEFAULT_COLOR_ON  COLOR_GREEN
#define DEFAULT_COLOR_OFF COLOR_RED

// ============================================================================
// TIMING
// ============================================================================
#define TOUCH_DEBOUNCE_MS     50UL
#define RELAY_SAVE_DELAY_MS   300UL    // relay state: short, must survive a quick power loss
#define COLOR_SAVE_DELAY_MS   3000UL   // color prefs: longer, coalesces UI fiddling
#define WIFI_RESTART_DELAY_MS 1500UL   // lets the "saved" HTTP response reach the client first

// ============================================================================
// ACCESS POINT
// Device is AP-only - it never joins another network.
// ============================================================================
#define AP_SSID_MAX_LEN 32   // 802.11 SSID limit
#define AP_PASS_MAX_LEN 63   // WPA2-PSK limit
#define AP_PASS_MIN_LEN 8    // WPA2 minimum (0 is also valid: open network)

#endif // CONFIG_H
