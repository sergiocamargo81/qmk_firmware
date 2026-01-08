#ifndef NUMLOCK_H
#define NUMLOCK_H

#include QMK_KEYBOARD_H
#include "../kind.h"  // Para numlock_t

// Usa numlock_t diretamente da matrix (campos READONLY: base, row, col, keycode, led_index, supported_keymod, function)
// Campos modificáveis: state (uint8_t) - não usado neste módulo

// ===== Variáveis Globais =====
// Cache do ponteiro para numlock_t de KC_NUM (inicializado durante init)
extern numlock_t* numlock_position;

// ===== Funções =====
// Registra a posição de KC_NUM na matriz de behavior
// Deve ser chamado durante numlock_keyboard_post_init_user
void numlock_register_position(void);

// Função removida - estado é verificado diretamente do host quando necessário

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void numlock_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void numlock_init_hooks(void);

#endif // NUMLOCK_H

