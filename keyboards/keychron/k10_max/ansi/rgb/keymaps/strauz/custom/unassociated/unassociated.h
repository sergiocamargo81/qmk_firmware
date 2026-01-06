#ifndef BEHAVIOR_CUSTOM_UNASSOCIATED_UNASSOCIATED_H
#define BEHAVIOR_CUSTOM_UNASSOCIATED_UNASSOCIATED_H

#include QMK_KEYBOARD_H
#include "../../kind.h"  // Para custom_t
#include "../../keymod.h"  // Para keymod_t
#include "../../customs.h"  // Para customs_t

// ===== Defines =====
// Usa CUSTOM_COUNT de kind.h

// ===== Tipos =====
typedef uint8_t unassociated_state_t;
enum {
    UNASSOCIATED_IDLE,
    UNASSOCIATED_PRESSED
};

// Usa custom_t diretamente da matrix (campos READONLY: base, row, col, keycode, led_index, supported_keymod, persist_index, function)
// Campos modificáveis: state (uint8_t)

// ===== Funções de Estado =====
// Acesso direto O(1) via row/col → custom_t da matrix
custom_t* unassociated_get_key_by_position(uint8_t row, uint8_t col);
custom_t* unassociated_get_key_by_index(uint8_t persist_index);

void unassociated_update_indicators(void);

// ===== Callback de Notificação de FN =====

// Tipo de callback para notificar mudança de estado de FN
// Compatível com modifiers_fn_state_callback_t
typedef void (*unassociated_fn_state_callback_t)(keymod_t keymod);

// Retorna o callback de notificação de FN
// Será chamado externamente para registrar o callback em modifiers
unassociated_fn_state_callback_t unassociated_get_fn_callback(void);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void unassociated_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void unassociated_init_hooks(void);

#endif // BEHAVIOR_CUSTOM_UNASSOCIATED_UNASSOCIATED_H
