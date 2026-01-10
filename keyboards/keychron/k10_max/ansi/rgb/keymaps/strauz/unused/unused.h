#ifndef UNUSED_H
#define UNUSED_H

#include QMK_KEYBOARD_H
#include "../kind.h"  // Para unused_t
#include "../keymod.h"  // Para keymod_t

// ===== Tipos =====
typedef uint8_t unused_state_t;
enum {
    UNUSED_IDLE,
    UNUSED_PRESSED
};

// ===== Funções =====

// Processa eventos de teclas unused
bool unused_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// Atualiza indicadores RGB das teclas unused (pulsação em branco)
void unused_update_indicators(void);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void unused_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void unused_init_hooks(void);

#endif // UNUSED_H