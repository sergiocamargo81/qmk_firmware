#include "unused.h"

#include "../kind.h"
#include "../behavior.h"
#include "../event_bus.h"  // Para Event Bus
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"
#include "rgb_matrix.h"

// ===== Funções Auxiliares =====

// Callback para atualizar LED de uma tecla unused
static bool unused_update_led_callback(unused_t* unused, void* user_data) {
    uint8_t brightness = *(uint8_t*)user_data;

    // Garante que o estado está inicializado (se não estiver, assume IDLE)
    if (unused->state != UNUSED_IDLE && unused->state != UNUSED_PRESSED) {
        unused->state = UNUSED_IDLE;
    }

    // Verifica se a tecla está pressionada usando state
    if (unused->state == UNUSED_PRESSED) {
        // Tecla está pressionada: verde contínuo
        color_rgb_t green = color_get_rgb(COLOR_GREEN);
        rgb_matrix_set_color(unused->led_index, green.r, green.g, green.b);
    } else {
        // Tecla não está pressionada: branco pulsante
        color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
        rgb_matrix_set_color(unused->led_index, white.r, white.g, white.b);
    }

    return true; // Continua iteração
}

// ===== Processamento de Eventos =====

// Processa eventos de teclas unused
bool unused_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (key == NULL || key->kind != KIND_UNUSED) {
        return true;
    }

    // Atualiza estado na matrix
    unused_t* unused = (unused_t*)key;
    unused->state = pressed ? UNUSED_PRESSED : UNUSED_IDLE;

    // Não consome o evento, permite que seja processado normalmente
    return true;
}

// ===== Funções de Hook QMK =====

// Hook rgb_matrix_indicators_user
// Atualiza LEDs de todas as teclas unused com pulsação em branco
static void unused_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    // Calcula brilho pulsante (0-255)
    uint8_t brightness = calculate_pulse_brightness();

    // Itera sobre todas as teclas unused e atualiza seus LEDs
    kind_iterate_unused(unused_update_led_callback, &brightness);
}

// ===== Inicialização =====

// Callback para inicializar estados e registrar handlers
static bool unused_init_callback(unused_t* unused, void* user_data) {
    (void)user_data;
    if (unused != NULL) {
        unused->state = UNUSED_IDLE;
        // Registra handler para esta posição
        behavior_register_position_function(unused->row, unused->col, unused_process_key);
    }
    return true;
}

// Inicializa estados e registra handlers
static void unused_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;
    // Itera sobre todas as teclas unused e inicializa estados
    kind_iterate_unused(unused_init_callback, NULL);
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void unused_init_early_hooks(void) {
    event_bus_subscribe_rgb_indicators(unused_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void unused_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, unused_keyboard_post_init_user);
}