#include "behavior.h"
#include "kind.h"

// ===== API: Baseada em posição (keypos_t) =====

// Resolve o handler associado a uma posição (row, col)
// Retorna true se encontrou behavior associado e preenche handler e accepted_keymods
// Retorna false se não encontrou behavior associado
bool behavior_resolve_handler_by_position(uint8_t row, uint8_t col, key_function_t* handler, keymod_t* accepted_keymods) {
    if (handler == NULL || accepted_keymods == NULL) {
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
            *accepted_keymods = custom->accepted_keymods;
            return true;
        }
    } else if (entry->kind == KIND_PROFILE) {
        profile_key_t* profile = (profile_key_t*)entry;
        if (profile->function != NULL) {
            *handler = profile->function;
            *accepted_keymods = profile->accepted_keymods;
            return true;
        }
    } else if (entry->kind == KIND_NUMLOCK) {
        numlock_t* numlock = (numlock_t*)entry;
        if (numlock->function != NULL) {
            *handler = numlock->function;
            *accepted_keymods = numlock->accepted_keymods;
            return true;
        }
    } else if (entry->kind == KIND_PERSISTENCE) {
        persistence_key_t* persistence = (persistence_key_t*)entry;
        if (persistence->function != NULL) {
            *handler = persistence->function;
            *accepted_keymods = persistence->accepted_keymods;
            return true;
        }
    } else if (entry->kind == KIND_MODIFIER) {
        modifier_t* modifier = (modifier_t*)entry;
        if (modifier->function != NULL) {
            *handler = modifier->function;
            *accepted_keymods = KEYMOD_NONE;  // Modifiers não têm restrição de keymod
            return true;
        }
    } else if (entry->kind == KIND_OTHER) {
        others_t* others = (others_t*)entry;
        if (others->function != NULL) {
            *handler = others->function;
            *accepted_keymods = KEYMOD_NONE;  // Others não têm restrição de keymod
            return true;
        }
    } else if (entry->kind == KIND_DISABLED) {
        disabled_t* disabled = (disabled_t*)entry;
        if (disabled->function != NULL) {
            *handler = disabled->function;
            *accepted_keymods = KEYMOD_NONE;  // Disabled não têm restrição de keymod
            return true;
        }
    } else if (entry->kind == KIND_BOLD) {
        bold_t* bold = (bold_t*)entry;
        if (bold->function != NULL) {
            *handler = bold->function;
            *accepted_keymods = KEYMOD_NONE;  // Bold não têm restrição de keymod
            return true;
        }
    }
    
    return false;
}

// ===== Funções para submódulos =====

// Obtém custom_t por persist_index (0-47)
custom_t* behavior_get_custom_by_index(uint8_t persist_index) {
    return kind_get_custom_by_index(persist_index);
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
