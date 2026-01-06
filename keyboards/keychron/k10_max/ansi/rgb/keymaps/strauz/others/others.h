#ifndef OTHERS_H
#define OTHERS_H

#include QMK_KEYBOARD_H
#include "../keymod.h"

// ===== Inicialização =====

// Inicializa o módulo (chamado durante keyboard_post_init_user)
void others_init(void);

// Registra hooks QMK (chamado durante keyboard_post_init_user)
void others_init_hooks(void);

// ===== Processamento de Eventos =====

// Processa eventos de others (TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN)
// Retorna false se consumiu o evento, true caso contrário
bool others_process_record_user(keyrecord_t *record, keymod_t keymod);

#endif // OTHERS_H
