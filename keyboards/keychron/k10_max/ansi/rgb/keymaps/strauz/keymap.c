#include QMK_KEYBOARD_H
#include "keychron_common.h"


// ===== CONFIGURAÇÃO AUTOFIRE =====
#define AUTOFIRE_DELAY 500           // ms para começar AutoFire
#define AUTOFIRE_INTERVAL 50         // ms entre repetições
#define AUTOFIRE_KEY_LIMIT 10        // máximo de teclas simultâneas
#define LED_DEACTIVATE_TIME 5000     // ms para manter LED amarelo após desativar


// ===== TIPOS AUTOFIRE =====
typedef struct {
    uint16_t keycode;
    uint32_t timer;
    bool active;
    bool showing_deactivate;
    uint32_t deactivate_timer;
} autofire_key_t;


static autofire_key_t autofire_keys[AUTOFIRE_KEY_LIMIT];
static uint8_t autofire_count = 0;
static bool rgb_matrix_disabled = false;
static bool autofire_profile_mode = false;


// ===== CUSTOM KEYCODES =====
enum custom_keycodes {
    AFR_MODE = SAFE_RANGE,  // Fn+PrintScreen = Toggle autofire profile
};


// ===== ARRAY DE TECLAS ELEGÍVEIS PARA AUTOFIRE =====
static const uint16_t autofire_eligible_keys[] = {
    KC_A,       KC_B,       KC_C,       KC_D,       KC_E,
    KC_F,       KC_G,       KC_H,       KC_I,       KC_J,
    KC_K,       KC_L,       KC_M,       KC_N,       KC_O,
    KC_P,       KC_Q,       KC_R,       KC_S,       KC_T,
    KC_U,       KC_V,       KC_W,       KC_X,       KC_Y,
    KC_Z,       KC_1,       KC_2,       KC_3,       KC_4,
    KC_5,       KC_6,       KC_7,       KC_8,       KC_9,
    KC_0,       KC_MINUS,   KC_EQL,     KC_LBRC,    KC_RBRC,
    KC_BSLS,    KC_SCLN,    KC_QUOT,    KC_GRV,     KC_COMM,
    KC_DOT,     KC_SLSH,    KC_SPC
};


#define AUTOFIRE_ELIGIBLE_COUNT (sizeof(autofire_eligible_keys) / sizeof(autofire_eligible_keys[0]))


// ===== MAPA MANUAL DE ROW/COL PARA CADA KEYCODE =====
typedef struct {
    uint16_t keycode;
    uint8_t row;
    uint8_t col;
} keycode_position_t;


static const keycode_position_t keycode_position_map[] = {
    {KC_GRV,    1, 0},
    {KC_1,      1, 1},
    {KC_2,      1, 2},
    {KC_3,      1, 3},
    {KC_4,      1, 4},
    {KC_5,      1, 5},
    {KC_6,      1, 6},
    {KC_7,      1, 7},
    {KC_8,      1, 8},
    {KC_9,      1, 9},
    {KC_0,      1, 10},
    {KC_MINUS,  1, 11},
    {KC_EQL,    1, 12},

    {KC_Q,      2, 1},
    {KC_W,      2, 2},
    {KC_E,      2, 3},
    {KC_R,      2, 4},
    {KC_T,      2, 5},
    {KC_Y,      2, 6},
    {KC_U,      2, 7},
    {KC_I,      2, 8},
    {KC_O,      2, 9},
    {KC_P,      2, 10},
    {KC_LBRC,   2, 11},
    {KC_RBRC,   2, 12},
    {KC_BSLS,   2, 13},

    {KC_A,      3, 1},
    {KC_S,      3, 2},
    {KC_D,      3, 3},
    {KC_F,      3, 4},
    {KC_G,      3, 5},
    {KC_H,      3, 6},
    {KC_J,      3, 7},
    {KC_K,      3, 8},
    {KC_L,      3, 9},
    {KC_SCLN,   3, 10},
    {KC_QUOT,   3, 11},

    {KC_Z,      4, 2},
    {KC_X,      4, 3},
    {KC_C,      4, 4},
    {KC_V,      4, 5},
    {KC_B,      4, 6},
    {KC_N,      4, 7},
    {KC_M,      4, 8},
    {KC_COMM,   4, 9},
    {KC_DOT,    4, 10},
    {KC_SLSH,   4, 11},

    {KC_SPC,    5, 6},
    {KC_NUM,    1, 17},  // Num Lock - posição no numpad
};


#define KEYCODE_POSITION_MAP_COUNT (sizeof(keycode_position_map) / sizeof(keycode_position_map[0]))


// ===== HELPER FUNCTIONS =====
static bool get_keycode_position(uint16_t keycode, uint8_t *row, uint8_t *col) {
    for (uint8_t i = 0; i < KEYCODE_POSITION_MAP_COUNT; i++) {
        if (keycode_position_map[i].keycode == keycode) {
            *row = keycode_position_map[i].row;
            *col = keycode_position_map[i].col;
            return true;
        }
    }
    return false;
}


static uint8_t get_led_index_from_position(uint8_t row, uint8_t col) {
    uint8_t led_idx = g_led_config.matrix_co[row][col];
    return led_idx;
}


static bool is_autofire_eligible(uint16_t keycode) {
    int32_t left = 0;
    int32_t right = AUTOFIRE_ELIGIBLE_COUNT - 1;
    while (left <= right) {
        int32_t mid = left + ((right - left) / 2);
        uint16_t mid_key = autofire_eligible_keys[mid];
        if (mid_key == keycode) {
            return true;
        } else if (mid_key < keycode) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return false;
}


static int8_t find_autofire_index(uint16_t keycode) {
    for (uint8_t i = 0; i < autofire_count; i++) {
        if (autofire_keys[i].keycode == keycode) {
            return i;
        }
    }
    return -1;
}


static bool add_autofire_key(uint16_t keycode) {
    if (autofire_count >= AUTOFIRE_KEY_LIMIT) {
        return false;
    }
    int8_t idx = find_autofire_index(keycode);
    if (idx == -1) {
        autofire_key_t *key = &autofire_keys[autofire_count];
        key->keycode = keycode;
        key->timer = timer_read32();
        key->active = false;
        key->deactivate_timer = 0;
        key->showing_deactivate = false;
        autofire_count++;
        return true;
    } else {
        autofire_key_t *key = &autofire_keys[idx];
        key->timer = timer_read32();
        key->active = false;
        key->deactivate_timer = 0;
        key->showing_deactivate = false;
        return true;
    }
}


static void remove_autofire_key(uint16_t keycode) {
    int8_t idx = find_autofire_index(keycode);
    if (idx != -1) {
        autofire_key_t *key = &autofire_keys[idx];
        if (key->active) {
            key->deactivate_timer = timer_read32();
            key->showing_deactivate = true;
            key->active = false;
        } else {
            for (uint8_t j = idx; j < autofire_count - 1; j++) {
                autofire_keys[j] = autofire_keys[j + 1];
            }
            autofire_count--;
        }
    }
}


static void clear_all_autofire(void) {
    for (uint8_t i = 0; i < autofire_count; i++) {
        autofire_key_t *key = &autofire_keys[i];
        if (key->active && !key->showing_deactivate) {
            key->showing_deactivate = true;
            key->deactivate_timer = timer_read32();
            key->active = false;
        }
    }
}


enum layers {
    MAC_BASE,
    MAC_FN,
    WIN_BASE,
    WIN_FN,
};


// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAC_BASE] = LAYOUT_ansi_108(
        KC_ESC,             KC_BRID,  KC_BRIU,  KC_MCTRL, KC_LNPAD, UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,    KC_VOLU,    KC_SNAP,  KC_SIRI,  UG_NEXT,  KC_F13,   KC_F14,   KC_F15,   KC_F16,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,     KC_BSPC,    KC_INS,   KC_HOME,  KC_PGUP,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,    KC_BSLS,    KC_DEL,   KC_END,   KC_PGDN,  KC_P7,    KC_P8,    KC_P9,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,                                   KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,              KC_UP,              KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,  KC_LOPTN, KC_LCMMD,                               KC_SPC,                                 KC_RCMMD, KC_ROPTN, MO(MAC_FN), KC_RCTL,    KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT,  KC_PENT),

    [MAC_FN] = LAYOUT_ansi_108(
        _______,            KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,     AFR_MODE,  _______,  UG_TOGG,  _______,  _______,  _______,  _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,
        UG_TOGG,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                                  _______,  _______,  _______,  _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,              _______,            _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,            _______,  _______),

    [WIN_BASE] = LAYOUT_ansi_108(
        KC_ESC,                   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,     KC_PSCR,  KC_CTANA, UG_NEXT,  _______,  _______,  _______,  _______,
        KC_GRV,         KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,     KC_BSPC,    KC_INS,   KC_HOME,  KC_PGUP,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,         KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,    KC_BSLS,    KC_DEL,   KC_END,   KC_PGDN,  KC_P7,    KC_P8,    KC_P9,
        LSFT(KC_TAB),   KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,                                   KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,                  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,              KC_UP,              KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,        KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  KC_RWIN,  MO(WIN_FN), KC_RCTL,    KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT,  KC_PENT),

    [WIN_FN] = LAYOUT_ansi_108(
        _______,            KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,    KC_VOLU,    AFR_MODE,  _______,  UG_TOGG,  _______,  _______,  _______,  _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,
        UG_TOGG,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                                  _______,  _______,  _______,  _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,              _______,            _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,            _______,  _______)
};
// clang-format on


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_keychron_common(keycode, record)) {
        return false;
    }

    if (keycode == AFR_MODE && record->event.pressed) {
        autofire_profile_mode = !autofire_profile_mode;
        return true;
    }

    if (keycode == UG_TOGG && record->event.pressed) {
        rgb_matrix_disabled = !rgb_matrix_disabled;
        return true;
    }

    if (keycode == KC_SPC && record->event.pressed) {
        clear_all_autofire();
        add_autofire_key(KC_SPC);
        return true;
    }
    if (keycode == KC_SPC && !record->event.pressed) {
        remove_autofire_key(KC_SPC);
        return true;
    }

    if (is_autofire_eligible(keycode)) {
        if (record->event.pressed) {
            add_autofire_key(keycode);
            return true;
        } else {
            remove_autofire_key(keycode);
            return true;
        }
    }
    return true;
}


void matrix_scan_user(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < autofire_count; i++) {
        autofire_key_t *key = &autofire_keys[i];
        uint32_t elapsed = timer_elapsed32(key->timer);

        if (key->showing_deactivate) {
            uint32_t deactivate_elapsed = timer_elapsed32(key->deactivate_timer);
            if (deactivate_elapsed >= LED_DEACTIVATE_TIME) {
                for (uint8_t j = i; j < autofire_count - 1; j++) {
                    autofire_keys[j] = autofire_keys[j + 1];
                }
                autofire_count--;
                i--;
                continue;
            }
            continue;
        }

        if (!key->active && elapsed >= AUTOFIRE_DELAY) {
            key->active = true;
            key->timer = now;
            tap_code(key->keycode);
        }

        if (key->active) {
            uint32_t repeat_elapsed = timer_elapsed32(key->timer);
            if (repeat_elapsed >= AUTOFIRE_INTERVAL) {
                tap_code(key->keycode);
                key->timer = now;
            }
        }
    }
}


bool rgb_matrix_indicators_user(void) {
    // Se RGB foi desativado com Fn+Tab, apagar tudo
    if (rgb_matrix_disabled) {
        rgb_matrix_set_color_all(RGB_OFF);
        return false;
    }

    // PERFIL AUTOFIRE: Apaga tudo, mostra APENAS autofire + Num Lock
    if (autofire_profile_mode) {
        // Apagar todos os LEDs primeiro
        rgb_matrix_set_color_all(RGB_OFF);

        // Mostrar Num Lock em branco (indicador persistente)
        uint8_t row, col;
        if (get_keycode_position(KC_NUM, &row, &col)) {
            uint8_t led_idx = get_led_index_from_position(row, col);
            if (led_idx != NO_LED) {
                if (host_keyboard_led_state().num_lock) {
                    rgb_matrix_set_color(led_idx, RGB_WHITE);
                } else {
                    rgb_matrix_set_color(led_idx, RGB_OFF);
                }
            }
        }

        // Mostrar LEDs do autofire
        for (uint8_t i = 0; i < autofire_count; i++) {
            autofire_key_t *key = &autofire_keys[i];

            uint8_t row2, col2;
            if (get_keycode_position(key->keycode, &row2, &col2)) {
                uint8_t led_idx = get_led_index_from_position(row2, col2);

                if (led_idx != NO_LED) {
                    if (key->showing_deactivate) {
                        rgb_matrix_set_color(led_idx, RGB_YELLOW);
                    } else if (key->active) {
                        rgb_matrix_set_color(led_idx, RGB_RED);
                    } else {
                        rgb_matrix_set_color(led_idx, RGB_WHITE);
                    }
                }
            }
        }

        // Retorna false para não deixar RGB padrão sobrepor
        return false;
    }

    // RGB NORMAL: deixar padrão do teclado funcionar
    return true;
}
