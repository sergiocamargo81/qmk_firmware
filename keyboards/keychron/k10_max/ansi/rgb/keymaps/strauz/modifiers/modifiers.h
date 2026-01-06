#ifndef MODIFIERS_H
#define MODIFIERS_H

#include QMK_KEYBOARD_H
#include "../keymod.h"  // Para keymod_t
#include <stdbool.h>
#include <stdint.h>

// ===== Defines =====
#define MODIFIERS_MAX_CALLBACKS 3  // Máximo de callbacks registrados (hold, toggle, unassociated)
#define MODIFIERS_COUNT 4  // Número total de modifiers (RSFT, RALT, FN, RCTL)

// ===== Tipos =====

// Tipo de callback para notificar mudança de estado de FN
// keymod: keymod_t ativo (KEYMOD_FN_ONLY, KEYMOD_FN_RCTL, KEYMOD_FN_RALT, KEYMOD_FN_RSFT) ou KEYMOD_NONE quando solto
typedef void (*modifiers_fn_state_callback_t)(keymod_t keymod);

// ===== API Pública =====

// Registra um callback para ser notificado quando FN é pressionada/solta
// Retorna true se sucesso, false se limite atingido ou callback já registrado
bool modifiers_register_fn_callback(modifiers_fn_state_callback_t callback);

// Remove um callback registrado
// Retorna true se encontrado e removido, false caso contrário
bool modifiers_unregister_fn_callback(modifiers_fn_state_callback_t callback);

// Consulta se FN está atualmente pressionada
// Útil para módulos que precisam verificar estado sem callback
bool modifiers_is_fn_pressed(void);

// ===== Inicialização =====

// Inicializa o módulo (chamado durante keyboard_post_init_user)
void modifiers_init(void);

// Registra hooks QMK (chamado durante keyboard_post_init_user)
void modifiers_init_hooks(void);

// ===== Processamento de Eventos =====

// Processa eventos de modifiers (FN, RSHIFT, RALT, RCTRL)
// Retorna false se consumiu o evento, true caso contrário
bool modifiers_process_record_user(keyrecord_t *record, keymod_t keymod);

#endif // MODIFIERS_H
