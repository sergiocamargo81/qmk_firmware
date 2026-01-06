#include "hold.h"

#include "../../behavior.h"
#include "../../kind.h"
#include "../../customs.h"  // Para customs_t
#include "../../profile.h"
#include "../../settings.h"
#include "../custom.h"
#include "../../hooks.h"
#include "../../keymod.h"  // Para keymod_t
#include "../../colors.h"  // Para cores centralizadas
#include "../../pulse.h"
#include <lib/lib8tion/lib8tion.h> // Para scale8
#include <string.h>  // Para memset

// Declarações forward
bool hold_process_record_user(keyrecord_t *record, keymod_t keymod);
void hold_key_add_callback(uint8_t row, uint8_t col, customs_t behavior);
void hold_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior);

// ===== Variáveis Globais =====

bool hold_profile_mode = true;
uint8_t hold_enabled_keys_bitfield[6] = {0};

// ===== Estado de FN =====

// Rastreia keymod atual (para mudar cor das teclas em WAITING)
static keymod_t hold_fn_keymod = KEYMOD_NONE;

// ===== Funções Auxiliares =====
static bool is_key_enabled(custom_t *custom) {
    if (!custom) return false;
    // Verifica diretamente o behavior atual em vez de depender do bitfield
    customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
    return (behavior == KEY_CUSTOM_HOLD);
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
static void hold_fn_state_callback(keymod_t keymod) {
    hold_fn_keymod = keymod;
}

// Função pública para obter o callback de notificação de FN
hold_fn_state_callback_t hold_get_fn_callback(void) {
    return hold_fn_state_callback;
}

// Callback para notificar inclusão de tecla no controle de hold
void hold_key_add_callback(uint8_t row, uint8_t col, customs_t behavior) {
    custom_t* custom = hold_get_key_by_position(row, col);
    if (custom) {
        // Inicializa estado apenas para esta tecla
        custom->state = WAITING;
        // Atualiza o LED imediatamente após a associação
        // O LED ficará branco pulsante (WAITING) ou azul pulsante se FN pressionado
        // A pulsação será aplicada em update_indicators()
        // Inicializa com cor apropriada baseada no estado de FN
        uint8_t brightness = calculate_pulse_brightness();
        if (keymod_is_only_value(hold_fn_keymod, KEYMOD_FN_ONLY)) {
            // Apenas FN está pressionada: azul pulsante
            color_rgb_t blue = color_apply_brightness(COLOR_BLUE, brightness);
            rgb_matrix_set_color(custom->led_index, blue.r, blue.g, blue.b);
        } else {
            // FN não está pressionada: branco pulsante
            color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
            rgb_matrix_set_color(custom->led_index, white.r, white.g, white.b);
        }
    }
}

// Callback para notificar remoção de tecla do controle de hold
void hold_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior) {
    custom_t* custom = hold_get_key_by_position(row, col);
    if (custom) {
        // Limpa estado - a tecla será recolorida quando associada a outro módulo
        custom->state = DISABLED;
    }
}

// ===== Funções de Estado =====
custom_t* hold_get_key_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é HOLD
    customs_t behavior = custom_get_behavior_by_position(row, col);
    if (behavior != KEY_CUSTOM_HOLD) return NULL;
    return custom;
}

custom_t* hold_get_key_by_index(uint8_t persist_index) {
    custom_t* custom = kind_get_custom_by_index(persist_index);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é HOLD
    customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
    if (behavior != KEY_CUSTOM_HOLD) return NULL;
    return custom;
}

void update_hold_states(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se o behavior é HOLD
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior != KEY_CUSTOM_HOLD) continue;
        
        if (custom->state == DISABLED) continue;
        switch (custom->state) {
            case PRESSING: {
                if (timer_elapsed32(custom->timer) >= CUSTOM_DELAY) {
                    custom->state = FIRING; custom->timer = now; tap_code(custom->keycode);
                }
                break;
            }
            case FIRING: {
                if (timer_elapsed32(custom->timer) >= CUSTOM_INTERVAL) {
                    tap_code(custom->keycode); custom->timer = now;
                }
                break;
            }
            case RESTING: {
                if (timer_elapsed32(custom->timer) >= LED_DEACTIVATE_TIME) custom->state = WAITING;
                break;
            }
            default: break;
        }
    }
}

void activate_hold_key(custom_t *custom) {
    if (!custom) return;
    uint32_t now = timer_read32();
    custom->state = PRESSING; custom->timer = now;
}

void deactivate_hold_key(custom_t *custom) {
    if (!custom) return;
    uint32_t now = timer_read32();
    if (custom->state == FIRING) { custom->state = RESTING; custom->timer = now; return; }
    if (custom->state == PRESSING) { custom->state = WAITING; return; }
}

void deactivate_all_hold(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se o behavior é HOLD
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior != KEY_CUSTOM_HOLD) continue;
        
        if (custom->state == FIRING) { custom->state = RESTING; custom->timer = now; }
    }
}

void sync_enabled_states(void) {
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        bool is_enabled = is_key_enabled(custom);
        if (!is_enabled) {
            custom->state = DISABLED;
        } else {
            // Se está habilitado e estava desabilitado, inicializa como WAITING
            if (custom->state == DISABLED) {
                custom->state = WAITING;
            }
            // Se já estava em outro estado (WAITING, PRESSING, etc), mantém o estado
        }
    }
}


void update_indicators(void) {
    // Atualiza apenas os LEDs das teclas que estão associadas a HOLD
    // Verifica diretamente o behavior atual para garantir que apenas teclas realmente associadas sejam atualizadas
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se esta tecla está realmente associada a HOLD
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior == KEY_CUSTOM_HOLD) {
            // Esta tecla está associada a HOLD
            // Atualiza LED baseado no estado
            if (custom->state != DISABLED) {
                switch (custom->state) {
                        case WAITING: {
                            // Estado inicial: azul pulsante
                            uint8_t brightness = calculate_pulse_brightness();
                            color_rgb_t blue = color_apply_brightness(COLOR_BLUE, brightness);
                            rgb_matrix_set_color(custom->led_index, blue.r, blue.g, blue.b);
                            break;
                        }
                        case PRESSING: {
                            // Tecla está pressionada: verde contínuo
                            color_rgb_t green = color_get_rgb(COLOR_GREEN);
                            rgb_matrix_set_color(custom->led_index, green.r, green.g, green.b);
                            break;
                        }
                        case FIRING: {
                            // Disparo: vermelho contínuo
                            color_rgb_t red = color_get_rgb(COLOR_RED);
                            rgb_matrix_set_color(custom->led_index, red.r, red.g, red.b);
                            break;
                        }
                        case RESTING: {
                            // Após soltar: amarelo contínuo por um tempo
                            color_rgb_t yellow = color_get_rgb(COLOR_YELLOW);
                            rgb_matrix_set_color(custom->led_index, yellow.r, yellow.g, yellow.b);
                            break;
                        }
                        case DISABLED: {
                            // Estado DISABLED - não renderiza, será recolorido quando associado a outro módulo
                            break;
                        }
                    }
            } else {
                // Estado é DISABLED - não renderiza, será recolorido quando associado a outro módulo
            }
        }
    }
}

bool is_profile_mode_active(void) {
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile || profiles_is_empty(active_profile)) {
        return false;
    }
    return hold_profile_mode;
}

void reset_all_enabled_keys(void) {
    memset(hold_enabled_keys_bitfield, 0, sizeof(hold_enabled_keys_bitfield));
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom) {
            custom->state = DISABLED;
        }
    }
}

// ===== Funções de Hook QMK =====

// Processa eventos de tecla para hold
bool hold_process_record_user(keyrecord_t *record, keymod_t keymod) {
    // Obtém row/col do record (mais eficiente que usar keycode)
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // Busca a tecla custom por posição (O(1))
    custom_t* custom = hold_get_key_by_position(row, col);
    
    if (custom) {
        // Verifica se o behavior é HOLD e está habilitado usando custom
        customs_t behavior = custom_get_behavior_by_position(row, col);
        if (behavior == KEY_CUSTOM_HOLD && custom->state != DISABLED) {
            if (record->event.pressed) {
                activate_hold_key(custom);
            } else {
                deactivate_hold_key(custom);
            }
            return true;
        }
    }

    return true;
}

// Hook matrix_scan_user
static void hold_matrix_scan_user(void) {
    update_hold_states();
}

// Hook rgb_matrix_indicators_user
static bool hold_rgb_matrix_indicators_user(void) {
    if (!is_profile_mode_active()) return true;
    update_indicators();
    return false;
}

// Hook keyboard_post_init_user
static void hold_keyboard_post_init_user(void) {
    // Registra callbacks para KEY_CUSTOM_HOLD
    custom_callbacks_t callbacks = {
        .key_handler = hold_process_record_user,
        .add_callback = hold_key_add_callback,
        .remove_callback = hold_key_remove_callback
    };
    custom_register_callbacks(KEY_CUSTOM_HOLD, callbacks);
    
    // Estado de FN será inicializado como KEYMOD_NONE
    // Será atualizado via notificação quando FN for pressionado
    hold_fn_keymod = KEYMOD_NONE;
    
    // Inicializa outras coisas...
    sync_enabled_states();
    
    // Força chamada de add_callback para teclas que já têm o behavior no profile ativo
    // Isso garante que os LEDs sejam ligados na inicialização
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        if (behavior == KEY_CUSTOM_HOLD) {
            // Chama add_callback para inicializar LED e estado
            hold_key_add_callback(custom->row, custom->col, KEY_CUSTOM_HOLD);
        }
    }
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void hold_init_early_hooks(void) {
    hooks_matrix_scan_register(hold_matrix_scan_user);
    hooks_rgb_indicators_register(hold_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void hold_init_hooks(void) {
    hooks_keyboard_post_init_register(hold_keyboard_post_init_user);
}
