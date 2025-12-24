#include "hooks.h"

// ===== EECONFIG Init Hooks =====
// Executado durante quantum_init() se EEPROM inválida (primeiro hook)

static eeconfig_init_callback_t eeconfig_init_callbacks[HOOKS_EECONFIG_MAX_CALLBACKS] = {NULL};
static uint8_t eeconfig_init_callback_count = 0;

bool hooks_eeconfig_init_register(eeconfig_init_callback_t callback) {
    if (callback == NULL) {
        return false;
    }
    
    if (eeconfig_init_callback_count >= HOOKS_EECONFIG_MAX_CALLBACKS) {
        return false; // Limite atingido
    }
    
    // Verifica se já está registrado
    for (uint8_t i = 0; i < eeconfig_init_callback_count; i++) {
        if (eeconfig_init_callbacks[i] == callback) {
            return false; // Já registrado
        }
    }
    
    eeconfig_init_callbacks[eeconfig_init_callback_count++] = callback;
    return true;
}

void hooks_eeconfig_init_dispatch(void) {
    for (uint8_t i = 0; i < eeconfig_init_callback_count; i++) {
        if (eeconfig_init_callbacks[i] != NULL) {
            eeconfig_init_callbacks[i]();
        }
    }
}

// ===== Keyboard Post Init Hooks =====
// Executado no final de keyboard_init() (segundo hook)

static keyboard_post_init_callback_t keyboard_post_init_callbacks[HOOKS_KEYBOARD_MAX_CALLBACKS] = {NULL};
static uint8_t keyboard_post_init_callback_count = 0;

bool hooks_keyboard_post_init_register(keyboard_post_init_callback_t callback) {
    if (callback == NULL) {
        return false;
    }
    
    if (keyboard_post_init_callback_count >= HOOKS_KEYBOARD_MAX_CALLBACKS) {
        return false; // Limite atingido
    }
    
    // Verifica se já está registrado
    for (uint8_t i = 0; i < keyboard_post_init_callback_count; i++) {
        if (keyboard_post_init_callbacks[i] == callback) {
            return false; // Já registrado
        }
    }
    
    keyboard_post_init_callbacks[keyboard_post_init_callback_count++] = callback;
    return true;
}

void hooks_keyboard_post_init_dispatch(void) {
    for (uint8_t i = 0; i < keyboard_post_init_callback_count; i++) {
        if (keyboard_post_init_callbacks[i] != NULL) {
            keyboard_post_init_callbacks[i]();
        }
    }
}

// ===== Matrix Scan Hooks =====
// Executado periodicamente no loop principal (terceiro hook)

static matrix_scan_callback_t matrix_scan_callbacks[HOOKS_MATRIX_MAX_CALLBACKS] = {NULL};
static uint8_t matrix_scan_callback_count = 0;

bool hooks_matrix_scan_register(matrix_scan_callback_t callback) {
    if (callback == NULL) {
        return false;
    }
    
    if (matrix_scan_callback_count >= HOOKS_MATRIX_MAX_CALLBACKS) {
        return false; // Limite atingido
    }
    
    // Verifica se já está registrado
    for (uint8_t i = 0; i < matrix_scan_callback_count; i++) {
        if (matrix_scan_callbacks[i] == callback) {
            return false; // Já registrado
        }
    }
    
    matrix_scan_callbacks[matrix_scan_callback_count++] = callback;
    return true;
}

void hooks_matrix_scan_dispatch(void) {
    for (uint8_t i = 0; i < matrix_scan_callback_count; i++) {
        if (matrix_scan_callbacks[i] != NULL) {
            matrix_scan_callbacks[i]();
        }
    }
}

// ===== RGB Indicators Hooks =====
// Executado periodicamente durante renderização RGB (quarto hook)

static rgb_indicators_callback_t rgb_indicators_callbacks[HOOKS_RGB_MAX_CALLBACKS] = {NULL};
static uint8_t rgb_indicators_callback_count = 0;

bool hooks_rgb_indicators_register(rgb_indicators_callback_t callback) {
    if (callback == NULL) {
        return false;
    }
    
    if (rgb_indicators_callback_count >= HOOKS_RGB_MAX_CALLBACKS) {
        return false; // Limite atingido
    }
    
    // Verifica se já está registrado
    for (uint8_t i = 0; i < rgb_indicators_callback_count; i++) {
        if (rgb_indicators_callbacks[i] == callback) {
            return false; // Já registrado
        }
    }
    
    rgb_indicators_callbacks[rgb_indicators_callback_count++] = callback;
    return true;
}

bool hooks_rgb_indicators_dispatch(void) {
    bool result = true;
    for (uint8_t i = 0; i < rgb_indicators_callback_count; i++) {
        if (rgb_indicators_callbacks[i] != NULL) {
            if (!rgb_indicators_callbacks[i]()) {
                result = false;
            }
        }
    }
    return result;
}

