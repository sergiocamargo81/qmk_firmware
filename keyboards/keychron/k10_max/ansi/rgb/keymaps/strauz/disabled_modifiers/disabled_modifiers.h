#ifndef DISABLED_MODIFIERS_H
#define DISABLED_MODIFIERS_H

#include QMK_KEYBOARD_H
#include "../keymod.h"
#include "../kind.h"  // Para base_key_t

// ===== Inicialização =====

// Inicializa o módulo (chamado durante keyboard_post_init_user)
void disabled_modifiers_init(void);

// Registra hooks QMK (chamado durante keyboard_post_init_user)
void disabled_modifiers_init_hooks(void);

// ===== Processamento de Eventos =====

// Processa eventos de disabled_modifiers (TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN)
// Retorna false se consumiu o evento, true caso contrário
bool disabled_modifiers_process_key(base_key_t* key, bool pressed, keymod_t keymod);

#endif // DISABLED_MODIFIERS_H
