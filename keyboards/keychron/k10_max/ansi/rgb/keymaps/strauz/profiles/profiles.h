#ifndef BEHAVIOR_PROFILES_PROFILES_H
#define BEHAVIOR_PROFILES_PROFILES_H

#include QMK_KEYBOARD_H
#include "../profile.h"
#include "../kind.h"  // Para profile_key_t

// ===== Tipos =====
// Estados possíveis de uma key de profile (armazenados em profile_key_t->state)
typedef uint8_t profile_state_t;
enum {
    PROFILE_ACTIVE,    // Profile ativo
    PROFILE_EMPTY,     // Profile vazio
    PROFILE_NONEMPTY,  // Profile não vazio (mas não ativo)
    PROFILE_PRESSED    // Tecla está pressionada (sobrescreve outros estados temporariamente)
};

// ===== Funções =====
// Registra todas as posições de KC_P0-KC_P9 na matriz de behavior
// Deve ser chamado durante profiles_keyboard_post_init_user
void profiles_register_positions(void);

// Inicialização
void profiles_sync_from_settings(void);

// Atualização de estados e indicadores
void profiles_update_states(void);  // Atualiza estados na matrix baseado em profiles_t
void profiles_update_indicators(void);  // Renderiza LEDs baseado nos estados na matrix

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void profiles_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void profiles_init_hooks(void);

#endif // BEHAVIOR_PROFILES_PROFILES_H

