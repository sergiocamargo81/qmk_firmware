#ifndef BEHAVIOR_CUSTOM_TOGGLE_TOGGLE_H
#define BEHAVIOR_CUSTOM_TOGGLE_TOGGLE_H

#include QMK_KEYBOARD_H
#include "../../kind.h"  // Para custom_t
#include "../../settings.h"
#include "../../event_bus.h"  // Para event_handler_t
#include "../../keymod.h"  // Para keymod_t

// ===== Defines =====
#define TOGGLE_INTERVAL 50
// Usa CUSTOM_COUNT de kind.h

// ===== Tipos =====
typedef uint8_t toggle_state_t;
enum {
    TOGGLE_WAITING,  // Verde pulsante, aguardando clique
    TOGGLE_FIRING    // Vermelho contínuo, disparando automaticamente
};

// Usa custom_t diretamente da matrix (campos READONLY: base, row, col, keycode, led_index, accepted_keymods, persist_index, process_key)
// Campos modificáveis: state (uint8_t), timer (uint32_t)

// ===== Variáveis Globais =====
extern bool toggle_profile_mode;
extern uint8_t toggle_enabled_keys_bitfield[6];

// ===== Funções de Estado =====
// Acesso direto O(1) via row/col → custom_t da matrix
custom_t* toggle_get_key_by_position(uint8_t row, uint8_t col);
custom_t* toggle_get_key_by_index(uint8_t persist_index);

void toggle_update_states(void);
void toggle_update_indicators(void);

bool toggle_is_profile_mode_active(void);

// ===== Callback de Notificação de FN =====

// Callback para notificar mudança de estado de FN via Event Bus
// Deve ser registrado no Event Bus em keyboard_post_init_user
void toggle_fn_state_callback(const event_t* event);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void toggle_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void toggle_init_hooks(void);

#endif // BEHAVIOR_CUSTOM_TOGGLE_TOGGLE_H

