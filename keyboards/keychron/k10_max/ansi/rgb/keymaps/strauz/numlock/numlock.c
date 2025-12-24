#include "numlock.h"

#include "../behavior.h"
#include "../kind.h"
#include "../hooks.h"
#include "../keymod.h"  // Para keymod_t
#include "rgb_matrix.h"  // Para g_rgb_timer
#include "lib8tion.h"     // Para sin8, scale8

// Declaração forward
bool numlock_process_record_user(keyrecord_t *record, keymod_t keymod);

// ===== Variáveis Globais =====
numlock_t numlock_key = {
    .led_index = NO_LED,
};

// ===== Funções de Registro =====

// Registra a posição de KC_NUM na matriz de behavior
// Usa posição fixa: row=1, col=17
void numlock_register_position(void) {
    position_t* pos = kind_get_position(1, 17);
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
    position_t* pos = kind_get_position(row, col);
    if (pos && pos->keycode == KC_NUM) {
        // Envia KC_NUM ao SO (dispara toggle real)
        tap_code(KC_NUM);
        // Não precisa sincronizar - o estado será verificado diretamente no próximo render
        return false;  // Bloqueia propagação
    }

    return true;
}

// Hook matrix_scan_user
// Não precisa mais sincronizar - o estado é verificado diretamente no render
static void numlock_matrix_scan_user(void) {
    // Vazio - estado é verificado diretamente em rgb_matrix_indicators_user
}

// Função auxiliar para calcular brilho pulsante
// Retorna valor de 0-255 baseado em seno do tempo
static uint8_t calculate_pulse_brightness(void) {
    // Usa g_rgb_timer para criar efeito de pulsação
    // Velocidade: divide por 8 para pulsação lenta (~2 segundos por ciclo)
    uint8_t time = scale16by8(g_rgb_timer, 1);
    // sin8 retorna 0-255, onde 128 é o meio
    // Ajusta para que o mínimo seja ~30% e máximo seja 100%
    uint8_t sine = sin8(time);
    // Escala de 77 (30% de 255) a 255 (100%)
    return scale8(sine, 178) + 77;
}

// Hook rgb_matrix_indicators_user
static bool numlock_rgb_matrix_indicators_user(void) {
    // Se nenhum LED assignado, retorna
    if (numlock_key.led_index == NO_LED) return true;

    // Verifica o estado diretamente do host (sempre atualizado)
    // Isso evita problemas de sincronização durante a inicialização
    if (numlock_is_on()) {
        // NUM LOCK ON → Apagado (LED desligado)
        rgb_matrix_set_color(numlock_key.led_index, 0, 0, 0);
    } else {
        // NUM LOCK OFF → Vermelho pulsante
        uint8_t brightness = calculate_pulse_brightness();
        rgb_matrix_set_color(numlock_key.led_index, brightness, 0, 0);
    }
    
    return true;
}

// Hook keyboard_post_init_user
static void numlock_keyboard_post_init_user(void) {
    // Registra a posição de KC_NUM
    numlock_register_position();
    
    // Usa posição fixa: row=1, col=17
    position_t* pos = kind_get_position(1, 17);
    if (pos) {
        numlock_key.led_index = pos->led_index;
    } else {
        numlock_key.led_index = NO_LED;
    }

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
    hooks_matrix_scan_register(numlock_matrix_scan_user);
}

