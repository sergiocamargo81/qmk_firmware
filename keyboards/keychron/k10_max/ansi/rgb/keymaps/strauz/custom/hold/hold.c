#include "hold.h"

#include "../../behavior.h"
#include "../../kind.h"
#include "../../customs.h"  // Para customs_t
#include "../../profiles.h"
#include "../../settings.h"
#include "../custom.h"
#include "../../hooks.h"
#include "../../keymod.h"  // Para keymod_t

// Declarações forward
bool hold_process_record_user(keyrecord_t *record, keymod_t keymod);
void hold_key_add_callback(uint8_t row, uint8_t col, customs_t behavior);
void hold_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior);

// ===== Variáveis Globais =====
hold_key_t hold_keys[] = {
    {0, 3, 1, NO_LED, DISABLED, 0, 0}, {0, 4, 6, NO_LED, DISABLED, 0, 1},
    {0, 4, 4, NO_LED, DISABLED, 0, 2}, {0, 3, 3, NO_LED, DISABLED, 0, 3},
    {0, 2, 3, NO_LED, DISABLED, 0, 4}, {0, 3, 4, NO_LED, DISABLED, 0, 5},
    {0, 3, 5, NO_LED, DISABLED, 0, 6}, {0, 3, 6, NO_LED, DISABLED, 0, 7},
    {0, 2, 8, NO_LED, DISABLED, 0, 8}, {0, 3, 7, NO_LED, DISABLED, 0, 9},
    {0, 3, 8, NO_LED, DISABLED, 0, 10}, {0, 3, 9, NO_LED, DISABLED, 0, 11},
    {0, 4, 8, NO_LED, DISABLED, 0, 12}, {0, 4, 7, NO_LED, DISABLED, 0, 13},
    {0, 2, 9, NO_LED, DISABLED, 0, 14}, {0, 2, 10, NO_LED, DISABLED, 0, 15},
    {0, 2, 1, NO_LED, DISABLED, 0, 16}, {0, 2, 4, NO_LED, DISABLED, 0, 17},
    {0, 3, 2, NO_LED, DISABLED, 0, 18}, {0, 2, 5, NO_LED, DISABLED, 0, 19},
    {0, 2, 7, NO_LED, DISABLED, 0, 20}, {0, 4, 5, NO_LED, DISABLED, 0, 21},
    {0, 2, 2, NO_LED, DISABLED, 0, 22}, {0, 4, 3, NO_LED, DISABLED, 0, 23},
    {0, 2, 6, NO_LED, DISABLED, 0, 24}, {0, 4, 2, NO_LED, DISABLED, 0, 25},
    {0, 1, 1, NO_LED, DISABLED, 0, 26}, {0, 1, 2, NO_LED, DISABLED, 0, 27},
    {0, 1, 3, NO_LED, DISABLED, 0, 28}, {0, 1, 4, NO_LED, DISABLED, 0, 29},
    {0, 1, 5, NO_LED, DISABLED, 0, 30}, {0, 1, 6, NO_LED, DISABLED, 0, 31},
    {0, 1, 7, NO_LED, DISABLED, 0, 32}, {0, 1, 8, NO_LED, DISABLED, 0, 33},
    {0, 1, 9, NO_LED, DISABLED, 0, 34}, {0, 1, 10, NO_LED, DISABLED, 0, 35},
    {0, 5, 6, NO_LED, DISABLED, 0, 36}, {0, 1, 11, NO_LED, DISABLED, 0, 37},
    {0, 1, 12, NO_LED, DISABLED, 0, 38}, {0, 2, 11, NO_LED, DISABLED, 0, 39},
    {0, 2, 12, NO_LED, DISABLED, 0, 40}, {0, 2, 13, NO_LED, DISABLED, 0, 41},
    {0, 3, 10, NO_LED, DISABLED, 0, 42}, {0, 3, 11, NO_LED, DISABLED, 0, 43},
    {0, 1, 0, NO_LED, DISABLED, 0, 44}, {0, 4, 9, NO_LED, DISABLED, 0, 45},
    {0, 4, 10, NO_LED, DISABLED, 0, 46}, {0, 4, 11, NO_LED, DISABLED, 0, 47},
};

bool hold_profile_mode = true;
uint8_t hold_enabled_keys_bitfield[6] = {0};

// ===== Funções Auxiliares =====
static bool is_key_enabled(hold_key_t *key) {
    if (!key) return false;
    // Verifica diretamente o behavior atual em vez de depender do bitfield
    customs_t behavior = custom_get_behavior_by_position(key->row, key->col);
    return (behavior == KEY_CUSTOM_HOLD);
}

static bool profiles_is_empty(profile_t* profile) {
    if (!profile) return true;
    for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
        if (profile->behaviors[i] != KEY_CUSTOM_DISABLED) {
            return false;
        }
    }
    return true;
}

// Callback para notificar inclusão de tecla no controle de hold
void hold_key_add_callback(uint8_t row, uint8_t col, customs_t behavior) {
    hold_key_t* key = hold_get_key_by_position(row, col);
    if (key) {
        // Garante que led_index está preenchido (caso não tenha sido inicializado)
        if (key->led_index == NO_LED) {
            custom_t* custom = kind_get_custom(row, col);
            if (custom) {
                key->led_index = custom->led_index;
            }
        }
        // Inicializa estado apenas para esta tecla
        key->state = WAITING;
        // Atualiza o LED imediatamente após a associação
        // O LED ficará cinza (WAITING) indicando que está aguardando
        if (key->led_index != NO_LED) {
            rgb_matrix_set_color(key->led_index, 128, 128, 128);
        }
    }
}

// Callback para notificar remoção de tecla do controle de hold
void hold_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior) {
    hold_key_t* key = hold_get_key_by_position(row, col);
    if (key) {
        // Limpa estado e apaga o LED
        key->state = DISABLED;
        if (key->led_index != NO_LED) {
            rgb_matrix_set_color(key->led_index, RGB_OFF);
            key->led_index = NO_LED; // Marca como não associada
        }
    }
}

// ===== Funções de Estado =====
hold_key_t* hold_get_key_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) return NULL;
    return &hold_keys[custom->custom_index];
}

hold_key_t* hold_get_key_by_index(uint8_t behavior_index) {
    if (behavior_index >= CUSTOM_KEYS_COUNT) return NULL;
    return &hold_keys[behavior_index];
}

void init_led_indices(void) {
    // NÃO preenche led_index aqui - led_index só deve ser preenchido quando a tecla é associada
    // via hold_key_add_callback. Isso previne que todas as teclas tenham LEDs ligados.
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom) {
            hold_keys[i].row = custom->row;
            hold_keys[i].col = custom->col;
            hold_keys[i].keycode = custom->keycode;
            // led_index permanece NO_LED até que a tecla seja associada
            hold_keys[i].led_index = NO_LED;
        }
    }
}

void update_hold_states(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        hold_key_t *key = &hold_keys[i];
        if (key->state == DISABLED) continue;
        switch (key->state) {
            case PRESSING: {
                if (timer_elapsed32(key->timer) >= CUSTOM_DELAY) {
                    key->state = FIRING; key->timer = now; tap_code(key->keycode);
                }
                break;
            }
            case FIRING: {
                if (timer_elapsed32(key->timer) >= CUSTOM_INTERVAL) {
                    tap_code(key->keycode); key->timer = now;
                }
                break;
            }
            case RESTING: {
                if (timer_elapsed32(key->timer) >= LED_DEACTIVATE_TIME) key->state = WAITING;
                break;
            }
            default: break;
        }
    }
}

void activate_hold_key(hold_key_t *key) {
    uint32_t now = timer_read32();
    key->state = PRESSING; key->timer = now;
}

void deactivate_hold_key(hold_key_t *key) {
    uint32_t now = timer_read32();
    if (key->state == FIRING) { key->state = RESTING; key->timer = now; return; }
    if (key->state == PRESSING) { key->state = WAITING; return; }
}

void deactivate_all_hold(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++)
        if (hold_keys[i].state == FIRING) { hold_keys[i].state = RESTING; hold_keys[i].timer = now; }
}

void sync_enabled_states(void) {
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        bool is_enabled = is_key_enabled(&hold_keys[i]);
        if (!is_enabled) {
            hold_keys[i].state = DISABLED;
        } else {
            // Se está habilitado e estava desabilitado, inicializa como WAITING
            if (hold_keys[i].state == DISABLED) {
                hold_keys[i].state = WAITING;
            }
            // Se já estava em outro estado (WAITING, PRESSING, etc), mantém o estado
        }
    }
}

void update_indicators(void) {
    // Atualiza apenas os LEDs das teclas que estão associadas a HOLD
    // Verifica diretamente o behavior atual para garantir que apenas teclas realmente associadas sejam atualizadas
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        hold_key_t *key = &hold_keys[i];
        // Verifica se esta tecla está realmente associada a HOLD
        customs_t behavior = custom_get_behavior_by_position(key->row, key->col);
        if (behavior == KEY_CUSTOM_HOLD && key->led_index != NO_LED) {
            // Esta tecla está associada a HOLD e tem led_index válido
            // Atualiza LED baseado no estado
            if (key->state != DISABLED) {
            switch (key->state) {
                case WAITING: rgb_matrix_set_color(key->led_index, 128, 128, 128); break;
                case PRESSING: rgb_matrix_set_color(key->led_index, 0, 255, 0); break;
                case FIRING: rgb_matrix_set_color(key->led_index, 255, 0, 0); break;
                case RESTING: rgb_matrix_set_color(key->led_index, 255, 255, 0); break;
                case DISABLED: rgb_matrix_set_color(key->led_index, RGB_OFF); break;
                }
            } else {
                // Estado é DISABLED, apaga o LED
                rgb_matrix_set_color(key->led_index, RGB_OFF);
            }
        }
    }
}

bool is_profile_mode_active(void) {
    settings_t* working = settings_get_working();
    profile_t* active_profile = profiles_get_active_profile(&working->profiles);
    if (!active_profile || profiles_is_empty(active_profile)) {
        return false;
    }
    return hold_profile_mode;
}

void reset_all_enabled_keys(void) {
    memset(hold_enabled_keys_bitfield, 0, sizeof(hold_enabled_keys_bitfield));
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) hold_keys[i].state = DISABLED;
}

// ===== Funções de Hook QMK =====

// Processa eventos de tecla para hold
bool hold_process_record_user(keyrecord_t *record, keymod_t keymod) {
    // Obtém row/col do record (mais eficiente que usar keycode)
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // Busca a tecla custom por posição (O(1))
    hold_key_t *key = hold_get_key_by_position(row, col);
    
    if (key) {
        // Verifica se o behavior é HOLD e está habilitado usando custom
        customs_t behavior = custom_get_behavior_by_position(row, col);
        if (behavior == KEY_CUSTOM_HOLD && key->state != DISABLED) {
            if (record->event.pressed) {
                // Space tem tratamento especial: desativa todas as outras antes
                // Desativa todas as teclas hold e toggle que estão ativas
                if (row == 5 && col == 6) {  // KC_SPC está em (5, 6)
                    deactivate_all_hold();
                    extern void toggle_deactivate_all(void);
                    toggle_deactivate_all();
                }
                activate_hold_key(key);
            } else {
                deactivate_hold_key(key);
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
    
    // Inicializa outras coisas...
    init_led_indices();
    sync_enabled_states();
    
    // Força chamada de add_callback para teclas que já têm o behavior no profile ativo
    // Isso garante que os LEDs sejam ligados na inicialização
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        hold_key_t *key = &hold_keys[i];
        customs_t behavior = custom_get_behavior_by_position(key->row, key->col);
        if (behavior == KEY_CUSTOM_HOLD) {
            // Chama add_callback para inicializar LED e estado
            hold_key_add_callback(key->row, key->col, KEY_CUSTOM_HOLD);
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

