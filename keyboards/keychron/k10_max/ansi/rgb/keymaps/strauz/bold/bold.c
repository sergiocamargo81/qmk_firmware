#include "bold.h"

#include "../kind.h"
#include "../behavior.h"
#include "../hooks.h"
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"
#include "rgb_matrix.h"

// ===== Funções Auxiliares =====

// Callback para atualizar LED de uma tecla bold
static bool bold_update_led_callback(bold_t* bold, void* user_data) {
    uint8_t brightness = *(uint8_t*)user_data;
    
    // Verifica se a tecla está pressionada usando state
    if (bold->state == BOLD_PRESSED) {
        // Tecla está pressionada: vermelho contínuo
        color_rgb_t red = color_get_rgb(COLOR_RED);
        rgb_matrix_set_color(bold->led_index, red.r, red.g, red.b);
    } else {
        // Tecla não está pressionada: laranja pulsante
        color_rgb_t orange = color_apply_brightness(COLOR_ORANGE_BURNT, brightness);
        rgb_matrix_set_color(bold->led_index, orange.r, orange.g, orange.b);
    }
    
    return true; // Continua iteração
}

// ===== Processamento de Eventos =====

// Processa eventos de teclas bold
bool bold_process_record_user(keyrecord_t *record, keymod_t keymod) {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    bool pressed = record->event.pressed;
    
    // Atualiza estado na matrix
    bold_t* bold = kind_get_bold(row, col);
    if (bold != NULL) {
        bold->state = pressed ? BOLD_PRESSED : BOLD_IDLE;
    }
    
    // Não consome o evento, permite que seja processado normalmente
    return true;
}

// ===== Funções de Hook QMK =====

// Hook rgb_matrix_indicators_user
// Atualiza LEDs de todas as teclas bold com pulsação em laranja
static bool bold_rgb_matrix_indicators_user(void) {
    // Calcula brilho pulsante (0-255)
    uint8_t brightness = calculate_pulse_brightness();
    
    // Itera sobre todas as teclas bold e atualiza seus LEDs
    kind_iterate_bold(bold_update_led_callback, &brightness);
    
    return true;
}

// ===== Inicialização =====

// Callback para inicializar estados e registrar handlers
static bool bold_init_callback(bold_t* bold, void* user_data) {
    (void)user_data;
    if (bold != NULL) {
        bold->state = BOLD_IDLE;
        // Registra handler para esta posição
        behavior_register_position_function(bold->row, bold->col, bold_process_record_user);
    }
    return true;
}

// Inicializa estados e registra handlers
static void bold_keyboard_post_init_user(void) {
    // Itera sobre todas as teclas bold e inicializa estados
    kind_iterate_bold(bold_init_callback, NULL);
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void bold_init_early_hooks(void) {
    hooks_rgb_indicators_register(bold_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void bold_init_hooks(void) {
    hooks_keyboard_post_init_register(bold_keyboard_post_init_user);
}
