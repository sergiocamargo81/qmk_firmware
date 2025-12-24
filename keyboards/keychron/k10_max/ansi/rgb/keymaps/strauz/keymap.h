#ifndef KEYMAP_H
#define KEYMAP_H

#include QMK_KEYBOARD_H
#include "settings.h"

enum layers {
    WIN_BASE,
    WIN_FN,
};

// Todos os keycodes customizados foram removidos
// SZ_SAVE foi substituído por KC_END (FN+END para salvar)
// PROFILE_0-9 foram substituídos por KC_P0-KC_P9 (FN+P0-P9 para trocar profile)

// ===== Hooks QMK =====

// Hook process_record_user: processa todos os keycodes e encaminha
bool process_record_user(uint16_t keycode, keyrecord_t *record);

// Hook matrix_scan_user: scans periódicos dos submódulos
void matrix_scan_user(void);

// Hook rgb_matrix_indicators_user: indicadores RGB dos submódulos
bool rgb_matrix_indicators_user(void);

// Hook keyboard_post_init_user: inicialização dos submódulos
void keyboard_post_init_user(void);

// Hook eeconfig_init_user: reset de EEPROM
void eeconfig_init_user(void);

#endif // KEYMAP_H
