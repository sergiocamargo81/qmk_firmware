#ifndef KEYMAP_H
#define KEYMAP_H

#include QMK_KEYBOARD_H

// ===== LAYER ENUM =====

enum layers {
	WIN_BASE,
	WIN_FN,
};

// ===== CUSTOM KEYCODES =====

enum custom_keycodes {
	AFR_MODE = SAFE_RANGE,
	AFR_RSET,
	AFR_EECLR
};

#endif // KEYMAP_H
