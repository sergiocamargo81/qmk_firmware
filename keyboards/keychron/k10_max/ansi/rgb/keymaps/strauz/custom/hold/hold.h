#ifndef BEHAVIOR_CUSTOM_HOLD_HOLD_H
#define BEHAVIOR_CUSTOM_HOLD_HOLD_H

#include QMK_KEYBOARD_H
#include "../../settings.h"

// ===== Defines =====
#define CUSTOM_DELAY 500
#define CUSTOM_INTERVAL 50
#define LED_DEACTIVATE_TIME 5000
#define CUSTOM_KEYS_COUNT 48

// ===== Tipos =====
typedef enum {
    DISABLED,
    WAITING,
    PRESSING,
    FIRING,
    RESTING
} hold_state_t;

typedef struct {
    uint16_t        keycode;
    uint8_t         row, col, led_index;
    hold_state_t state;
    uint32_t        timer;
    uint8_t         persist_index;
} hold_key_t;

// ===== Variáveis Globais =====
extern hold_key_t hold_keys[];
extern bool hold_profile_mode;
extern uint8_t hold_enabled_keys_bitfield[6];

// ===== Funções de Estado =====
// Acesso direto O(1) via row/col → behavior_index
hold_key_t* hold_get_key_by_position(uint8_t row, uint8_t col);
hold_key_t* hold_get_key_by_index(uint8_t behavior_index);

void init_led_indices(void);
void update_hold_states(void);

void activate_hold_key(hold_key_t *key);
void deactivate_hold_key(hold_key_t *key);
void deactivate_all_hold(void);

void sync_enabled_states(void);
void update_indicators(void);

bool is_profile_mode_active(void);

void reset_all_enabled_keys(void);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void hold_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void hold_init_hooks(void);

#endif // BEHAVIOR_CUSTOM_HOLD_HOLD_H

