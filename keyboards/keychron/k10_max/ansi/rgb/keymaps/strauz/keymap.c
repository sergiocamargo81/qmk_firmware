#include QMK_KEYBOARD_H
#include "keymap.h"
#include "keychron_common.h"
#include "behavior.h"
#include "hold/keymap.h"


// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [WIN_BASE] = LAYOUT_ansi_108(
        KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_PSCR, KC_CTANA, UG_NEXT, _______, _______, _______, _______,
        KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC, KC_INS, KC_HOME, KC_PGUP, KC_NUM, KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS, KC_DEL, KC_END, KC_PGDN, KC_P7, KC_P8, KC_P9,
        LSFT(KC_TAB), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT, KC_P4, KC_P5, KC_P6, KC_PPLS,
        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, KC_UP, KC_P1, KC_P2, KC_P3,
        KC_LCTL, KC_LWIN, KC_LALT, KC_SPC, KC_RALT, KC_RWIN, MO(WIN_FN), KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT, KC_P0, KC_PDOT, KC_PENT),


    [WIN_FN] = LAYOUT_ansi_108(
        _______, KC_BRID, KC_BRIU, KC_TASK, KC_FILE, UG_VALD, UG_VALU, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, KC_PSCR, _______, UG_TOGG, _______, _______, _______, _______,
        _______, BT_HST1, BT_HST2, BT_HST3, P2P4G, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        UG_TOGG, UG_NEXT, UG_VALU, UG_HUEU, UG_SATU, UG_SPDU, _______, _______, _______, _______, _______, _______, _______, _______, SZ_RESET, SZ_EECLR, _______, PROFILE_7, PROFILE_8, PROFILE_9,
        _______, UG_PREV, UG_VALD, UG_HUED, UG_SATD, UG_SPDD, _______, _______, _______, _______, _______, _______, _______, PROFILE_4, PROFILE_5, PROFILE_6, _______,
        _______, _______, _______, _______, _______, BAT_LVL, NK_TOGG, _______, _______, _______, _______, _______, _______, PROFILE_1, PROFILE_2, PROFILE_3,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, PROFILE_0, _______, _______)
};
// clang-format on


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_keychron_common(keycode, record)) return false;
    if (!process_record_behavior(keycode, record)) return false;
    if (!process_record_hold(keycode, record)) return false;
    return true;
}


void matrix_scan_user(void) { matrix_scan_hold(); }
bool rgb_matrix_indicators_user(void) { return rgb_matrix_indicators_hold(); }
void keyboard_post_init_user(void) { behavior_init(); keyboard_post_init_hold(); }


void eeconfig_init_user(void) {
    behavior_init_defaults();
    behavior_save_to_eeprom();
    eeconfig_init_hold();
}
