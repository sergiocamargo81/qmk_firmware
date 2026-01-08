#ifndef PERSISTENCE_PERSISTENCE_H
#define PERSISTENCE_PERSISTENCE_H

#include QMK_KEYBOARD_H

// ===== Tipos =====

// Estados possíveis da persistência
typedef enum {
    PERSISTENCE_SAVED,          // Estado normal (sem diferença entre persisted e working, persisted == working) - branco pulsante
    PERSISTENCE_DIFF,           // Há diferença entre persisted e working (verde pulsante)
    PERSISTENCE_PRESSED,        // END está pressionado (sem FN) - verde contínuo
    PERSISTENCE_SAVING,         // Salvando (amarelo sólido por 3 segundos)
    PERSISTENCE_BLINKING_SAVED, // Verde sólido após salvar com sucesso (3 segundos)
    PERSISTENCE_BLINKING_ERROR  // Vermelho sólido após erro ao salvar (3 segundos)
} persistence_state_t;

// ===== Funções =====

// Salva settings na EEPROM
bool persistence_save(void);

// Carrega settings da EEPROM
bool persistence_load(void);

// Atualiza estado da persistência (chamado em matrix_scan)
void persistence_update_state(void);

// Atualiza indicadores RGB da persistência
void persistence_update_indicators(void);

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void persistence_init_early_hooks(void);

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void persistence_init_hooks(void);

#endif // PERSISTENCE_PERSISTENCE_H
