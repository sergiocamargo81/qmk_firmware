#include "behavior.h"
#include "kind.h"

// ===== Estado de Keymod =====

// Keymod atual mantido por behavior (atualizado pelo módulo modifiers)
static keymod_t g_current_keymod = KEYMOD_NONE;

// ===== API: Baseada em posição (keypos_t) =====

// Obtém a key associada a uma posição (row, col)
// Retorna ponteiro para base_key_t se encontrou, NULL caso contrário
base_key_t* behavior_get_key_by_position(uint8_t row, uint8_t col) {
    base_key_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->process_key == NULL) {
        return NULL;
    }
    return entry;
}

// ===== Gerenciamento de Keymod =====

// Atualiza o keymod atual mantido por behavior
// Chamado pelo módulo modifiers quando o keymod muda
void behavior_update_keymod(keymod_t keymod) {
    g_current_keymod = keymod;
}

// Obtém o keymod atual mantido por behavior
keymod_t behavior_get_current_keymod(void) {
    return g_current_keymod;
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
bool behavior_register_position_function(uint8_t row, uint8_t col, process_key_t function) {
    return kind_register_function(row, col, function);
    }
