#include "numlock.h"

#include "../behavior.h"
#include "../kind.h"
#include "../hooks.h"
#include "../keymod.h"  // Para keymod_t
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"

// Declaração forward
bool numlock_process_record_user(keyrecord_t *record, keymod_t keymod);

// ===== Variáveis Globais =====
numlock_t* numlock_position = NULL;

// ===== Funções de Registro =====

// Registra a posição de KC_NUM na matriz de behavior
// Usa posição fixa: row=1, col=17
void numlock_register_position(void) {
    numlock_t* pos = kind_get_numlock(1, 17);
    if (pos) {
        behavior_register_position_function(pos->row, pos->col, numlock_process_record_user);
    }
}

// ===== Funções de Estado =====

// Função auxiliar para obter o estado atual do numlock diretamente do host
// Isso evita problemas de sincronização durante a inicialização
static bool numlock_is_on(void) {
    return host_keyboard_led_state().num_lock;
}

// ===== Funções de Hook QMK =====

// Processa KC_NUM (baseado em posição, não keycode)
bool numlock_process_record_user(keyrecord_t *record, keymod_t keymod) {
    if (!record->event.pressed) return true;

    // Obtém posição (row, col)
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // Verifica se a posição corresponde a KC_NUM
    numlock_t* pos = kind_get_numlock(row, col);
    if (pos && pos->keycode == KC_NUM) {
        // Envia KC_NUM ao SO (dispara toggle real)
        tap_code(KC_NUM);
        // Não precisa sincronizar - o estado será verificado diretamente no próximo render
        return false;  // Bloqueia propagação
    }

    return true;
}

// Hook rgb_matrix_indicators_user
static bool numlock_rgb_matrix_indicators_user(void) {
    // Se position não foi inicializado, retorna
    if (numlock_position == NULL) return true;

    // Verifica o estado diretamente do host (sempre atualizado)
    // Isso evita problemas de sincronização durante a inicialização
    uint8_t brightness = calculate_pulse_brightness();
    if (numlock_is_on()) {
        // NUM LOCK ON → Verde pulsante
        color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
        rgb_matrix_set_color(numlock_position->led_index, green.r, green.g, green.b);
    } else {
        // NUM LOCK OFF → Vermelho pulsante
        color_rgb_t red = color_apply_brightness(COLOR_RED, brightness);
        rgb_matrix_set_color(numlock_position->led_index, red.r, red.g, red.b);
    }
    
    return true;
}

// Hook keyboard_post_init_user
static void numlock_keyboard_post_init_user(void) {
    // Registra a posição de KC_NUM
    numlock_register_position();
    
    // Usa posição fixa: row=1, col=17
    numlock_position = kind_get_numlock(1, 17);

    // Não precisa sincronizar estado inicial - será verificado diretamente no render
    // O rgb_matrix_indicators_user será chamado automaticamente e verificará o estado atual
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void numlock_init_early_hooks(void) {
    hooks_rgb_indicators_register(numlock_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void numlock_init_hooks(void) {
    hooks_keyboard_post_init_register(numlock_keyboard_post_init_user);
}

