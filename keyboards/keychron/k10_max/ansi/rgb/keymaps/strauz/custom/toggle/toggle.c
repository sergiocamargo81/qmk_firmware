#include "toggle.h"

#include "../../behavior.h"
#include "../../kind.h"
#include "../../customs.h"  // Para customs_t
#include "../../profile.h"
#include "../../settings.h"
#include "../custom.h"
#include "../../hooks.h"
#include "../../keymod.h"  // Para keymod_t
#include "../hold/hold.h"  // Para deactivate_all_hold
#include "../../colors.h"  // Para cores centralizadas
#include "../../pulse.h"
#include <string.h>  // Para memset

// Declarações forward
bool toggle_process_record_user(keyrecord_t *record, keymod_t keymod);
void toggle_key_add_callback(uint8_t row, uint8_t col, customs_t behavior);
void toggle_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior);

// ===== Variáveis Globais =====

bool toggle_profile_mode = true;
uint8_t toggle_enabled_keys_bitfield[6] = {0};

// ===== Estado de FN =====

// Rastreia keymod atual (para mudar cor das teclas em TOGGLE_OFF)
static keymod_t toggle_fn_keymod = KEYMOD_NONE;

// ===== Funções Auxiliares =====
static bool is_key_enabled(custom_t *custom) {
    if (!custom) return false;
    // Verifica diretamente o behavior atual em vez de depender do bitfield
    customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
    return (behavior == KEY_CUSTOM_TOGGLE);
}

static bool profiles_is_empty(profile_t* profile) {
    if (!profile) return true;
    for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
        if (profile->behaviors[i] != KEY_CUSTOM_UNASSOCIATED) {
            return false;
        }
    }
    return true;
}

// Callback para notificar mudança de estado de FN
static void toggle_fn_state_callback(keymod_t keymod) {
    toggle_fn_keymod = keymod;
}

// Função pública para obter o callback de notificação de FN
toggle_fn_state_callback_t toggle_get_fn_callback(void) {
    return toggle_fn_state_callback;
}

// Callback para notificar inclusão de tecla no controle de toggle
void toggle_key_add_callback(uint8_t row, uint8_t col, customs_t behavior) {
    custom_t* custom = toggle_get_key_by_position(row, col);
    if (custom) {
        // Inicializa estado apenas para esta tecla
        custom->state = TOGGLE_OFF;
        // Atualiza o LED imediatamente após a associação
        // O LED ficará branco pulsante (TOGGLE_OFF) ou verde pulsante se FN pressionado
        // A pulsação será aplicada em toggle_update_indicators()
        // Inicializa com cor apropriada baseada no estado de FN
        uint8_t brightness = calculate_pulse_brightness();
        if (keymod_is_only_value(toggle_fn_keymod, KEYMOD_FN_ONLY)) {
            // Apenas FN está pressionada: verde pulsante
            color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
            rgb_matrix_set_color(custom->led_index, green.r, green.g, green.b);
        } else {
            // FN não está pressionada: branco pulsante
            color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
            rgb_matrix_set_color(custom->led_index, white.r, white.g, white.b);
        }
    }
}

// Callback para notificar remoção de tecla do controle de toggle
void toggle_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior) {
    custom_t* custom = toggle_get_key_by_position(row, col);
    if (custom) {
        // Limpa estado - a tecla será recolorida quando associada a outro módulo
        custom->state = TOGGLE_OFF;
    }
}

// ===== Funções de Estado =====
custom_t* toggle_get_key_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é TOGGLE
    customs_t behavior = custom_get_behavior_by_position(row, col);
    if (behavior != KEY_CUSTOM_TOGGLE) return NULL;
    return custom;
}

custom_t* toggle_get_key_by_index(uint8_t persist_index) {
    custom_t* custom = kind_get_custom_by_index(persist_index);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é TOGGLE
    customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
    if (behavior != KEY_CUSTOM_TOGGLE) return NULL;
    return custom;
}

void toggle_update_states(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se o behavior é TOGGLE
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior != KEY_CUSTOM_TOGGLE) continue;
        
        if (custom->state == TOGGLE_OFF) continue;
        if (custom->state == TOGGLE_ON) {
            if (timer_elapsed32(custom->timer) >= CUSTOM_INTERVAL) {
                tap_code(custom->keycode);
                custom->timer = now;
            }
        }
    }
}

void toggle_activate_key(custom_t *custom) {
    if (!custom) return;
    uint32_t now = timer_read32();
    // Determina o estado anterior (ignora TOGGLE_PRESSED)
    toggle_state_t prev_state = (custom->state == TOGGLE_PRESSED) ? TOGGLE_OFF : custom->state;
    
    // Alterna o estado
    if (prev_state == TOGGLE_OFF) {
        custom->state = TOGGLE_ON;
        custom->timer = now;
        tap_code(custom->keycode);
    } else if (prev_state == TOGGLE_ON) {
        custom->state = TOGGLE_OFF;
    }
}

void toggle_deactivate_key(custom_t *custom) {
    // Não faz nada no release, o toggle é controlado apenas no press
    (void)custom;  // Evita warning de parâmetro não usado
}

void toggle_deactivate_all(void) {
    // Desativa todas as teclas toggle que estão em estado TOGGLE_ON
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se o behavior é TOGGLE
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior != KEY_CUSTOM_TOGGLE) continue;
        
        if (custom->state == TOGGLE_ON) {
            custom->state = TOGGLE_OFF;
        }
    }
}

void toggle_sync_enabled_states(void) {
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        bool is_enabled = is_key_enabled(custom);
        if (!is_enabled) custom->state = TOGGLE_OFF;
    }
}


void toggle_update_indicators(void) {
    // Atualiza apenas os LEDs das teclas que estão associadas a TOGGLE
    // Verifica diretamente o behavior atual para garantir que apenas teclas realmente associadas sejam atualizadas
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se esta tecla está realmente associada a TOGGLE
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior == KEY_CUSTOM_TOGGLE) {
            // Esta tecla está associada a TOGGLE
            // Atualiza LED baseado no estado
            switch (custom->state) {
                case TOGGLE_PRESSED: {
                    // Tecla está pressionada: verde contínuo (transitório, logo alterna para ON/OFF)
                    color_rgb_t green = color_get_rgb(COLOR_GREEN);
                    rgb_matrix_set_color(custom->led_index, green.r, green.g, green.b);
                    break;
                }
                case TOGGLE_OFF: {
                    // Estado OFF: verde pulsante
                    uint8_t brightness = calculate_pulse_brightness();
                    color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
                    rgb_matrix_set_color(custom->led_index, green.r, green.g, green.b);
                    break;
                }
                case TOGGLE_ON: {
                    // Estado ON: vermelho contínuo
                    color_rgb_t red = color_get_rgb(COLOR_RED);
                    rgb_matrix_set_color(custom->led_index, red.r, red.g, red.b);
                    break;
                }
            }
        }
    }
}

bool toggle_is_profile_mode_active(void) {
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile || profiles_is_empty(active_profile)) {
        return false;
    }
    return toggle_profile_mode;
}

void toggle_reset_all_keys(void) {
    memset(toggle_enabled_keys_bitfield, 0, sizeof(toggle_enabled_keys_bitfield));
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom) {
            custom->state = TOGGLE_OFF;
        }
    }
}

// ===== Funções de Hook QMK =====

// Processa eventos de tecla para toggle
bool toggle_process_record_user(keyrecord_t *record, keymod_t keymod) {
    // Obtém row/col do record (mais eficiente que usar keycode)
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // Busca a tecla toggle por posição (O(1))
    custom_t* custom = toggle_get_key_by_position(row, col);
    
    if (custom) {
        // Verifica se o behavior é TOGGLE usando custom
        customs_t behavior = custom_get_behavior_by_position(row, col);
        if (behavior == KEY_CUSTOM_TOGGLE) {
            if (record->event.pressed) {
                // Salva o estado anterior antes de marcar como PRESSED
                toggle_state_t prev_state = (custom->state == TOGGLE_PRESSED) ? TOGGLE_OFF : custom->state;
                // Marca como pressionado
                custom->state = TOGGLE_PRESSED;
                // Alterna o estado baseado no estado anterior
                uint32_t now = timer_read32();
                if (prev_state == TOGGLE_OFF) {
                    custom->state = TOGGLE_ON;
                    custom->timer = now;
                    tap_code(custom->keycode);
                } else if (prev_state == TOGGLE_ON) {
                    custom->state = TOGGLE_OFF;
                }
            } else {
                // No release, se ainda está em PRESSED (não deveria acontecer), restaura
                if (custom->state == TOGGLE_PRESSED) {
                    custom->state = TOGGLE_OFF;
                }
                // Caso contrário, mantém o estado alternado (TOGGLE_OFF ou TOGGLE_ON)
                toggle_deactivate_key(custom);
            }
            return true;
        }
    }

    return true;
}

// Hook matrix_scan_user
static void toggle_matrix_scan_user(void) {
    toggle_update_states();
}

// Hook rgb_matrix_indicators_user
static bool toggle_rgb_matrix_indicators_user(void) {
    if (!toggle_is_profile_mode_active()) return true;
    toggle_update_indicators();
    return false;
}

// Hook keyboard_post_init_user
static void toggle_keyboard_post_init_user(void) {
    // Registra callbacks para KEY_CUSTOM_TOGGLE
    custom_callbacks_t callbacks = {
        .key_handler = toggle_process_record_user,
        .add_callback = toggle_key_add_callback,
        .remove_callback = toggle_key_remove_callback
    };
    custom_register_callbacks(KEY_CUSTOM_TOGGLE, callbacks);
    
    // Estado de FN será inicializado como KEYMOD_NONE
    // Será atualizado via notificação quando FN for pressionado
    toggle_fn_keymod = KEYMOD_NONE;
    
    // Inicializa outras coisas...
    toggle_sync_enabled_states();
    
    // Força chamada de add_callback para teclas que já têm o behavior no profile ativo
    // Isso garante que os LEDs sejam ligados na inicialização
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior == KEY_CUSTOM_TOGGLE) {
            // Chama add_callback para inicializar LED e estado
            toggle_key_add_callback(custom->row, custom->col, KEY_CUSTOM_TOGGLE);
        }
    }
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void toggle_init_early_hooks(void) {
    hooks_matrix_scan_register(toggle_matrix_scan_user);
    hooks_rgb_indicators_register(toggle_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void toggle_init_hooks(void) {
    hooks_keyboard_post_init_register(toggle_keyboard_post_init_user);
}
