#ifndef BEHAVIOR_CUSTOM_UNASSOCIATED_UNASSOCIATED_H
#define BEHAVIOR_CUSTOM_UNASSOCIATED_UNASSOCIATED_H

#include QMK_KEYBOARD_H
#include "../../kind.h"  // Para custom_t
#include "../../event_bus.h"  // Para event_handler_t
#include "../../keymod.h"  // Para keymod_t
#include "../../custom_behaviors.h"  // Para custom_behaviors_t

// ===== Defines =====
// Usa CUSTOM_COUNT de kind.h

// ===== Tipos =====
typedef uint8_t unassociated_state_t;
enum {
    UNASSOCIATED_IDLE,
    UNASSOCIATED_PRESSED
};

// Usa custom_t diretamente da matrix (campos READONLY: base, row, col, keycode, led_index, accepted_keymods, persist_index, process_key)
// Campos modificáveis: state (uint8_t)

// ===== Funções de Estado =====
// Acesso direto O(1) via row/col → custom_t da matrix
custom_t* unassociated_get_key_by_position(uint8_t row, uint8_t col);
custom_t* unassociated_get_key_by_index(uint8_t persist_index);

void unassociated_update_indicators(void);

// ===== Callback de Notificação de FN =====

// Callback para notificar mudança de estado de FN via Event Bus
// Deve ser registrado no Event Bus em keyboard_post_init_user
void unassociated_fn_state_callback(const event_t* event);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void unassociated_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void unassociated_init_hooks(void);

#endif // BEHAVIOR_CUSTOM_UNASSOCIATED_UNASSOCIATED_H
