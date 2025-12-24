#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include QMK_KEYBOARD_H
#include "kind.h"        // Para base_t, position_t, custom_t, key_function_t
#include "keymod.h"      // Para keymod_t

// ===== API: Baseada em posição (keypos_t) =====

// Resolve o handler associado a uma posição (row, col)
// Retorna true se encontrou behavior associado e preenche handler
// Retorna false se não encontrou behavior associado
// Usado por process_record_user para rotear para o submódulo correto
bool behavior_resolve_handler_by_position(uint8_t row, uint8_t col, key_function_t* handler, keymod_t* supported_keymod);

// ===== Funções para submódulos =====

// Obtém custom_t por custom_index (0-47)
custom_t* behavior_get_custom_by_index(uint8_t custom_index);


// Verifica se uma posição pode ter behavior custom
bool behavior_position_is_custom(uint8_t row, uint8_t col);

// ===== Sistema de Registro (para submódulos) =====

// Registra uma função para uma posição específica (row, col)
// Deve ser chamado durante {modulo}_keyboard_post_init_user dos submódulos
// Permite busca O(1) na matriz de funções
bool behavior_register_position_function(uint8_t row, uint8_t col, key_function_t function);

#endif // BEHAVIOR_H
