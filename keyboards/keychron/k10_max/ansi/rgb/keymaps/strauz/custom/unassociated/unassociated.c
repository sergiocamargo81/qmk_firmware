#include "unassociated.h"

#include "../../behavior.h"
#include "../../kind.h"
#include "../../customs.h"  // Para customs_t
#include "../custom.h"
#include "../../hooks.h"
#include "../../keymod.h"  // Para keymod_t
#include "../../colors.h"  // Para cores centralizadas
#include "../../pulse.h"
#include "rgb_matrix.h"

// Declarações forward
bool unassociated_process_record_user(keyrecord_t *record, keymod_t keymod);
void unassociated_key_add_callback(uint8_t row, uint8_t col, customs_t behavior);
void unassociated_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior);

// ===== Estado de FN =====

// Rastreia keymod atual (para mudar cor das teclas em UNASSOCIATED_IDLE)
static keymod_t unassociated_fn_keymod = KEYMOD_NONE;

// ===== Funções Auxiliares =====

// Callback para notificar mudança de estado de FN
static void unassociated_fn_state_callback(keymod_t keymod) {
    unassociated_fn_keymod = keymod;
}

// Função pública para obter o callback de notificação de FN
unassociated_fn_state_callback_t unassociated_get_fn_callback(void) {
    return unassociated_fn_state_callback;
}

// Callback para notificar inclusão de tecla no controle de unassociated
void unassociated_key_add_callback(uint8_t row, uint8_t col, customs_t behavior) {
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

// Callback para notificar remoção de tecla do controle de unassociated
void unassociated_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior) {
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
    // Verifica se o behavior é UNASSOCIATED
    customs_t behavior = custom_get_behavior_by_position(row, col);
    if (behavior != KEY_CUSTOM_UNASSOCIATED) return NULL;
    return custom;
}

custom_t* unassociated_get_key_by_index(uint8_t persist_index) {
    custom_t* custom = kind_get_custom_by_index(persist_index);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é UNASSOCIATED
    customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
    if (behavior != KEY_CUSTOM_UNASSOCIATED) return NULL;
    return custom;
}

void unassociated_update_indicators(void) {
    // Atualiza apenas os LEDs das teclas que estão associadas a UNASSOCIATED
    // Verifica diretamente o behavior atual para garantir que apenas teclas realmente associadas sejam atualizadas
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se esta tecla está realmente associada a UNASSOCIATED
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior == KEY_CUSTOM_UNASSOCIATED) {
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
                    if (keymod_is_only_value(unassociated_fn_keymod, KEYMOD_FN_ONLY)) {
                        // FN pressionado: pulsar rapidamente entre branco e amarelo
                        uint8_t pulse_value = calculate_dual_color_pulse();
                        color_rgb_t yellow = color_apply_brightness(COLOR_YELLOW, brightness);
                        // Interpola entre branco (pulse_value = 0) e amarelo (pulse_value = 255)
                        color_rgb_t mixed;
                        mixed.r = white.r + ((yellow.r - white.r) * pulse_value) / 255;
                        mixed.g = white.g + ((yellow.g - white.g) * pulse_value) / 255;
                        mixed.b = white.b + ((yellow.b - white.b) * pulse_value) / 255;
                        rgb_matrix_set_color(custom->led_index, mixed.r, mixed.g, mixed.b);
                    } else if (keymod_is_only_value(unassociated_fn_keymod, KEYMOD_FN_RCTL)) {
                        // FN+RCTRL: pulsar rapidamente entre branco e azul
                        uint8_t pulse_value = calculate_dual_color_pulse();
                        color_rgb_t blue = color_apply_brightness(COLOR_BLUE, brightness);
                        // Interpola entre branco (pulse_value = 0) e azul (pulse_value = 255)
                        color_rgb_t mixed;
                        mixed.r = white.r + ((blue.r - white.r) * pulse_value) / 255;
                        mixed.g = white.g + ((blue.g - white.g) * pulse_value) / 255;
                        mixed.b = white.b + ((blue.b - white.b) * pulse_value) / 255;
                        rgb_matrix_set_color(custom->led_index, mixed.r, mixed.g, mixed.b);
                    } else if (keymod_is_only_value(unassociated_fn_keymod, KEYMOD_FN_RALT)) {
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
bool unassociated_process_record_user(keyrecord_t *record, keymod_t keymod) {
    // Obtém row/col do record (mais eficiente que usar keycode)
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // Busca a tecla unassociated por posição (O(1))
    custom_t* custom = unassociated_get_key_by_position(row, col);
    
    if (custom) {
        // Verifica se o behavior é UNASSOCIATED usando custom
        customs_t behavior = custom_get_behavior_by_position(row, col);
        if (behavior == KEY_CUSTOM_UNASSOCIATED) {
            // Atualiza estado
            if (record->event.pressed) {
                custom->state = UNASSOCIATED_PRESSED;
            } else {
                custom->state = UNASSOCIATED_IDLE;
            }
            // Não consome o evento, permite que seja processado normalmente
            return true;
        }
    }

    return true;
}

// Hook rgb_matrix_indicators_user
static bool unassociated_rgb_matrix_indicators_user(void) {
    // Sempre atualiza indicadores para todas as teclas com behavior UNASSOCIATED
    unassociated_update_indicators();
    return true;
}

// Hook keyboard_post_init_user
static void unassociated_keyboard_post_init_user(void) {
    // Registra callbacks para KEY_CUSTOM_UNASSOCIATED
    custom_callbacks_t callbacks = {
        .key_handler = unassociated_process_record_user,
        .add_callback = unassociated_key_add_callback,
        .remove_callback = unassociated_key_remove_callback
    };
    custom_register_callbacks(KEY_CUSTOM_UNASSOCIATED, callbacks);
    
    // Estado de FN será inicializado como KEYMOD_NONE
    // Será atualizado via notificação quando FN for pressionado
    unassociated_fn_keymod = KEYMOD_NONE;
    
    // Força chamada de add_callback para teclas que já têm o behavior no profile ativo
    // Isso garante que os LEDs sejam ligados na inicialização
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior == KEY_CUSTOM_UNASSOCIATED) {
            // Chama add_callback para inicializar LED e estado
            unassociated_key_add_callback(custom->row, custom->col, KEY_CUSTOM_UNASSOCIATED);
        }
    }
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void unassociated_init_early_hooks(void) {
    hooks_rgb_indicators_register(unassociated_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void unassociated_init_hooks(void) {
    hooks_keyboard_post_init_register(unassociated_keyboard_post_init_user);
}
