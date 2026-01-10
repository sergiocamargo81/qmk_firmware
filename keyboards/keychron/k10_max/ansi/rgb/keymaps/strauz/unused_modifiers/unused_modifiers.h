#ifndef UNUSED_MODIFIERS_H
#define UNUSED_MODIFIERS_H

#include QMK_KEYBOARD_H
#include "../keymod.h"
#include "../kind.h"  // Para base_key_t

// ===== Inicialização =====

// Inicializa o módulo (chamado durante keyboard_post_init_user)
void unused_modifiers_init(void);

// Registra hooks QMK (chamado durante keyboard_post_init_user)
void unused_modifiers_init_hooks(void);

// ===== Processamento de Eventos =====

// Processa eventos de unused_modifiers (TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN)
// Retorna false se consumiu o evento, true caso contrário
bool unused_modifiers_process_key(base_key_t* key, bool pressed, keymod_t keymod);

#endif // UNUSED_MODIFIERS_H