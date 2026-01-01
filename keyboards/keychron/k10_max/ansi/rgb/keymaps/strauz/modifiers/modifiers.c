#include "modifiers.h"

#include "../hooks.h"
#include "../keymap.h"  // Para WIN_FN
#include "../kind.h"    // Para kind_get_modifier
#include "matrix.h"
#include "action_layer.h"  // Para layer_state_is
#include "rgb_matrix.h"

// ===== Defines =====

// Cor roxa quando FN está pressionada
#define MODIFIER_PURPLE_R 128
#define MODIFIER_PURPLE_G 0
#define MODIFIER_PURPLE_B 128

// ===== Variáveis Globais =====

static modifiers_fn_state_callback_t fn_callbacks[MODIFIERS_MAX_CALLBACKS] = {NULL};
static uint8_t fn_callback_count = 0;
static bool fn_pressed_state = false;
static bool fn_prev_state = false;

// Cache de ponteiros para modifiers do grid (inicializados durante init)
static modifier_t* fn_modifier = NULL;
static modifier_t* ralt_modifier = NULL;
static modifier_t* rctl_modifier = NULL;
static modifier_t* rsft_modifier = NULL;
static bool modifiers_initialized = false;

// ===== Funções Auxiliares =====

// Detecta se FN está pressionada
static bool detect_fn_pressed(void) {
    if (fn_modifier == NULL) {
        return false;
    }
    
    // Verifica se MO(WIN_FN) está sendo pressionado diretamente na matriz
    if (matrix_is_on(fn_modifier->row, fn_modifier->col)) {
        return true;
    }
    
    // Verifica se a layer FN está ativa
    if (layer_state_is(WIN_FN)) {
        return true;
    }
    
    return false;
}

// Notifica todos os callbacks registrados sobre mudança de estado de FN
static void notify_fn_state_change(bool fn_pressed) {
    for (uint8_t i = 0; i < fn_callback_count; i++) {
        if (fn_callbacks[i] != NULL) {
            fn_callbacks[i](fn_pressed);
        }
    }
}

// Atualiza LEDs dos modificadores
static void update_modifier_leds(void) {
    if (!modifiers_initialized) {
        return;
    }
    
    if (fn_pressed_state) {
        // FN está pressionada: acende todos em roxo
        if (fn_modifier != NULL && fn_modifier->led_index != NO_LED) {
            rgb_matrix_set_color(fn_modifier->led_index, MODIFIER_PURPLE_R, MODIFIER_PURPLE_G, MODIFIER_PURPLE_B);
        }
        if (ralt_modifier != NULL && ralt_modifier->led_index != NO_LED) {
            rgb_matrix_set_color(ralt_modifier->led_index, MODIFIER_PURPLE_R, MODIFIER_PURPLE_G, MODIFIER_PURPLE_B);
        }
        if (rctl_modifier != NULL && rctl_modifier->led_index != NO_LED) {
            rgb_matrix_set_color(rctl_modifier->led_index, MODIFIER_PURPLE_R, MODIFIER_PURPLE_G, MODIFIER_PURPLE_B);
        }
        if (rsft_modifier != NULL && rsft_modifier->led_index != NO_LED) {
            rgb_matrix_set_color(rsft_modifier->led_index, MODIFIER_PURPLE_R, MODIFIER_PURPLE_G, MODIFIER_PURPLE_B);
        }
    } else {
        // FN não está pressionada: desliga LEDs (outros módulos controlam)
        // Não fazemos nada aqui - deixamos outros módulos controlarem
    }
}

// ===== API Pública =====

bool modifiers_register_fn_callback(modifiers_fn_state_callback_t callback) {
    if (callback == NULL) {
        return false;
    }
    
    // Verifica se já está registrado
    for (uint8_t i = 0; i < fn_callback_count; i++) {
        if (fn_callbacks[i] == callback) {
            return false; // Já registrado
        }
    }
    
    // Verifica se há espaço
    if (fn_callback_count >= MODIFIERS_MAX_CALLBACKS) {
        return false; // Limite atingido
    }
    
    // Registra callback
    fn_callbacks[fn_callback_count++] = callback;
    return true;
}

bool modifiers_unregister_fn_callback(modifiers_fn_state_callback_t callback) {
    if (callback == NULL) {
        return false;
    }
    
    // Procura e remove callback
    for (uint8_t i = 0; i < fn_callback_count; i++) {
        if (fn_callbacks[i] == callback) {
            // Move callbacks seguintes para frente
            for (uint8_t j = i; j < fn_callback_count - 1; j++) {
                fn_callbacks[j] = fn_callbacks[j + 1];
            }
            fn_callbacks[--fn_callback_count] = NULL;
            return true;
        }
    }
    
    return false; // Não encontrado
}

bool modifiers_is_fn_pressed(void) {
    return fn_pressed_state;
}

// ===== Inicialização =====

void modifiers_init(void) {
    // Inicializa estado
    fn_pressed_state = false;
    fn_prev_state = false;
    
    // Obtém ponteiros para modifiers do grid
    // Posições baseadas no pool de modifiers em kind.c:
    // RSFT: row 4, col 13
    // RALT: row 5, col 10
    // FN:   row 5, col 12
    // RCTL: row 5, col 13
    fn_modifier = kind_get_modifier(5, 12);
    ralt_modifier = kind_get_modifier(5, 10);
    rctl_modifier = kind_get_modifier(5, 13);
    rsft_modifier = kind_get_modifier(4, 13);
    
    modifiers_initialized = true;
}

// ===== Hooks QMK =====

// Hook matrix_scan_user: detecta mudanças de estado de FN
static void modifiers_matrix_scan_user(void) {
    // Detecta estado atual de FN
    fn_pressed_state = detect_fn_pressed();
    
    // Verifica se houve mudança de estado
    if (fn_pressed_state != fn_prev_state) {
        // Notifica callbacks sobre mudança de estado
        notify_fn_state_change(fn_pressed_state);
        fn_prev_state = fn_pressed_state;
    }
}

// Hook rgb_matrix_indicators_user: atualiza LEDs dos modificadores
static bool modifiers_rgb_matrix_indicators_user(void) {
    update_modifier_leds();
    return true;
}

// ===== Registro de Hooks =====

void modifiers_init_hooks(void) {
    // Registra hook de matrix_scan para detectar mudanças de estado
    hooks_matrix_scan_register(modifiers_matrix_scan_user);
    
    // Registra hook de rgb_indicators para atualizar LEDs
    hooks_rgb_indicators_register(modifiers_rgb_matrix_indicators_user);
}
