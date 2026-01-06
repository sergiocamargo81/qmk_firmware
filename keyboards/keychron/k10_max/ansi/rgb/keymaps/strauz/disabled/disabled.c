#include "disabled.h"

#include "../kind.h"
#include "../behavior.h"
#include "../hooks.h"
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"
#include "rgb_matrix.h"

// ===== Funções Auxiliares =====

// Callback para atualizar LED de uma tecla disabled
static bool disabled_update_led_callback(disabled_t* disabled, void* user_data) {
    uint8_t brightness = *(uint8_t*)user_data;
    
    // Garante que o estado está inicializado (se não estiver, assume IDLE)
    if (disabled->state != DISABLED_IDLE && disabled->state != DISABLED_PRESSED) {
        disabled->state = DISABLED_IDLE;
    }
    
    // Verifica se a tecla está pressionada usando state
    if (disabled->state == DISABLED_PRESSED) {
        // Tecla está pressionada: verde contínuo
        color_rgb_t green = color_get_rgb(COLOR_GREEN);
        rgb_matrix_set_color(disabled->led_index, green.r, green.g, green.b);
    } else {
        // Tecla não está pressionada: branco pulsante
        color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
        rgb_matrix_set_color(disabled->led_index, white.r, white.g, white.b);
    }
    
    return true; // Continua iteração
}

// ===== Processamento de Eventos =====

// Processa eventos de teclas disabled
bool disabled_process_record_user(keyrecord_t *record, keymod_t keymod) {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    bool pressed = record->event.pressed;
    
    // Atualiza estado na matrix
    disabled_t* disabled = kind_get_disabled(row, col);
    if (disabled != NULL) {
        disabled->state = pressed ? DISABLED_PRESSED : DISABLED_IDLE;
    }
    
    // Não consome o evento, permite que seja processado normalmente
    return true;
}

// ===== Funções de Hook QMK =====

// Hook rgb_matrix_indicators_user
// Atualiza LEDs de todas as teclas disabled com pulsação em branco
static bool disabled_rgb_matrix_indicators_user(void) {
    // Calcula brilho pulsante (0-255)
    uint8_t brightness = calculate_pulse_brightness();
    
    // Itera sobre todas as teclas disabled e atualiza seus LEDs
    kind_iterate_disabled(disabled_update_led_callback, &brightness);
    
    return true;
}

// ===== Inicialização =====

// Callback para inicializar estados e registrar handlers
static bool disabled_init_callback(disabled_t* disabled, void* user_data) {
    (void)user_data;
    if (disabled != NULL) {
        disabled->state = DISABLED_IDLE;
        // Registra handler para esta posição
        behavior_register_position_function(disabled->row, disabled->col, disabled_process_record_user);
    }
    return true;
}

// Inicializa estados e registra handlers
static void disabled_keyboard_post_init_user(void) {
    // Itera sobre todas as teclas disabled e inicializa estados
    kind_iterate_disabled(disabled_init_callback, NULL);
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void disabled_init_early_hooks(void) {
    hooks_rgb_indicators_register(disabled_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void disabled_init_hooks(void) {
    hooks_keyboard_post_init_register(disabled_keyboard_post_init_user);
}
