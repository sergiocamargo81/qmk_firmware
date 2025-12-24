#ifndef HOLD_KEYMAP_H
#define HOLD_KEYMAP_H

#include QMK_KEYBOARD_H
#include "../keymap.h"

bool process_record_hold(uint16_t keycode, keyrecord_t *record);
void matrix_scan_hold(void);
bool rgb_matrix_indicators_hold(void);
void keyboard_post_init_hold(void);
void eeconfig_init_hold(void);

#endif // HOLD_KEYMAP_H
