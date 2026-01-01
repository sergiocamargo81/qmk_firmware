#ifndef MODIFIERS_H
#define MODIFIERS_H

#include <stdbool.h>
#include <stdint.h>

// ===== Defines =====
#define MODIFIERS_MAX_CALLBACKS 8  // Máximo de callbacks registrados

// ===== Tipos =====

// Tipo de callback para notificar mudança de estado de FN
// fn_pressed: true quando FN começa a ser segurado, false quando é solto
typedef void (*modifiers_fn_state_callback_t)(bool fn_pressed);

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

#endif // MODIFIERS_H
