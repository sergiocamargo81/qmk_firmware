#include "behavior.h"
#include "kind.h"

// ===== API: Baseada em posição (keypos_t) =====

// Resolve o handler associado a uma posição (row, col)
// Retorna true se encontrou behavior associado e preenche handler e supported_keymod
// Retorna false se não encontrou behavior associado
bool behavior_resolve_handler_by_position(uint8_t row, uint8_t col, key_function_t* handler, keymod_t* supported_keymod) {
    if (handler == NULL || supported_keymod == NULL) {
        return false;
    }
    
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL) {
        return false;
    }
    
    if (entry->kind == KIND_CUSTOM) {
        custom_t* custom = (custom_t*)entry;
        if (custom->function != NULL) {
            *handler = custom->function;
            *supported_keymod = custom->supported_keymod;
            return true;
        }
    } else if (entry->kind == KIND_POSITION) {
        position_t* position = (position_t*)entry;
        if (position->function != NULL) {
            *handler = position->function;
            *supported_keymod = position->supported_keymod;
            return true;
        }
    }
    
    return false;
}

// ===== Funções para submódulos =====

// Obtém custom_t por custom_index (0-47)
custom_t* behavior_get_custom_by_index(uint8_t custom_index) {
    return kind_get_custom_by_index(custom_index);
}


// Verifica se uma posição pode ter behavior custom
bool behavior_position_is_custom(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    return custom != NULL;
}

// ===== Sistema de Registro =====

// Registra uma função para uma posição específica (row, col)
// Permite busca O(1) na matriz de funções
bool behavior_register_position_function(uint8_t row, uint8_t col, key_function_t function) {
    return kind_register_function(row, col, function);
    }
