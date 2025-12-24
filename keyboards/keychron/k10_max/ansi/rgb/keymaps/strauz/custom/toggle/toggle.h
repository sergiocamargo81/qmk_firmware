#ifndef BEHAVIOR_CUSTOM_TOGGLE_TOGGLE_H
#define BEHAVIOR_CUSTOM_TOGGLE_TOGGLE_H

#include QMK_KEYBOARD_H
#include "../../settings.h"

// ===== Defines =====
#define CUSTOM_DELAY 500
#define CUSTOM_INTERVAL 50
#define LED_DEACTIVATE_TIME 5000
#define CUSTOM_KEYS_COUNT 48

// ===== Tipos =====
typedef enum {
    TOGGLE_OFF,
    TOGGLE_ON
} toggle_state_t;

typedef struct {
    uint16_t        keycode;
    uint8_t         row, col, led_index;
    toggle_state_t state;
    uint32_t        timer;
    uint8_t         persist_index;
} toggle_key_t;

// ===== Variáveis Globais =====
extern toggle_key_t toggle_keys[];
extern bool toggle_profile_mode;
extern uint8_t toggle_enabled_keys_bitfield[6];

// ===== Funções de Estado =====
// Acesso direto O(1) via row/col → behavior_index
toggle_key_t* toggle_get_key_by_position(uint8_t row, uint8_t col);
toggle_key_t* toggle_get_key_by_index(uint8_t behavior_index);

void toggle_init_led_indices(void);
void toggle_update_states(void);

void toggle_activate_key(toggle_key_t *key);
void toggle_deactivate_key(toggle_key_t *key);
void toggle_deactivate_all(void);

void toggle_sync_enabled_states(void);
void toggle_update_indicators(void);

bool toggle_is_profile_mode_active(void);

void toggle_reset_all_keys(void);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void toggle_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void toggle_init_hooks(void);

#endif // BEHAVIOR_CUSTOM_TOGGLE_TOGGLE_H

