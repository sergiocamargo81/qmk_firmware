/*
* Keychron V10 Max - Autofire Keyboard Firmware (CONSOLIDATED - OPTIMIZED)

* FEATURES:
* - Autofire System: 5-state machine (WAITING, PRESSING, FIRING, RESTING, DISABLED)
* - State-based LED Feedback: Different colors for each autofire state
* - 48 Autofire-eligible Keys: Letters (A-Z), Numbers (0-9), Special chars, Space
* - Space Key Special Logic: Cancels all other autofires when pressed
* - Windows Layout Only: Single-OS support
* - Num Lock Indicator: Always visible in autofire profile mode
* - Profile Toggle: Fn+Insert to switch between autofire and normal RGB modes
* - Binary Search Optimization: O(log n) key lookup for 48 eligible keys
* - Per-Key Disable with EEPROM: Fn+RControl+KEY to toggle, persists on reboot
* - Reset All: Fn+Delete to reset all keys to ENABLED
* - EEPROM Safe: Uses EECONFIG_USER (official QMK EEPROM area)
* - Bitfield Storage: Array of 6 bytes (one bit per key), using persist_index directly

* TIMING CONFIGURATION:
* - AUTOFIRE_DELAY: 500ms wait before autofire activates
* - AUTOFIRE_INTERVAL: 50ms between each repeated tap
* - LED_DEACTIVATE_TIME: 5000ms (5 seconds) to show RESTING state (yellow LED)

* STATE TRANSITIONS:
* User presses key -> PRESSING (green) -> 500ms delay -> FIRING (red, repeating)
* User releases key -> RESTING (yellow) -> 5s timeout -> WAITING (gray)
* Disabled key -> DISABLED (off) - No autofire, LED off

* CUSTOM KEYCODES:
* - AFR_MODE (Fn+Insert): Toggle autofire profile on/off
* - AFR_RSET (Fn+Delete): Reset all disabled keys (enable everything)
* - Fn+RControl+KEY: Toggle autofire disable for that key (persistent)
*/

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include <string.h>

// ===== AUTOFIRE CONFIGURATION =====
#define AUTOFIRE_DELAY 500
#define AUTOFIRE_INTERVAL 50
#define LED_DEACTIVATE_TIME 5000
#define EECONFIG_AUTOFIRE_DISABLED (EECONFIG_USER)
#define AUTOFIRE_BYTES_FOR_STORAGE 6 // 48 keys / 8 = 6 bytes

// ===== STATE MACHINE TYPES =====
typedef enum {
    DISABLED,  // Off - Key disabled, no autofire
    WAITING,   // Gray - Eligible for autofire (not pressed)
    PRESSING,  // Green - Pressed, waiting 500ms to activate
    FIRING,    // Red - Active, repeating every 50ms
    RESTING    // Yellow - Deactivating, showing for 5s
} autofire_state_t;

typedef struct {
    uint16_t keycode;
    uint8_t row;
    uint8_t col;
    uint8_t led_index;
    autofire_state_t state;
    uint32_t timer;
    uint8_t persist_index; // Index in bitfield for EEPROM persistence
} autofire_key_t;

typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t led_index;
} num_lock_key_t;

static bool autofire_profile_mode = true;
static uint8_t disabled_keys_bitfield[AUTOFIRE_BYTES_FOR_STORAGE] = {0};

// ===== CUSTOM KEYCODES =====
enum custom_keycodes {
    AFR_MODE = SAFE_RANGE,
    AFR_RSET,
    AFR_EECLR
};

// ===== AUTOFIRE-ELIGIBLE KEYS ARRAY =====
// Ordered by keycode for binary search O(log n)
// persist_index matches array position directly for bitfield operations
static autofire_key_t autofire_keys[] = {
    {KC_A,      3, 1,  NO_LED, WAITING, 0, 0},
    {KC_B,      4, 6,  NO_LED, WAITING, 0, 1},
    {KC_C,      4, 4,  NO_LED, WAITING, 0, 2},
    {KC_D,      3, 3,  NO_LED, WAITING, 0, 3},
    {KC_E,      2, 3,  NO_LED, WAITING, 0, 4},
    {KC_F,      3, 4,  NO_LED, WAITING, 0, 5},
    {KC_G,      3, 5,  NO_LED, WAITING, 0, 6},
    {KC_H,      3, 6,  NO_LED, WAITING, 0, 7},
    {KC_I,      2, 8,  NO_LED, WAITING, 0, 8},
    {KC_J,      3, 7,  NO_LED, WAITING, 0, 9},
    {KC_K,      3, 8,  NO_LED, WAITING, 0, 10},
    {KC_L,      3, 9,  NO_LED, WAITING, 0, 11},
    {KC_M,      4, 8,  NO_LED, WAITING, 0, 12},
    {KC_N,      4, 7,  NO_LED, WAITING, 0, 13},
    {KC_O,      2, 9,  NO_LED, WAITING, 0, 14},
    {KC_P,      2, 10, NO_LED, WAITING, 0, 15},
    {KC_Q,      2, 1,  NO_LED, WAITING, 0, 16},
    {KC_R,      2, 4,  NO_LED, WAITING, 0, 17},
    {KC_S,      3, 2,  NO_LED, WAITING, 0, 18},
    {KC_T,      2, 5,  NO_LED, WAITING, 0, 19},
    {KC_U,      2, 7,  NO_LED, WAITING, 0, 20},
    {KC_V,      4, 5,  NO_LED, WAITING, 0, 21},
    {KC_W,      2, 2,  NO_LED, WAITING, 0, 22},
    {KC_X,      4, 3,  NO_LED, WAITING, 0, 23},
    {KC_Y,      2, 6,  NO_LED, WAITING, 0, 24},
    {KC_Z,      4, 2,  NO_LED, WAITING, 0, 25},
    {KC_1,      1, 1,  NO_LED, WAITING, 0, 26},
    {KC_2,      1, 2,  NO_LED, WAITING, 0, 27},
    {KC_3,      1, 3,  NO_LED, WAITING, 0, 28},
    {KC_4,      1, 4,  NO_LED, WAITING, 0, 29},
    {KC_5,      1, 5,  NO_LED, WAITING, 0, 30},
    {KC_6,      1, 6,  NO_LED, WAITING, 0, 31},
    {KC_7,      1, 7,  NO_LED, WAITING, 0, 32},
    {KC_8,      1, 8,  NO_LED, WAITING, 0, 33},
    {KC_9,      1, 9,  NO_LED, WAITING, 0, 34},
    {KC_0,      1, 10, NO_LED, WAITING, 0, 35},
    {KC_SPC,    5, 6,  NO_LED, WAITING, 0, 36},
    {KC_MINUS,  1, 11, NO_LED, WAITING, 0, 37},
    {KC_EQL,    1, 12, NO_LED, WAITING, 0, 38},
    {KC_LBRC,   2, 11, NO_LED, WAITING, 0, 39},
    {KC_RBRC,   2, 12, NO_LED, WAITING, 0, 40},
    {KC_BSLS,   2, 13, NO_LED, WAITING, 0, 41},
    {KC_SCLN,   3, 10, NO_LED, WAITING, 0, 42},
    {KC_QUOT,   3, 11, NO_LED, WAITING, 0, 43},
    {KC_GRV,    1, 0,  NO_LED, WAITING, 0, 44},
    {KC_COMM,   4, 9,  NO_LED, WAITING, 0, 45},
    {KC_DOT,    4, 10, NO_LED, WAITING, 0, 46},
    {KC_SLSH,   4, 11, NO_LED, WAITING, 0, 47},
};

#define AUTOFIRE_KEYS_COUNT (sizeof(autofire_keys) / sizeof(autofire_keys[0]))

num_lock_key_t num_lock_key = {
    .row = 1,
    .col = 17,
    .led_index = NO_LED,
};

// ===== EEPROM FUNCTIONS =====
static void load_disabled_keys(void);
static void save_disabled_keys(void);

void eeconfig_init_user(void) {
    memset(disabled_keys_bitfield, 0, sizeof(disabled_keys_bitfield));
    save_disabled_keys();
}

static void load_disabled_keys(void) {
    eeprom_read_block(disabled_keys_bitfield,
                      (void*)EECONFIG_AUTOFIRE_DISABLED,
                      sizeof(disabled_keys_bitfield));
}

static void save_disabled_keys(void) {
    eeprom_write_block(disabled_keys_bitfield,
                       (void*)EECONFIG_AUTOFIRE_DISABLED,
                       sizeof(disabled_keys_bitfield));
}

// ===== BIT MANIPULATION HELPERS (USING PERSIST_INDEX) =====
static bool is_key_disabled(autofire_key_t *key) {
    if (!key) return false;
    uint8_t byte_index = key->persist_index / 8;
    uint8_t bit_index = key->persist_index % 8;
    return (disabled_keys_bitfield[byte_index] & (1 << bit_index)) != 0;
}

static void set_key_disabled(autofire_key_t *key, bool disabled) {
    if (!key) return;
    uint8_t byte_index = key->persist_index / 8;
    uint8_t bit_index = key->persist_index % 8;
    if (disabled) {
        disabled_keys_bitfield[byte_index] |= (1 << bit_index);
    } else {
        disabled_keys_bitfield[byte_index] &= ~(1 << bit_index);
    }
}

// ===== STATE-BASED DISABLE FUNCTIONS =====
static void sync_disabled_key_to_state(autofire_key_t *key) {
    if (!key) return;
    bool is_disabled = is_key_disabled(key);
    if (is_disabled) {
        key->state = DISABLED;
    } else if (key->state == DISABLED) {
        key->state = WAITING;
    }
}

static void toggle_key_disabled(autofire_key_t *key) {
    if (!key) return;
    bool currently_disabled = is_key_disabled(key);
    set_key_disabled(key, !currently_disabled);
    save_disabled_keys();
    sync_disabled_key_to_state(key);
}

static void reset_all_disabled_keys(void) {
    memset(disabled_keys_bitfield, 0, sizeof(disabled_keys_bitfield));
    save_disabled_keys();
    for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
        autofire_keys[i].state = WAITING;
    }
}

// ===== HELPER FUNCTIONS =====
static bool try_find_autofire_key(uint16_t keycode, autofire_key_t **out_key) {
    int32_t left = 0;
    int32_t right = AUTOFIRE_KEYS_COUNT - 1;
    while (left <= right) {
        int32_t mid = left + (right - left) / 2;
        uint16_t mid_keycode = autofire_keys[mid].keycode;
        if (mid_keycode == keycode) {
            if (autofire_keys[mid].state != DISABLED) {
                *out_key = &autofire_keys[mid];
                return true;
            }
            *out_key = NULL;
            return false;
        }
        if (mid_keycode < keycode) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    *out_key = NULL;
    return false;
}

static autofire_key_t *find_key_by_keycode(uint16_t keycode) {
    int32_t left = 0;
    int32_t right = AUTOFIRE_KEYS_COUNT - 1;
    while (left <= right) {
        int32_t mid = left + (right - left) / 2;
        uint16_t mid_keycode = autofire_keys[mid].keycode;
        if (mid_keycode == keycode) {
            return &autofire_keys[mid];
        }
        if (mid_keycode < keycode) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return NULL;
}

static void activate_autofire_key(autofire_key_t *key) {
    uint32_t now = timer_read32();
    key->state = PRESSING;
    key->timer = now;
}

static void deactivate_autofire_key(autofire_key_t *key) {
    uint32_t now = timer_read32();
    if (key->state == FIRING) {
        key->state = RESTING;
        key->timer = now;
        return;
    }
    if (key->state == PRESSING) {
        key->state = WAITING;
        return;
    }
}

static void deactivate_all_autofire(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
        if (autofire_keys[i].state == FIRING) {
            autofire_keys[i].state = RESTING;
            autofire_keys[i].timer = now;
        }
    }
}

enum layers {
    WIN_BASE,
    WIN_FN,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [WIN_BASE] = LAYOUT_ansi_108(
        KC_ESC,                   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,     KC_PSCR,  KC_CTANA, UG_NEXT,  _______,  _______,  _______,  _______,
        KC_GRV,         KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,     KC_BSPC,    KC_INS,   KC_HOME,  KC_PGUP,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,         KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,    KC_BSLS,    KC_DEL,   KC_END,   KC_PGDN,  KC_P7,    KC_P8,    KC_P9,
        LSFT(KC_TAB),   KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,                                   KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,                  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,              KC_UP,              KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,        KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  KC_RWIN,  MO(WIN_FN), KC_RCTL,    KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT,  KC_PENT),

    [WIN_FN] = LAYOUT_ansi_108(
        _______,            KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,    KC_VOLU,    KC_PSCR,   _______,    UG_TOGG,  _______,  _______,  _______,  _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    AFR_MODE,  _______,    _______,  _______,  _______,  _______,  _______,
        UG_TOGG,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    AFR_RSET,  AFR_EECLR,  _______,  _______,  _______,  _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                                     _______,  _______,  _______,  _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,               _______,              _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,   _______,    _______,  _______,            _______,  _______)
};
// clang-format on

void matrix_init_user(void) {
    num_lock_key.led_index = g_led_config.matrix_co[num_lock_key.row][num_lock_key.col];
    for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
        autofire_keys[i].led_index = g_led_config.matrix_co[autofire_keys[i].row][autofire_keys[i].col];
    }
}

void keyboard_post_init_user(void) {
    load_disabled_keys();
    // Sync all states from EEPROM using persist_index from structure
    for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
        sync_disabled_key_to_state(&autofire_keys[i]);
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    // ===== HARD EEPROM RESET (Fn+End) =====
    if (keycode == AFR_EECLR && record->event.pressed) {
        eeconfig_init();
        eeconfig_init_user();
        reset_all_disabled_keys();
        return false;
    }

    // ===== RESET ALL DISABLED KEYS =====
    if (keycode == AFR_RSET) {
        if (record->event.pressed) {
            reset_all_disabled_keys();
        }
        return true;
    }

    // ===== PROFILE MODE TOGGLE =====
    if (keycode == AFR_MODE) {
        if (record->event.pressed) {
            autofire_profile_mode = !autofire_profile_mode;
        }
        return true;
    }

    if (!process_record_keychron_common(keycode, record)) {
        return false;
    }

    // ===== TOGGLE AUTOFIRE DISABLE (Fn+RControl+KEY) =====
    if (record->event.pressed) {
        uint8_t current_layer = get_highest_layer(layer_state);
        uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();
        if (current_layer == WIN_FN && (mods & MOD_BIT(KC_RCTL))) {
            uint16_t base_keycode = keymap_key_to_keycode(WIN_BASE, record->event.key);
            autofire_key_t *key = find_key_by_keycode(base_keycode);
            if (key) {
                toggle_key_disabled(key);
                return false;
            }
        }
    }

    // ===== SPACE KEY SPECIAL - Cancel all autofires (even if disabled) =====
    if (keycode == KC_SPC) {
        if (record->event.pressed) {
            deactivate_all_autofire();
            autofire_key_t *space_key = find_key_by_keycode(keycode);
            if (space_key) {
                if (space_key->state != DISABLED) {
                    activate_autofire_key(space_key);
                }
            }
        } else {
            autofire_key_t *space_key = find_key_by_keycode(keycode);
            if (space_key) {
                deactivate_autofire_key(space_key);
            }
        }
        return true;
    }

    // ===== AUTOFIRE KEY PROCESSING =====
    autofire_key_t *key = NULL;
    if (try_find_autofire_key(keycode, &key)) {
        if (record->event.pressed) {
            activate_autofire_key(key);
        } else {
            deactivate_autofire_key(key);
        }
        return true;
    }

    return true;
}

void matrix_scan_user(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
        autofire_key_t *key = &autofire_keys[i];
        if (key->state == DISABLED) {
            continue;
        }

        switch (key->state) {
        case PRESSING: {
            uint32_t elapsed = timer_elapsed32(key->timer);
            if (elapsed >= AUTOFIRE_DELAY) {
                key->state = FIRING;
                key->timer = now;
                tap_code(key->keycode);
            }
            break;
        }

        case FIRING: {
            uint32_t repeat_elapsed = timer_elapsed32(key->timer);
            if (repeat_elapsed >= AUTOFIRE_INTERVAL) {
                tap_code(key->keycode);
                key->timer = now;
            }
            break;
        }

        case RESTING: {
            uint32_t rest_elapsed = timer_elapsed32(key->timer);
            if (rest_elapsed >= LED_DEACTIVATE_TIME) {
                key->state = WAITING;
            }
            break;
        }

        default:
            break;
        }
    }
}

bool rgb_matrix_indicators_user(void) {
    if (!autofire_profile_mode) {
        return true;
    }

    rgb_matrix_set_color_all(RGB_OFF);

    if (num_lock_key.led_index != NO_LED) {
        if (host_keyboard_led_state().num_lock) {
            rgb_matrix_set_color(num_lock_key.led_index, RGB_WHITE);
        } else {
            rgb_matrix_set_color(num_lock_key.led_index, RGB_OFF);
        }
    }

    for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
        autofire_key_t *key = &autofire_keys[i];
        if (key->led_index != NO_LED) {
            switch (key->state) {
            case WAITING:
                rgb_matrix_set_color(key->led_index, 128, 128, 128);
                break;
            case PRESSING:
                rgb_matrix_set_color(key->led_index, 0, 255, 0);
                break;
            case FIRING:
                rgb_matrix_set_color(key->led_index, 255, 0, 0);
                break;
            case RESTING:
                rgb_matrix_set_color(key->led_index, 255, 255, 0);
                break;
            case DISABLED:
                rgb_matrix_set_color(key->led_index, RGB_OFF);
                break;
            }
        }
    }

    return false;
}
