#ifndef HOOKS_H
#define HOOKS_H

#include QMK_KEYBOARD_H

// ===== EECONFIG Init Hooks =====
// Executado durante quantum_init() se EEPROM inválida (primeiro hook)

// Tamanho do array de callbacks
// ATENÇÃO: Se adicionar um novo evento que registre neste hook, AUMENTE este valor!
// Registros atuais: custom (1)
#define HOOKS_EECONFIG_MAX_CALLBACKS 1

// Tipo de callback para eeconfig_init_user
typedef void (*eeconfig_init_callback_t)(void);

// Registra um callback para eeconfig_init_user
// Retorna true se sucesso, false se limite atingido
bool hooks_eeconfig_init_register(eeconfig_init_callback_t callback);

// Dispara todos os callbacks registrados (chamado por eeconfig_init_user)
void hooks_eeconfig_init_dispatch(void);

// ===== Keyboard Post Init Hooks =====
// Executado no final de keyboard_init() (segundo hook)

// Tamanho do array de callbacks
// ATENÇÃO: Se adicionar um novo evento que registre neste hook, AUMENTE este valor!
// Registros atuais: custom, hold, toggle, numlock, profiles, persistence (6)
#define HOOKS_KEYBOARD_MAX_CALLBACKS 6

// Tipo de callback para keyboard_post_init_user
typedef void (*keyboard_post_init_callback_t)(void);

// Registra um callback para keyboard_post_init_user
// Retorna true se sucesso, false se limite atingido
bool hooks_keyboard_post_init_register(keyboard_post_init_callback_t callback);

// Dispara todos os callbacks registrados (chamado por keyboard_post_init_user)
void hooks_keyboard_post_init_dispatch(void);

// ===== Matrix Scan Hooks =====
// Executado periodicamente no loop principal (terceiro hook)

// Tamanho do array de callbacks
// ATENÇÃO: Se adicionar um novo evento que registre neste hook, AUMENTE este valor!
// Registros atuais: hold, toggle, persistence (3)
#define HOOKS_MATRIX_MAX_CALLBACKS 3

// Tipo de callback para matrix_scan_user
typedef void (*matrix_scan_callback_t)(void);

// Registra um callback para matrix_scan_user
// Retorna true se sucesso, false se limite atingido
bool hooks_matrix_scan_register(matrix_scan_callback_t callback);

// Dispara todos os callbacks registrados (chamado por matrix_scan_user)
void hooks_matrix_scan_dispatch(void);

// ===== RGB Indicators Hooks =====
// Executado periodicamente durante renderização RGB (quarto hook)

// Tamanho do array de callbacks
// ATENÇÃO: Se adicionar um novo evento que registre neste hook, AUMENTE este valor!
// Registros atuais: hold, toggle, numlock, profiles, persistence (5)
#define HOOKS_RGB_MAX_CALLBACKS 5

// Tipo de callback para rgb_matrix_indicators_user
typedef bool (*rgb_indicators_callback_t)(void);

// Registra um callback para rgb_matrix_indicators_user
// Retorna true se sucesso, false se limite atingido
bool hooks_rgb_indicators_register(rgb_indicators_callback_t callback);

// Dispara todos os callbacks registrados (chamado por rgb_matrix_indicators_user)
// Retorna true se todos retornaram true, false caso contrário
bool hooks_rgb_indicators_dispatch(void);

#endif // HOOKS_H

