#include "toggle.h"

#include "../../behavior.h"
#include "../../kind.h"
#include "../../customs.h"  // Para customs_t
#include "../../profiles.h"
#include "../../settings.h"
#include "../custom.h"
#include "../../hooks.h"
#include "../../keymod.h"  // Para keymod_t
#include "../hold/hold.h"  // Para deactivate_all_hold
#include "../../pulse.h"

// Declarações forward
bool toggle_process_record_user(keyrecord_t *record, keymod_t keymod);
void toggle_key_add_callback(uint8_t row, uint8_t col, customs_t behavior);
void toggle_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior);

// ===== Variáveis Globais =====
toggle_key_t toggle_keys[] = {
    {0, 3, 1, NO_LED, TOGGLE_OFF, 0, 0}, {0, 4, 6, NO_LED, TOGGLE_OFF, 0, 1},
    {0, 4, 4, NO_LED, TOGGLE_OFF, 0, 2}, {0, 3, 3, NO_LED, TOGGLE_OFF, 0, 3},
    {0, 2, 3, NO_LED, TOGGLE_OFF, 0, 4}, {0, 3, 4, NO_LED, TOGGLE_OFF, 0, 5},
    {0, 3, 5, NO_LED, TOGGLE_OFF, 0, 6}, {0, 3, 6, NO_LED, TOGGLE_OFF, 0, 7},
    {0, 2, 8, NO_LED, TOGGLE_OFF, 0, 8}, {0, 3, 7, NO_LED, TOGGLE_OFF, 0, 9},
    {0, 3, 8, NO_LED, TOGGLE_OFF, 0, 10}, {0, 3, 9, NO_LED, TOGGLE_OFF, 0, 11},
    {0, 4, 8, NO_LED, TOGGLE_OFF, 0, 12}, {0, 4, 7, NO_LED, TOGGLE_OFF, 0, 13},
    {0, 2, 9, NO_LED, TOGGLE_OFF, 0, 14}, {0, 2, 10, NO_LED, TOGGLE_OFF, 0, 15},
    {0, 2, 1, NO_LED, TOGGLE_OFF, 0, 16}, {0, 2, 4, NO_LED, TOGGLE_OFF, 0, 17},
    {0, 3, 2, NO_LED, TOGGLE_OFF, 0, 18}, {0, 2, 5, NO_LED, TOGGLE_OFF, 0, 19},
    {0, 2, 7, NO_LED, TOGGLE_OFF, 0, 20}, {0, 4, 5, NO_LED, TOGGLE_OFF, 0, 21},
    {0, 2, 2, NO_LED, TOGGLE_OFF, 0, 22}, {0, 4, 3, NO_LED, TOGGLE_OFF, 0, 23},
    {0, 2, 6, NO_LED, TOGGLE_OFF, 0, 24}, {0, 4, 2, NO_LED, TOGGLE_OFF, 0, 25},
    {0, 1, 1, NO_LED, TOGGLE_OFF, 0, 26}, {0, 1, 2, NO_LED, TOGGLE_OFF, 0, 27},
    {0, 1, 3, NO_LED, TOGGLE_OFF, 0, 28}, {0, 1, 4, NO_LED, TOGGLE_OFF, 0, 29},
    {0, 1, 5, NO_LED, TOGGLE_OFF, 0, 30}, {0, 1, 6, NO_LED, TOGGLE_OFF, 0, 31},
    {0, 1, 7, NO_LED, TOGGLE_OFF, 0, 32}, {0, 1, 8, NO_LED, TOGGLE_OFF, 0, 33},
    {0, 1, 9, NO_LED, TOGGLE_OFF, 0, 34}, {0, 1, 10, NO_LED, TOGGLE_OFF, 0, 35},
    {0, 5, 6, NO_LED, TOGGLE_OFF, 0, 36}, {0, 1, 11, NO_LED, TOGGLE_OFF, 0, 37},
    {0, 1, 12, NO_LED, TOGGLE_OFF, 0, 38}, {0, 2, 11, NO_LED, TOGGLE_OFF, 0, 39},
    {0, 2, 12, NO_LED, TOGGLE_OFF, 0, 40}, {0, 2, 13, NO_LED, TOGGLE_OFF, 0, 41},
    {0, 3, 10, NO_LED, TOGGLE_OFF, 0, 42}, {0, 3, 11, NO_LED, TOGGLE_OFF, 0, 43},
    {0, 1, 0, NO_LED, TOGGLE_OFF, 0, 44}, {0, 4, 9, NO_LED, TOGGLE_OFF, 0, 45},
    {0, 4, 10, NO_LED, TOGGLE_OFF, 0, 46}, {0, 4, 11, NO_LED, TOGGLE_OFF, 0, 47},
};

bool toggle_profile_mode = true;
uint8_t toggle_enabled_keys_bitfield[6] = {0};

// ===== Funções Auxiliares =====
static bool is_key_enabled(toggle_key_t *key) {
    if (!key) return false;
    // Verifica diretamente o behavior atual em vez de depender do bitfield
    customs_t behavior = custom_get_behavior_by_position(key->row, key->col);
    return (behavior == KEY_CUSTOM_TOGGLE);
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

// Callback para notificar inclusão de tecla no controle de toggle
void toggle_key_add_callback(uint8_t row, uint8_t col, customs_t behavior) {
    toggle_key_t* key = toggle_get_key_by_position(row, col);
    if (key) {
        // Garante que led_index está preenchido (caso não tenha sido inicializado)
        if (key->led_index == NO_LED) {
            custom_t* custom = kind_get_custom(row, col);
            if (custom) {
                key->led_index = custom->led_index;
            }
        }
        // Inicializa estado apenas para esta tecla
        key->state = TOGGLE_OFF;
        // Atualiza o LED imediatamente após a associação
        // O LED ficará verde pulsante (TOGGLE_OFF) até que a tecla seja pressionada
        // A pulsação será aplicada em toggle_update_indicators()
        if (key->led_index != NO_LED) {
            // Inicializa com verde, a pulsação será aplicada no próximo toggle_update_indicators
            rgb_matrix_set_color(key->led_index, 0, 255, 0);
        }
    }
}

// Callback para notificar remoção de tecla do controle de toggle
void toggle_key_remove_callback(uint8_t row, uint8_t col, customs_t behavior) {
    toggle_key_t* key = toggle_get_key_by_position(row, col);
    if (key) {
        // Limpa estado e apaga o LED
        key->state = TOGGLE_OFF;
        if (key->led_index != NO_LED) {
            rgb_matrix_set_color(key->led_index, RGB_OFF);
            key->led_index = NO_LED; // Marca como não associada
        }
    }
}

// ===== Funções de Estado =====
toggle_key_t* toggle_get_key_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) return NULL;
    return &toggle_keys[custom->custom_index];
}

toggle_key_t* toggle_get_key_by_index(uint8_t behavior_index) {
    if (behavior_index >= CUSTOM_KEYS_COUNT) return NULL;
    return &toggle_keys[behavior_index];
}

void toggle_init_led_indices(void) {
    // NÃO preenche led_index aqui - led_index só deve ser preenchido quando a tecla é associada
    // via toggle_key_add_callback. Isso previne que todas as teclas tenham LEDs ligados.
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom) {
            toggle_keys[i].row = custom->row;
            toggle_keys[i].col = custom->col;
            toggle_keys[i].keycode = custom->keycode;
            // led_index permanece NO_LED até que a tecla seja associada
            toggle_keys[i].led_index = NO_LED;
        }
    }
}

void toggle_update_states(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        toggle_key_t *key = &toggle_keys[i];
        if (key->state == TOGGLE_OFF) continue;
        if (key->state == TOGGLE_ON) {
            if (timer_elapsed32(key->timer) >= CUSTOM_INTERVAL) {
                tap_code(key->keycode);
                key->timer = now;
            }
        }
    }
}

void toggle_activate_key(toggle_key_t *key) {
    uint32_t now = timer_read32();
    if (key->state == TOGGLE_OFF) {
        key->state = TOGGLE_ON;
        key->timer = now;
        tap_code(key->keycode);
    } else {
        key->state = TOGGLE_OFF;
    }
}

void toggle_deactivate_key(toggle_key_t *key) {
    // Não faz nada no release, o toggle é controlado apenas no press
}

void toggle_deactivate_all(void) {
    // Desativa todas as teclas toggle que estão em estado TOGGLE_ON
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        if (toggle_keys[i].state == TOGGLE_ON) {
            toggle_keys[i].state = TOGGLE_OFF;
        }
    }
}

void toggle_sync_enabled_states(void) {
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        bool is_enabled = is_key_enabled(&toggle_keys[i]);
        if (!is_enabled) toggle_keys[i].state = TOGGLE_OFF;
    }
}


void toggle_update_indicators(void) {
    // Atualiza apenas os LEDs das teclas que estão associadas a TOGGLE
    // Verifica diretamente o behavior atual para garantir que apenas teclas realmente associadas sejam atualizadas
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        toggle_key_t *key = &toggle_keys[i];
        // Verifica se esta tecla está realmente associada a TOGGLE
        customs_t behavior = custom_get_behavior_by_position(key->row, key->col);
        if (behavior == KEY_CUSTOM_TOGGLE && key->led_index != NO_LED) {
            // Esta tecla está associada a TOGGLE e tem led_index válido
            // Atualiza LED baseado no estado
            switch (key->state) {
                case TOGGLE_OFF: {
                    // Estado inicial: verde pulsante
                    uint8_t brightness = calculate_pulse_brightness();
                    rgb_matrix_set_color(key->led_index, 0, brightness, 0);
                    break;
                }
                case TOGGLE_ON: rgb_matrix_set_color(key->led_index, 255, 0, 0); break;   // Vermelho quando ligado
            }
        }
    }
}

bool toggle_is_profile_mode_active(void) {
    settings_t* working = settings_get_working();
    profile_t* active_profile = profiles_get_active_profile(&working->profiles);
    if (!active_profile || profiles_is_empty(active_profile)) {
        return false;
    }
    return toggle_profile_mode;
}

void toggle_reset_all_keys(void) {
    memset(toggle_enabled_keys_bitfield, 0, sizeof(toggle_enabled_keys_bitfield));
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) toggle_keys[i].state = TOGGLE_OFF;
}

// ===== Funções de Hook QMK =====

// Processa eventos de tecla para toggle
bool toggle_process_record_user(keyrecord_t *record, keymod_t keymod) {
    // Obtém row/col do record (mais eficiente que usar keycode)
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // Busca a tecla toggle por posição (O(1))
    toggle_key_t *key = toggle_get_key_by_position(row, col);
    
    if (key) {
        // Verifica se o behavior é TOGGLE usando custom
        customs_t behavior = custom_get_behavior_by_position(row, col);
        if (behavior == KEY_CUSTOM_TOGGLE) {
            if (record->event.pressed) {
                // Space tem tratamento especial: desativa todas as outras antes
                // Desativa todas as teclas hold e toggle que estão ativas
                if (row == 5 && col == 6) {  // KC_SPC está em (5, 6)
                    deactivate_all_hold();
                    toggle_deactivate_all();
                }
                // Alterna o estado: OFF -> ON, ON -> OFF
                toggle_activate_key(key);
            } else {
                // No release, não faz nada (toggle é controlado apenas no press)
                toggle_deactivate_key(key);
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
    
    // Inicializa outras coisas...
    toggle_init_led_indices();
    toggle_sync_enabled_states();
    
    // Força chamada de add_callback para teclas que já têm o behavior no profile ativo
    // Isso garante que os LEDs sejam ligados na inicialização
    for (uint8_t i = 0; i < CUSTOM_KEYS_COUNT; i++) {
        toggle_key_t *key = &toggle_keys[i];
        customs_t behavior = custom_get_behavior_by_position(key->row, key->col);
        if (behavior == KEY_CUSTOM_TOGGLE) {
            // Chama add_callback para inicializar LED e estado
            toggle_key_add_callback(key->row, key->col, KEY_CUSTOM_TOGGLE);
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

