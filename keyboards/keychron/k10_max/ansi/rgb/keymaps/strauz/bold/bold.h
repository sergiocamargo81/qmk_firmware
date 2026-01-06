#ifndef BOLD_BOLD_H
#define BOLD_BOLD_H

#include QMK_KEYBOARD_H
#include "../kind.h"  // Para bold_t
#include "../keymod.h"  // Para keymod_t

// ===== Tipos =====
typedef uint8_t bold_state_t;
enum {
    BOLD_IDLE,
    BOLD_PRESSED
};

// ===== Funções =====

// Processa eventos de teclas bold
bool bold_process_record_user(keyrecord_t *record, keymod_t keymod);

// Atualiza indicadores RGB das teclas bold (pulsação em laranja)
void bold_update_indicators(void);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void bold_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void bold_init_hooks(void);

#endif // BOLD_BOLD_H
