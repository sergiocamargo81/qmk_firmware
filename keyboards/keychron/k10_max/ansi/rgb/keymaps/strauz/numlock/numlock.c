#include "numlock.h"

#include "../behavior.h"
#include "../kind.h"
#include "../event_bus.h"  // Para Event Bus
#include "../keymod.h"  // Para keymod_t
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"

// Declaração forward
bool numlock_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// ===== Variáveis Globais =====
numlock_t* numlock_position = NULL;

// ===== Funções de Registro =====

// Registra a posição de KC_NUM na matriz de behavior
void numlock_register_position(void) {
    numlock_t* pos = kind_get_numlock_key();
    if (pos) {
        behavior_register_position_function(pos->row, pos->col, numlock_process_key);
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
bool numlock_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (!pressed) return true;

    if (key == NULL || key->kind != KIND_NUMLOCK) {
        return true;
    }
    
    // Verifica se a key corresponde a KC_CALC
    numlock_t* pos = (numlock_t*)key;
    if (pos->keycode == KC_CALC) {
        // Envia KC_CALC ao SO (dispara toggle real)
        tap_code(KC_CALC);
        // Não precisa sincronizar - o estado será verificado diretamente no próximo render
        return false;  // Bloqueia propagação
    }

    return true;
}

// Hook rgb_matrix_indicators_user
static void numlock_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    // Se position não foi inicializado, retorna
    if (numlock_position == NULL) return;

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
}

// Hook keyboard_post_init_user
static void numlock_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;
    // Obtém a tecla numlock diretamente do pool
    numlock_position = kind_get_numlock_key();
    
    // Registra a posição de KC_NUM
    numlock_register_position();

    // Não precisa sincronizar estado inicial - será verificado diretamente no render
    // O rgb_matrix_indicators_user será chamado automaticamente e verificará o estado atual
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void numlock_init_early_hooks(void) {
    event_bus_subscribe_rgb_indicators(numlock_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void numlock_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, numlock_keyboard_post_init_user);
}

