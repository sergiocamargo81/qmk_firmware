#ifndef KEYMAP_H
#define KEYMAP_H

#include QMK_KEYBOARD_H
#include "common.h"

enum layers {
    WIN_BASE,
    WIN_FN,
};

enum custom_keycodes {
    SZ_RESET = SAFE_RANGE,
    SZ_EECLR,
    PROFILE_0, PROFILE_1, PROFILE_2, PROFILE_3, PROFILE_4,
    PROFILE_5, PROFILE_6, PROFILE_7, PROFILE_8, PROFILE_9,
};

#endif // KEYMAP_H
