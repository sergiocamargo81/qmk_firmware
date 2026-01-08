#ifndef CUSTOM_CUSTOM_H
#define CUSTOM_CUSTOM_H

#include QMK_KEYBOARD_H
#include "../custom_behaviors.h"  // Para custom_behaviors_t
#include "../keymod.h"   // Para keymod_t
#include "../kind.h"     // Para base_key_t

// ===== Tipos =====

// Tipo de função para processar eventos de tecla pressionada
typedef bool (*custom_key_handler_t)(base_key_t* key, bool pressed, keymod_t keymod);

// ===== API para consulta =====

// Obtém o custom atual de uma tecla por posição
custom_behaviors_t custom_get_behavior_by_position(uint8_t row, uint8_t col);
custom_behaviors_t custom_get_behavior_by_keypos(keypos_t key);

// Obtém o keycode da layer BASE para uma posição row/col
uint16_t custom_get_keycode(uint8_t row, uint8_t col);

// ===== API para modificação =====

// Define o custom de uma tecla por posição
// Atualiza o profile working e notifica os callbacks apropriados
bool custom_set_behavior_by_position(uint8_t row, uint8_t col, custom_behaviors_t behavior);
bool custom_set_behavior_by_keypos(keypos_t key, custom_behaviors_t behavior);

// ===== Sistema de Registro de Handlers =====

// Registra handler para um behavior (usado apenas para processamento de teclas)
// ADDED/REMOVED são notificados via Event Bus (EVENT_CUSTOM_BEHAVIOR_ADDED/REMOVED)
// Deve ser chamado durante {modulo}_keyboard_post_init_user
// Retorna true se sucesso, false se behavior inválido ou já registrado
// Após registro, custom dispara eventos ADDED para todas as teclas com este behavior
bool custom_register_handler(custom_behaviors_t behavior, custom_key_handler_t handler);

// ===== Função Auxiliar para Resolução de Handler =====

// Retorna o handler registrado para um behavior (usado internamente por custom_process_key)
// Retorna NULL se não houver handler registrado
custom_key_handler_t custom_get_handler_for_behavior(custom_behaviors_t behavior);

// ===== Processamento de Eventos =====

// Processa custom (associação + handlers) - baseado em posição, não keycode
// Retorna false se consumiu o evento, true caso contrário
bool custom_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// ===== Funções de Registro =====

// Registra todas as posições custom na matriz de behavior
void custom_register_positions(void);

// ===== Sincronização de Cache =====

// Atualiza o cache de todas as teclas custom com os behaviors do profile ativo
// Útil quando o profile muda ou na inicialização
void custom_sync_all_caches(void);

// ===== Inicialização =====

// Deve ser chamado durante custom_keyboard_post_init_user
void custom_init(void);

// ===== Callback de Notificação de FN =====
// Nota: Callbacks de FN são gerenciados diretamente por cada módulo (hold, toggle, unassociated)
// através de Event Bus em keymap.c

// ===== Inicialização de Hooks =====

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void custom_init_hooks(void);

#endif // CUSTOM_CUSTOM_H
