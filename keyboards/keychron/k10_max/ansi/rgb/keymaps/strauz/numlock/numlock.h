#ifndef NUMLOCK_H
#define NUMLOCK_H

#include QMK_KEYBOARD_H

// ===== Tipos =====
typedef struct {
    uint8_t led_index;
} numlock_t;

// ===== Variáveis Globais =====
extern numlock_t numlock_key;

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
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void numlock_init_hooks(void);

#endif // NUMLOCK_H

