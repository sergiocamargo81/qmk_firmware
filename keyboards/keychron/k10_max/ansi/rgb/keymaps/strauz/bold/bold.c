#include "bold.h"

#include "../kind.h"
#include "../behavior.h"
#include "../event_bus.h"  // Para Event Bus
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
        // Tecla não está pressionada: verde pulsante
        color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
        rgb_matrix_set_color(bold->led_index, green.r, green.g, green.b);
    }
    
    return true; // Continua iteração
}

// ===== Processamento de Eventos =====

// Processa eventos de teclas bold
bool bold_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (key == NULL || key->kind != KIND_BOLD) {
        return true;
    }
    
    // Atualiza estado na matrix
    bold_t* bold = (bold_t*)key;
    bold->state = pressed ? BOLD_PRESSED : BOLD_IDLE;
    
    // Não consome o evento, permite que seja processado normalmente
    return true;
}

// ===== Funções de Hook QMK =====

// Hook rgb_matrix_indicators_user
// Atualiza LEDs de todas as teclas bold com pulsação em laranja
static void bold_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    // Calcula brilho pulsante (0-255)
    uint8_t brightness = calculate_pulse_brightness();
    
    // Itera sobre todas as teclas bold e atualiza seus LEDs
    kind_iterate_bold(bold_update_led_callback, &brightness);
}

// ===== Inicialização =====

// Callback para inicializar estados e registrar handlers
static bool bold_init_callback(bold_t* bold, void* user_data) {
    (void)user_data;
    if (bold != NULL) {
        bold->state = BOLD_IDLE;
        // Registra handler para esta posição
        behavior_register_position_function(bold->row, bold->col, bold_process_key);
    }
    return true;
}

// Inicializa estados e registra handlers
static void bold_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;
    // Itera sobre todas as teclas bold e inicializa estados
    kind_iterate_bold(bold_init_callback, NULL);
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void bold_init_early_hooks(void) {
    event_bus_subscribe_rgb_indicators(bold_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void bold_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, bold_keyboard_post_init_user);
}
