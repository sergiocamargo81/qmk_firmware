#ifndef BEHAVIOR_PROFILES_PROFILES_H
#define BEHAVIOR_PROFILES_PROFILES_H

#include QMK_KEYBOARD_H
#include "../profiles.h"

// ===== Tipos =====
// Estados possíveis do LED de um profile
typedef enum {
    PROFILES_LED_ACTIVE,    // Profile ativo: Verde
    PROFILES_LED_EMPTY,     // Profile vazio: Branco
    PROFILES_LED_NONEMPTY   // Profile não vazio (mas não ativo): Amarelo
} profiles_led_state_t;

// Cores RGB para cada estado
typedef struct {
    uint8_t r, g, b;
} profiles_led_color_t;

// Estado do módulo profiles (apenas para LEDs)
typedef struct {
    profiles_led_state_t led_states[PROFILES_COUNT];  // Estado atual de cada LED (armazenado)
    bool led_initialized;                       // Se LEDs foram inicializados
} profiles_state_t;

// ===== Variáveis Globais =====
extern profiles_state_t profiles_state;

// ===== Funções =====
// Registra todas as posições de KC_P0-KC_P9 na matriz de behavior
// Deve ser chamado durante profiles_keyboard_post_init_user
void profiles_register_positions(void);

// Inicialização
void profiles_init_led_indices(void);
void profiles_sync_from_settings(void);

// Atualização de estados e indicadores
void profiles_update_led_states(void);  // Atualiza estados baseado em profiles_t
void profiles_update_indicators(void);  // Renderiza LEDs baseado nos estados

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void profiles_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void profiles_init_hooks(void);

#endif // BEHAVIOR_PROFILES_PROFILES_H

