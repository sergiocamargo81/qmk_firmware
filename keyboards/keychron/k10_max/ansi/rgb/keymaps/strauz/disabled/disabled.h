#ifndef DISABLED_DISABLED_H
#define DISABLED_DISABLED_H

#include QMK_KEYBOARD_H
#include "../kind.h"  // Para disabled_t
#include "../keymod.h"  // Para keymod_t

// ===== Tipos =====
typedef uint8_t disabled_state_t;
enum {
    DISABLED_IDLE,
    DISABLED_PRESSED
};

// ===== Funções =====

// Processa eventos de teclas disabled
bool disabled_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// Atualiza indicadores RGB das teclas disabled (pulsação em branco)
void disabled_update_indicators(void);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void disabled_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void disabled_init_hooks(void);

#endif // DISABLED_DISABLED_H
