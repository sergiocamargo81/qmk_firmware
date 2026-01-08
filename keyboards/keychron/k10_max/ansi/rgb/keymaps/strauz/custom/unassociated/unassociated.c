#include "unassociated.h"

#include "../../behavior.h"
#include "../../kind.h"
#include "../../custom_behaviors.h"  // Para custom_behaviors_t
#include "../custom.h"
#include "../../event_bus.h"  // Para Event Bus
#include "../../keymod.h"  // Para keymod_t
#include "../../colors.h"  // Para cores centralizadas
#include "../../pulse.h"
#include "rgb_matrix.h"

// Declarações forward
bool unassociated_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// ===== Estado de FN =====

// Rastreia keymod atual (para mudar cor das teclas em UNASSOCIATED_IDLE)
static keymod_t unassociated_fn_keymod = KEYMOD_NONE;

// ===== Funções Auxiliares =====

// Callback para notificar mudança de estado de FN
void unassociated_fn_state_callback(const event_t* event) {
    if (event->type != EVENT_FN_STATE_CHANGED) return;
    unassociated_fn_keymod = (keymod_t)event->data.fn_state_changed.keymod;
}

// Handler para notificar inclusão de tecla no controle de unassociated (via Event Bus)
static void unassociated_custom_behavior_added(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_ADDED) return;
    if (event->data.custom_behavior.behavior != CUSTOM_BEHAVIOR_UNASSOCIATED) return;
    
    uint8_t row = event->data.custom_behavior.row;
    uint8_t col = event->data.custom_behavior.col;
    custom_t* custom = unassociated_get_key_by_position(row, col);
    if (custom) {
        // Inicializa estado apenas para esta tecla
        custom->state = UNASSOCIATED_IDLE;
        // Atualiza o LED imediatamente após a associação
        // O LED ficará branco pulsante (UNASSOCIATED_IDLE)
        // A pulsação será aplicada em unassociated_update_indicators()
        uint8_t brightness = calculate_pulse_brightness();
        color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
        rgb_matrix_set_color(custom->led_index, white.r, white.g, white.b);
    }
}

// Handler para notificar remoção de tecla do controle de unassociated (via Event Bus)
static void unassociated_custom_behavior_removed(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_REMOVED) return;
    if (event->data.custom_behavior.behavior != CUSTOM_BEHAVIOR_UNASSOCIATED) return;
    
    uint8_t row = event->data.custom_behavior.row;
    uint8_t col = event->data.custom_behavior.col;
    custom_t* custom = unassociated_get_key_by_position(row, col);
    if (custom) {
        // Limpa estado - a tecla será recolorida quando associada a outro módulo
        custom->state = UNASSOCIATED_IDLE;
    }
}

// ===== Funções de Estado =====
custom_t* unassociated_get_key_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é UNASSOCIATED usando o cache
    if (custom->custom_behavior != CUSTOM_BEHAVIOR_UNASSOCIATED) return NULL;
    return custom;
}

custom_t* unassociated_get_key_by_index(uint8_t persist_index) {
    custom_t* custom = kind_get_custom_by_index(persist_index);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é UNASSOCIATED usando o cache
    if (custom->custom_behavior != CUSTOM_BEHAVIOR_UNASSOCIATED) return NULL;
    return custom;
}

void unassociated_update_indicators(void) {
    // Atualiza apenas os LEDs das teclas que estão associadas a UNASSOCIATED
    // Verifica diretamente o behavior atual para garantir que apenas teclas realmente associadas sejam atualizadas
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se esta tecla está realmente associada a UNASSOCIATED usando o cache
        if (custom->custom_behavior == CUSTOM_BEHAVIOR_UNASSOCIATED) {
            // Esta tecla está associada a UNASSOCIATED
            // Atualiza LED baseado no estado
            switch (custom->state) {
                case UNASSOCIATED_PRESSED: {
                    // Tecla está pressionada: verde contínuo
                    color_rgb_t green = color_get_rgb(COLOR_GREEN);
                    rgb_matrix_set_color(custom->led_index, green.r, green.g, green.b);
                    break;
                }
                case UNASSOCIATED_IDLE:
                default: {
                    // Estado IDLE: branco pulsante por padrão
                    uint8_t brightness = calculate_pulse_brightness();
                    color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
                    
                    // Verifica se FN está pressionado para alternar cores
                    if (keymod_equals(unassociated_fn_keymod, KEYMOD_FN_ONLY)) {
                        // FN pressionado: pulsar rapidamente entre branco e amarelo
                        uint8_t pulse_value = calculate_dual_color_pulse();
                        color_rgb_t yellow = color_apply_brightness(COLOR_YELLOW, brightness);
                        // Interpola entre branco (pulse_value = 0) e amarelo (pulse_value = 255)
                        color_rgb_t mixed;
                        mixed.r = white.r + ((yellow.r - white.r) * pulse_value) / 255;
                        mixed.g = white.g + ((yellow.g - white.g) * pulse_value) / 255;
                        mixed.b = white.b + ((yellow.b - white.b) * pulse_value) / 255;
                        rgb_matrix_set_color(custom->led_index, mixed.r, mixed.g, mixed.b);
                    } else if (keymod_equals(unassociated_fn_keymod, KEYMOD_FN_RCTL)) {
                        // FN+RCTRL: pulsar rapidamente entre branco e azul
                        uint8_t pulse_value = calculate_dual_color_pulse();
                        color_rgb_t blue = color_apply_brightness(COLOR_BLUE, brightness);
                        // Interpola entre branco (pulse_value = 0) e azul (pulse_value = 255)
                        color_rgb_t mixed;
                        mixed.r = white.r + ((blue.r - white.r) * pulse_value) / 255;
                        mixed.g = white.g + ((blue.g - white.g) * pulse_value) / 255;
                        mixed.b = white.b + ((blue.b - white.b) * pulse_value) / 255;
                        rgb_matrix_set_color(custom->led_index, mixed.r, mixed.g, mixed.b);
                    } else if (keymod_equals(unassociated_fn_keymod, KEYMOD_FN_RALT)) {
                        // FN+RALT: pulsar rapidamente entre branco e verde
                        uint8_t pulse_value = calculate_dual_color_pulse();
                        color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
                        // Interpola entre branco (pulse_value = 0) e verde (pulse_value = 255)
                        color_rgb_t mixed;
                        mixed.r = white.r + ((green.r - white.r) * pulse_value) / 255;
                        mixed.g = white.g + ((green.g - white.g) * pulse_value) / 255;
                        mixed.b = white.b + ((green.b - white.b) * pulse_value) / 255;
                        rgb_matrix_set_color(custom->led_index, mixed.r, mixed.g, mixed.b);
                    } else {
                        // FN não está pressionada: branco pulsante (IDLE)
                        rgb_matrix_set_color(custom->led_index, white.r, white.g, white.b);
                    }
                    break;
                }
            }
        }
    }
}

// ===== Funções de Hook QMK =====

// Processa eventos de tecla para unassociated
bool unassociated_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (key == NULL || key->kind != KIND_CUSTOM) {
        return true;
    }
    
    custom_t* custom = (custom_t*)key;
    
    // Verifica se o behavior é UNASSOCIATED usando o cache
    if (custom->custom_behavior == CUSTOM_BEHAVIOR_UNASSOCIATED) {
        // Atualiza estado
        if (pressed) {
            custom->state = UNASSOCIATED_PRESSED;
        } else {
            custom->state = UNASSOCIATED_IDLE;
        }
        // Não consome o evento, permite que seja processado normalmente
        return true;
    }

    return true;
}

// Hook rgb_matrix_indicators_user
static void unassociated_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    // Sempre atualiza indicadores para todas as teclas com behavior UNASSOCIATED
    unassociated_update_indicators();
}

// Hook keyboard_post_init_user
static void unassociated_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;
    
    // Registra handler para processamento de teclas
    custom_register_handler(CUSTOM_BEHAVIOR_UNASSOCIATED, unassociated_process_key);
    
    // Registra handlers para eventos ADDED/REMOVED via Event Bus
    event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_ADDED, unassociated_custom_behavior_added);
    event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_REMOVED, unassociated_custom_behavior_removed);
    
    // Estado de FN será inicializado como KEYMOD_NONE
    // Será atualizado via notificação quando FN for pressionado
    unassociated_fn_keymod = KEYMOD_NONE;
    
    // Força chamada de add para teclas que já têm o behavior no profile ativo
    // Isso garante que os LEDs sejam ligados na inicialização
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se o behavior é UNASSOCIATED usando o cache
        if (custom->custom_behavior == CUSTOM_BEHAVIOR_UNASSOCIATED) {
            // Dispara evento ADDED para inicializar LED e estado
            event_data_t data = {
                .custom_behavior = {
                    .row = custom->row,
                    .col = custom->col,
                    .behavior = CUSTOM_BEHAVIOR_UNASSOCIATED
                }
            };
            event_bus_publish(EVENT_CUSTOM_BEHAVIOR_ADDED, &data);
        }
    }
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void unassociated_init_early_hooks(void) {
    event_bus_subscribe_rgb_indicators(unassociated_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void unassociated_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, unassociated_keyboard_post_init_user);
}
