#ifndef BEHAVIOR_CUSTOM_HOLD_HOLD_H
#define BEHAVIOR_CUSTOM_HOLD_HOLD_H

#include QMK_KEYBOARD_H
#include "../../kind.h"  // Para custom_t
#include "../../settings.h"
#include "../../keymod.h"  // Para keymod_t

// ===== Defines =====
#define CUSTOM_DELAY 1500  // 1.5 segundos
#define CUSTOM_INTERVAL 50
#define LED_DEACTIVATE_TIME 5000
// Usa CUSTOM_COUNT de kind.h

// ===== Tipos =====
typedef uint8_t hold_state_t;
enum {
    DISABLED,
    WAITING,
    PRESSING,
    FIRING,
    RESTING
};

// Usa custom_t diretamente da matrix (campos READONLY: base, row, col, keycode, led_index, supported_keymod, persist_index, function)
// Campos modificáveis: state (uint8_t), timer (uint32_t)

// ===== Variáveis Globais =====
extern bool hold_profile_mode;
extern uint8_t hold_enabled_keys_bitfield[6];

// ===== Funções de Estado =====
// Acesso direto O(1) via row/col → custom_t da matrix
custom_t* hold_get_key_by_position(uint8_t row, uint8_t col);
custom_t* hold_get_key_by_index(uint8_t persist_index);

void update_hold_states(void);

void activate_hold_key(custom_t *custom);
void deactivate_hold_key(custom_t *custom);
void deactivate_all_hold(void);

void sync_enabled_states(void);
void update_indicators(void);

bool is_profile_mode_active(void);

void reset_all_enabled_keys(void);

// ===== Callback de Notificação de FN =====

// Tipo de callback para notificar mudança de estado de FN
// Compatível com modifiers_fn_state_callback_t
typedef void (*hold_fn_state_callback_t)(keymod_t keymod);

// Retorna o callback de notificação de FN
// Será chamado externamente para registrar o callback em modifiers
hold_fn_state_callback_t hold_get_fn_callback(void);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void hold_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void hold_init_hooks(void);

#endif // BEHAVIOR_CUSTOM_HOLD_HOLD_H

