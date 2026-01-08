#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include QMK_KEYBOARD_H
#include "kind.h"        // Para base_key_t, profile_key_t, numlock_t, persistence_t, custom_t, process_key_t
#include "keymod.h"      // Para keymod_t

// ===== API: Baseada em posição (keypos_t) =====

// Obtém a key associada a uma posição (row, col)
// Retorna ponteiro para base_key_t se encontrou, NULL caso contrário
// Usado por process_record_user para obter a struct da key
base_key_t* behavior_get_key_by_position(uint8_t row, uint8_t col);

// ===== Gerenciamento de Keymod =====

// Atualiza o keymod atual mantido por behavior
// Chamado pelo módulo modifiers quando o keymod muda
void behavior_update_keymod(keymod_t keymod);

// Obtém o keymod atual mantido por behavior
keymod_t behavior_get_current_keymod(void);

// ===== Funções para submódulos =====

// Obtém custom_t por persist_index (0-47)
custom_t* behavior_get_custom_by_index(uint8_t persist_index);


// Verifica se uma posição pode ter behavior custom
bool behavior_position_is_custom(uint8_t row, uint8_t col);

// ===== Sistema de Registro (para submódulos) =====

// Registra uma função para uma posição específica (row, col)
// Deve ser chamado durante {modulo}_keyboard_post_init_user dos submódulos
// Permite busca O(1) na matriz de funções
bool behavior_register_position_function(uint8_t row, uint8_t col, process_key_t function);

#endif // BEHAVIOR_H
