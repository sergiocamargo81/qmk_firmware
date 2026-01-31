#include "unused_modifiers.h"

#include "../kind.h"
#include "../behavior.h"
#include "../colors.h"
#include "../pulse.h"
#include "../event_bus.h"  // Para Event Bus
#include "../keymod.h"
#include "rgb_matrix.h"

// ===== Defines =====
typedef uint8_t unused_modifiers_state_t;
enum {
    UNUSED_MODIFIERS_IDLE,
    UNUSED_MODIFIERS_PRESSED
};

// ===== Variáveis Globais =====

static bool unused_modifiers_initialized = false;

// ===== Funções Auxiliares =====

// Callback para atualizar LED de um unused_modifier
static bool unused_modifiers_update_led_callback(unused_modifier_t* unused_modifier, void* user_data) {

    // Verifica se a tecla está pressionada usando state
    if (unused_modifier->state == UNUSED_MODIFIERS_PRESSED) {
        // Tecla está pressionada: verde contínuo
        color_rgb_t green = color_get_rgb(COLOR_GREEN);
        rgb_matrix_set_color(unused_modifier->led_index, green.r, green.g, green.b);
    } else {
        // Tecla não está pressionada: roxo pulsante
        uint8_t brightness = calculate_pulse_brightness();
        color_rgb_t purple = color_apply_brightness(COLOR_PURPLE, brightness);
        rgb_matrix_set_color(unused_modifier->led_index, purple.r, purple.g, purple.b);
    }

    return true; // Continua iteração
}

// Atualiza LEDs dos unused_modifiers
static void update_unused_modifiers_leds(void) {
    if (!unused_modifiers_initialized) {
        return;
    }

    // Itera sobre todos os unused_modifiers e atualiza LEDs
    kind_iterate_unused_modifiers(unused_modifiers_update_led_callback, NULL);
}

// ===== Inicialização =====

// Callback para inicializar estados e registrar handlers
static bool unused_modifiers_init_callback(unused_modifier_t* unused_modifier, void* user_data) {
    (void)user_data;
    if (unused_modifier == NULL) return true;

    // Inicializa estado na matrix
    unused_modifier->state = UNUSED_MODIFIERS_IDLE;

    // Registra handler para esta posição
    behavior_register_position_function(unused_modifier->row, unused_modifier->col, unused_modifiers_process_key);

    return true; // Continua iteração
}

void unused_modifiers_init(void) {
    // Itera sobre todos os unused_modifiers e inicializa estados e registra handlers
    kind_iterate_unused_modifiers(unused_modifiers_init_callback, NULL);

    unused_modifiers_initialized = true;
}

// ===== Processamento de Eventos =====

// Processa eventos de unused_modifiers
// Retorna false se consumiu o evento, true caso contrário
bool unused_modifiers_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (!unused_modifiers_initialized) {
        return true;
    }

    if (key == NULL || key->kind != KIND_UNUSED_MODIFIERS) {
        return true;
    }

    // Atualiza estado na matrix baseado no evento
    unused_modifier_t* unused_modifier = (unused_modifier_t*)key;
    unused_modifier->state = pressed ? UNUSED_MODIFIERS_PRESSED : UNUSED_MODIFIERS_IDLE;

    // Não consome o evento, permite que seja processado normalmente
    return true;
}

// Hook rgb_matrix_indicators_user: atualiza LEDs dos unused_modifiers
static void unused_modifiers_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    update_unused_modifiers_leds();
}

// ===== Registro de Hooks =====

void unused_modifiers_init_hooks(void) {
    // Registra hook de rgb_indicators para atualizar LEDs
    event_bus_subscribe_rgb_indicators(unused_modifiers_rgb_matrix_indicators_user);
}