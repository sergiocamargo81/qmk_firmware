#ifndef PERSISTENCE_PERSISTENCE_H
#define PERSISTENCE_PERSISTENCE_H

#include QMK_KEYBOARD_H

// ===== Tipos =====

// Estados possíveis da persistência
typedef enum {
    PERSISTENCE_IDLE,           // Estado normal (sem diferença entre persisted e working) - branco pulsante
    PERSISTENCE_DIFF,           // Há diferença entre persisted e working (verde pulsante)
    PERSISTENCE_PRESSED,        // END está pressionado (sem FN) - verde contínuo
    PERSISTENCE_SAVING,         // Salvando (amarelo sólido por 3 segundos)
    PERSISTENCE_BLINKING_SAVED, // Verde sólido após salvar com sucesso (3 segundos)
    PERSISTENCE_BLINKING_ERROR  // Vermelho sólido após erro ao salvar (3 segundos)
} persistence_state_t;

// Estado do módulo persistence
typedef struct {
    persistence_state_t state;  // Estado atual
    uint8_t led_index;          // LED index para KC_END
    uint32_t save_timer;        // Timer para controlar feedback visual (3 segundos)
    bool save_success;           // Resultado do último salvamento (true = sucesso, false = erro)
    bool led_initialized;       // Se LED foi inicializado
    uint8_t kc_end_row;         // Cache da posição de KC_END (row)
    uint8_t kc_end_col;          // Cache da posição de KC_END (col)
} persistence_t;

// ===== Variáveis Globais =====
extern persistence_t persistence;

// ===== Funções =====

// Registra a posição de KC_END na matriz de behavior
// Deve ser chamado durante persistence_keyboard_post_init_user
void persistence_register_position(void);

// Inicialização
void persistence_init_led_index(void);

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
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void persistence_init_hooks(void);

#endif // PERSISTENCE_PERSISTENCE_H
