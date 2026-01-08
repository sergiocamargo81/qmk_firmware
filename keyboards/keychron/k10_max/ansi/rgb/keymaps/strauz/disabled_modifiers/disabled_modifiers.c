#include "disabled_modifiers.h"

#include "../kind.h"
#include "../behavior.h"
#include "../colors.h"
#include "../pulse.h"
#include "../event_bus.h"  // Para Event Bus
#include "../keymod.h"
#include "rgb_matrix.h"

// ===== Defines =====
typedef uint8_t disabled_modifiers_state_t;
enum {
    DISABLED_MODIFIERS_IDLE,
    DISABLED_MODIFIERS_PRESSED
};

// ===== Variáveis Globais =====

static bool disabled_modifiers_initialized = false;

// ===== Funções Auxiliares =====

// Callback para atualizar LED de um disabled_modifier
static bool disabled_modifiers_update_led_callback(others_t* disabled_modifier, void* user_data) {
    (void)user_data;
    if (disabled_modifier == NULL) return true;
    
    // Todos os disabled_modifiers: roxo contínuo sempre (não muda quando pressionada)
    color_rgb_t purple = color_get_rgb(COLOR_PURPLE);
    rgb_matrix_set_color(disabled_modifier->led_index, purple.r, purple.g, purple.b);
    
    return true; // Continua iteração
}

// Atualiza LEDs dos disabled_modifiers
static void update_disabled_modifiers_leds(void) {
    if (!disabled_modifiers_initialized) {
        return;
    }
    
    // Itera sobre todos os disabled_modifiers e atualiza LEDs
    kind_iterate_disabled_modifiers(disabled_modifiers_update_led_callback, NULL);
}

// ===== Inicialização =====

// Callback para inicializar estados e registrar handlers
static bool disabled_modifiers_init_callback(others_t* disabled_modifier, void* user_data) {
    (void)user_data;
    if (disabled_modifier == NULL) return true;
    
    // Inicializa estado na matrix
    disabled_modifier->state = DISABLED_MODIFIERS_IDLE;
    
    // Registra handler para esta posição
    behavior_register_position_function(disabled_modifier->row, disabled_modifier->col, disabled_modifiers_process_key);
    
    return true; // Continua iteração
}

void disabled_modifiers_init(void) {
    // Itera sobre todos os disabled_modifiers e inicializa estados e registra handlers
    kind_iterate_disabled_modifiers(disabled_modifiers_init_callback, NULL);
    
    disabled_modifiers_initialized = true;
}

// ===== Processamento de Eventos =====

// Processa eventos de disabled_modifiers
// Retorna false se consumiu o evento, true caso contrário
bool disabled_modifiers_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (!disabled_modifiers_initialized) {
        return true;
    }
    
    if (key == NULL || key->kind != KIND_DISABLED_MODIFIERS) {
        return true;
    }
    
    // Atualiza estado na matrix baseado no evento
    others_t* disabled_modifier = (others_t*)key;
    disabled_modifier->state = pressed ? DISABLED_MODIFIERS_PRESSED : DISABLED_MODIFIERS_IDLE;
    
    // Não consome o evento, permite que seja processado normalmente
    return true;
}

// Hook rgb_matrix_indicators_user: atualiza LEDs dos disabled_modifiers
static void disabled_modifiers_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    update_disabled_modifiers_leds();
}

// ===== Registro de Hooks =====

void disabled_modifiers_init_hooks(void) {
    // Registra hook de rgb_indicators para atualizar LEDs
    event_bus_subscribe_rgb_indicators(disabled_modifiers_rgb_matrix_indicators_user);
}
