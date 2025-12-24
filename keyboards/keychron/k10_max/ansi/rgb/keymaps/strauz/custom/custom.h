#ifndef CUSTOM_CUSTOM_H
#define CUSTOM_CUSTOM_H

#include QMK_KEYBOARD_H
#include "../customs.h"  // Para customs_t
#include "../keymod.h"   // Para keymod_t

// ===== Tipos =====

// Tipo de função para processar eventos de tecla pressionada
typedef bool (*custom_key_handler_t)(keyrecord_t *record, keymod_t keymod);

// Tipo de callback para notificar inclusão de tecla no controle do módulo
// row, col: posição da tecla
// behavior: behavior da tecla (HOLD, TOGGLE, etc)
typedef void (*custom_key_add_callback_t)(uint8_t row, uint8_t col, customs_t behavior);

// Tipo de callback para notificar remoção de tecla do controle do módulo
// row, col: posição da tecla
// behavior: behavior anterior da tecla
typedef void (*custom_key_remove_callback_t)(uint8_t row, uint8_t col, customs_t behavior);

// Estrutura de callbacks para um tipo de custom
typedef struct {
    custom_key_handler_t key_handler;        // Função para tratar tecla pressionada
    custom_key_add_callback_t add_callback;  // Função para notificar inclusão de tecla
    custom_key_remove_callback_t remove_callback; // Função para notificar remoção de tecla
} custom_callbacks_t;

// ===== API para consulta =====

// Obtém o custom atual de uma tecla por posição
customs_t custom_get_behavior_by_position(uint8_t row, uint8_t col);
customs_t custom_get_behavior_by_keypos(keypos_t key);

// Obtém o keycode da layer BASE para uma posição row/col
uint16_t custom_get_keycode(uint8_t row, uint8_t col);

// ===== API para modificação =====

// Define o custom de uma tecla por posição
// Atualiza o profile working e notifica os callbacks apropriados
bool custom_set_behavior_by_position(uint8_t row, uint8_t col, customs_t behavior);
bool custom_set_behavior_by_keypos(keypos_t key, customs_t behavior);

// ===== Sistema de Registro de Callbacks =====

// Registra callbacks para um tipo de custom (HOLD, TOGGLE, etc)
// Deve ser chamado durante {modulo}_keyboard_post_init_user
// Retorna true se sucesso, false se behavior inválido ou já registrado
// Após registro, custom itera sobre todas as teclas com este behavior e chama add_callback
bool custom_register_callbacks(customs_t behavior, custom_callbacks_t callbacks);

// ===== Função Auxiliar para Resolução de Handler =====

// Retorna o handler registrado para um behavior (usado internamente por custom_process_record_user)
// Retorna NULL se não houver handler registrado
custom_key_handler_t custom_get_handler_for_behavior(customs_t behavior);

// ===== Processamento de Eventos =====

// Processa custom (associação + handlers) - baseado em posição, não keycode
// Retorna false se consumiu o evento, true caso contrário
bool custom_process_record_user(keyrecord_t *record, keymod_t keymod);

// ===== Funções de Registro =====

// Registra todas as posições custom na matriz de behavior
void custom_register_positions(void);

// ===== Inicialização =====

// Deve ser chamado durante custom_keyboard_post_init_user
void custom_init(void);

// ===== Inicialização de Hooks =====

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void custom_init_hooks(void);

#endif // CUSTOM_CUSTOM_H
