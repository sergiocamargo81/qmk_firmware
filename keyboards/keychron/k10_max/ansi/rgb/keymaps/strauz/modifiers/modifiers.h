#ifndef MODIFIERS_H
#define MODIFIERS_H

#include QMK_KEYBOARD_H
#include "../keymod.h"  // Para keymod_t
#include "../kind.h"    // Para base_key_t
#include <stdbool.h>
#include <stdint.h>

// ===== Defines =====
#define MODIFIERS_MAX_CALLBACKS 3  // Máximo de callbacks registrados (hold, toggle, unassociated)
#define MODIFIERS_COUNT 4  // Número total de modifiers (RSFT, RALT, FN, RCTL)

// ===== Tipos =====

// ===== API Pública =====
// Sistema de callbacks migrado para Event Bus
// Use event_bus_subscribe_fn_state_changed() em vez de modifiers_register_fn_callback()

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
bool modifiers_process_key(base_key_t* key, bool pressed, keymod_t keymod);

#endif // MODIFIERS_H
