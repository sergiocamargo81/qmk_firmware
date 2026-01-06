#include "custom.h"

#include "../profile.h"
#include "../settings.h"
#include "../persist.h"  // Para persist_read_settings, PERSIST_MAGIC_0, PERSIST_MAGIC_1, SETTINGS_VERSION
#include "../kind.h"
#include "../keymap.h"
#include "../keymod.h"  // Para keymod_t
#include "../behavior.h"
#include "../hooks.h"
#include "../customs.h"  // Para customs_t

// ===== Callbacks Registrados =====

// Array de callbacks indexado pela posição de cada customs_t
// Tamanho igual ao número de elementos em customs_t (4)
// [0] = KEY_CUSTOM_UNASSOCIATED
// [1] = KEY_CUSTOM_HOLD
// [2] = KEY_CUSTOM_TOGGLE
// [3] = KEY_CUSTOM_RESERVED
static custom_callbacks_t registered_callbacks[CUSTOMS_T_COUNT] = {0};
static bool callbacks_registered[CUSTOMS_T_COUNT] = {false};

// ===== Array Ordenado de Ponteiros =====
// Array de ponteiros para custom_t ordenado por tipo: hold, toggle, unassociated
// Permite iteração eficiente sobre cada tipo sem manter três arrays separados
static custom_t* custom_sorted_array[CUSTOM_COUNT] = {NULL};

// Índices de início e fim para cada tipo no array ordenado
// Ordem: hold, unassociated, toggle
// inicial = final = -1 significa que não há elementos do tipo
typedef struct {
    int16_t hold_start;         // Índice inicial de hold keys
    int16_t hold_end;           // Índice final de hold keys
    int16_t unassociated_start; // Índice inicial de unassociated keys
    int16_t unassociated_end;   // Índice final de unassociated keys
    int16_t toggle_start;       // Índice inicial de toggle keys
    int16_t toggle_end;         // Índice final de toggle keys
} custom_sorted_indices_t;

static custom_sorted_indices_t custom_sorted_indices = {
    .hold_start = -1, .hold_end = -1,
    .unassociated_start = -1, .unassociated_end = -1,
    .toggle_start = -1, .toggle_end = -1
};

// ===== Funções para Array Ordenado =====

// Declarações forward
static void custom_init_sorted_array(void);
static void custom_rebuild_sorted_array(void);

// Encontra o índice de uma tecla no array ordenado
// Retorna -1 se não encontrada
static int16_t custom_find_key_index_in_sorted_array(custom_t* target) {
    if (target == NULL) return -1;
    
    for (int16_t i = 0; i < CUSTOM_COUNT; i++) {
        if (custom_sorted_array[i] == target) {
            return i;
        }
    }
    return -1;
}

// Faz swap otimizado de uma tecla entre grupos adjacentes
// Ordem: hold, unassociated, toggle
static void custom_swap_key_in_sorted_array(custom_t* key, customs_t old_behavior, customs_t new_behavior) {
    if (key == NULL) return;
    
    int16_t key_idx = custom_find_key_index_in_sorted_array(key);
    if (key_idx == -1) {
        // Tecla não está no array, precisa reconstruir
        custom_rebuild_sorted_array();
        return;
    }
    
    // Caso 1: unassociated → toggle
    if (old_behavior == KEY_CUSTOM_UNASSOCIATED && new_behavior == KEY_CUSTOM_TOGGLE) {
        // Verifica se há elementos em unassociated e se key_idx está dentro do range
        if (custom_sorted_indices.unassociated_end >= custom_sorted_indices.unassociated_start &&
            key_idx >= custom_sorted_indices.unassociated_start && 
            key_idx <= custom_sorted_indices.unassociated_end) {
            int16_t last_unassociated_idx = custom_sorted_indices.unassociated_end;
            
            // Se key_idx já é a última, não precisa fazer swap
            if (key_idx != last_unassociated_idx) {
                // Faz swap com a última tecla de unassociated
                custom_t* temp = custom_sorted_array[key_idx];
                custom_sorted_array[key_idx] = custom_sorted_array[last_unassociated_idx];
                custom_sorted_array[last_unassociated_idx] = temp;
            }
            
            // Atualiza índices: unassociated diminui, toggle aumenta
            if (custom_sorted_indices.unassociated_start == custom_sorted_indices.unassociated_end) {
                // Era o último elemento de unassociated
                custom_sorted_indices.unassociated_start = -1;
                custom_sorted_indices.unassociated_end = -1;
            } else {
                custom_sorted_indices.unassociated_end--;
            }
            
            // Atualiza toggle
            if (custom_sorted_indices.toggle_start == -1) {
                // Toggle estava vazio, agora começa onde unassociated terminou
                custom_sorted_indices.toggle_start = last_unassociated_idx;
                custom_sorted_indices.toggle_end = last_unassociated_idx;
            } else {
                // Toggle já tinha elementos, expande para a esquerda
                custom_sorted_indices.toggle_start--;
            }
        } else {
            // Tecla não está no range esperado, reconstruir
            custom_rebuild_sorted_array();
        }
    }
    // Caso 2: hold → unassociated
    else if (old_behavior == KEY_CUSTOM_HOLD && new_behavior == KEY_CUSTOM_UNASSOCIATED) {
        // Verifica se há elementos em hold e se key_idx está dentro do range
        if (custom_sorted_indices.hold_end >= custom_sorted_indices.hold_start &&
            key_idx >= custom_sorted_indices.hold_start && 
            key_idx <= custom_sorted_indices.hold_end) {
            int16_t last_hold_idx = custom_sorted_indices.hold_end;
            
            // Se key_idx já é a última, não precisa fazer swap
            if (key_idx != last_hold_idx) {
                // Faz swap com a última tecla de hold
                custom_t* temp = custom_sorted_array[key_idx];
                custom_sorted_array[key_idx] = custom_sorted_array[last_hold_idx];
                custom_sorted_array[last_hold_idx] = temp;
            }
            
            // Atualiza índices: hold diminui, unassociated aumenta
            if (custom_sorted_indices.hold_start == custom_sorted_indices.hold_end) {
                // Era o último elemento de hold
                custom_sorted_indices.hold_start = -1;
                custom_sorted_indices.hold_end = -1;
            } else {
                custom_sorted_indices.hold_end--;
            }
            
            // Atualiza unassociated
            if (custom_sorted_indices.unassociated_start == -1) {
                // Unassociated estava vazio, agora começa onde hold terminou
                custom_sorted_indices.unassociated_start = last_hold_idx;
                custom_sorted_indices.unassociated_end = last_hold_idx;
            } else {
                // Unassociated já tinha elementos, expande para a esquerda
                custom_sorted_indices.unassociated_start--;
            }
        } else {
            // Tecla não está no range esperado, reconstruir
            custom_rebuild_sorted_array();
        }
    }
    // Caso 3: toggle → unassociated
    else if (old_behavior == KEY_CUSTOM_TOGGLE && new_behavior == KEY_CUSTOM_UNASSOCIATED) {
        // Verifica se há elementos em toggle e se key_idx está dentro do range
        if (custom_sorted_indices.toggle_end >= custom_sorted_indices.toggle_start &&
            key_idx >= custom_sorted_indices.toggle_start && 
            key_idx <= custom_sorted_indices.toggle_end) {
            int16_t first_toggle_idx = custom_sorted_indices.toggle_start;
            
            // Se key_idx já é a primeira, não precisa fazer swap
            if (key_idx != first_toggle_idx) {
                // Faz swap com a primeira tecla de toggle
                custom_t* temp = custom_sorted_array[key_idx];
                custom_sorted_array[key_idx] = custom_sorted_array[first_toggle_idx];
                custom_sorted_array[first_toggle_idx] = temp;
            }
            
            // Atualiza índices: toggle diminui, unassociated aumenta
            if (custom_sorted_indices.toggle_start == custom_sorted_indices.toggle_end) {
                // Era o último elemento de toggle
                custom_sorted_indices.toggle_start = -1;
                custom_sorted_indices.toggle_end = -1;
            } else {
                custom_sorted_indices.toggle_start++;
            }
            
            // Atualiza unassociated
            if (custom_sorted_indices.unassociated_start == -1) {
                // Unassociated estava vazio, agora começa onde toggle começava
                custom_sorted_indices.unassociated_start = first_toggle_idx;
                custom_sorted_indices.unassociated_end = first_toggle_idx;
            } else {
                // Unassociated já tinha elementos, expande para a direita
                custom_sorted_indices.unassociated_end++;
            }
        } else {
            // Tecla não está no range esperado, reconstruir
            custom_rebuild_sorted_array();
        }
    }
    // Caso 4: unassociated → hold
    else if (old_behavior == KEY_CUSTOM_UNASSOCIATED && new_behavior == KEY_CUSTOM_HOLD) {
        // Verifica se há elementos em unassociated e se key_idx está dentro do range
        if (custom_sorted_indices.unassociated_end >= custom_sorted_indices.unassociated_start &&
            key_idx >= custom_sorted_indices.unassociated_start && 
            key_idx <= custom_sorted_indices.unassociated_end) {
            int16_t first_unassociated_idx = custom_sorted_indices.unassociated_start;
            
            // Se key_idx já é a primeira, não precisa fazer swap
            if (key_idx != first_unassociated_idx) {
                // Faz swap com a primeira tecla de unassociated
                custom_t* temp = custom_sorted_array[key_idx];
                custom_sorted_array[key_idx] = custom_sorted_array[first_unassociated_idx];
                custom_sorted_array[first_unassociated_idx] = temp;
            }
            
            // Atualiza índices: unassociated diminui, hold aumenta
            if (custom_sorted_indices.unassociated_start == custom_sorted_indices.unassociated_end) {
                // Era o último elemento de unassociated
                custom_sorted_indices.unassociated_start = -1;
                custom_sorted_indices.unassociated_end = -1;
            } else {
                custom_sorted_indices.unassociated_start++;
            }
            
            // Atualiza hold
            if (custom_sorted_indices.hold_start == -1) {
                // Hold estava vazio, agora começa onde unassociated começava
                custom_sorted_indices.hold_start = first_unassociated_idx;
                custom_sorted_indices.hold_end = first_unassociated_idx;
            } else {
                // Hold já tinha elementos, expande para a direita
                custom_sorted_indices.hold_end++;
            }
        } else {
            // Tecla não está no range esperado, reconstruir
            custom_rebuild_sorted_array();
        }
    }
    // Caso 5: toggle → hold (passa por unassociated primeiro)
    else if (old_behavior == KEY_CUSTOM_TOGGLE && new_behavior == KEY_CUSTOM_HOLD) {
        // Primeiro: toggle → unassociated
        custom_swap_key_in_sorted_array(key, KEY_CUSTOM_TOGGLE, KEY_CUSTOM_UNASSOCIATED);
        // Depois: unassociated → hold
        custom_swap_key_in_sorted_array(key, KEY_CUSTOM_UNASSOCIATED, KEY_CUSTOM_HOLD);
    }
    // Caso 6: hold → toggle (passa por unassociated primeiro)
    else if (old_behavior == KEY_CUSTOM_HOLD && new_behavior == KEY_CUSTOM_TOGGLE) {
        // Primeiro: hold → unassociated
        custom_swap_key_in_sorted_array(key, KEY_CUSTOM_HOLD, KEY_CUSTOM_UNASSOCIATED);
        // Depois: unassociated → toggle
        custom_swap_key_in_sorted_array(key, KEY_CUSTOM_UNASSOCIATED, KEY_CUSTOM_TOGGLE);
    }
    // Outros casos: reconstruir array completo
    else {
        custom_rebuild_sorted_array();
    }
}

// Inicializa o array ordenado com TODOS os custom_t* da matrix
// Inicialmente, hold e toggle estão vazios, unassociated ocupa todo o array
// Ordem: hold, unassociated, toggle
static void custom_init_sorted_array(void) {
    // Preenche array com todos os custom_t*
    uint8_t count = 0;
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom != NULL) {
            custom_sorted_array[count++] = custom;
        }
    }
    
    // Inicializa índices: hold e toggle vazios, unassociated ocupa todo o array
    custom_sorted_indices.hold_start = -1;
    custom_sorted_indices.hold_end = -1;
    if (count > 0) {
        custom_sorted_indices.unassociated_start = 0;
        custom_sorted_indices.unassociated_end = count - 1;
    } else {
        custom_sorted_indices.unassociated_start = -1;
        custom_sorted_indices.unassociated_end = -1;
    }
    custom_sorted_indices.toggle_start = -1;
    custom_sorted_indices.toggle_end = -1;
}

// Reconstrói o array ordenado e atualiza os índices baseado nos behaviors atuais
// Ordem: hold, unassociated, toggle
// Deve ser chamado apenas quando necessário (após ler settings)
static void custom_rebuild_sorted_array(void) {
    // Primeira passagem: conta quantas teclas de cada tipo existem
    uint16_t hold_count = 0;
    uint16_t unassociated_count = 0;
    uint16_t toggle_count = 0;
    
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = custom_sorted_array[i];
        if (custom == NULL) continue;
        
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        
        if (behavior == KEY_CUSTOM_HOLD) {
            hold_count++;
        } else if (behavior == KEY_CUSTOM_UNASSOCIATED) {
            unassociated_count++;
        } else if (behavior == KEY_CUSTOM_TOGGLE) {
            toggle_count++;
        }
    }
    
    // Segunda passagem: reorganiza array ordenado (hold, unassociated, toggle)
    // Usa um array temporário para evitar sobrescrever durante reorganização
    custom_t* temp_array[CUSTOM_COUNT];
    // Inicializa array temporário com NULL
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        temp_array[i] = NULL;
    }
    
    uint16_t hold_idx = 0;
    uint16_t unassociated_idx = hold_count;
    uint16_t toggle_idx = hold_count + unassociated_count;
    
    // Preenche array temporário na ordem correta
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = custom_sorted_array[i];
        if (custom == NULL) continue;
        
        customs_t behavior = custom_get_behavior_by_position(custom->row, custom->col);
        
        if (behavior == KEY_CUSTOM_HOLD) {
            temp_array[hold_idx++] = custom;
        } else if (behavior == KEY_CUSTOM_UNASSOCIATED) {
            temp_array[unassociated_idx++] = custom;
        } else if (behavior == KEY_CUSTOM_TOGGLE) {
            temp_array[toggle_idx++] = custom;
        }
    }
    
    // Copia array temporário de volta
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_sorted_array[i] = temp_array[i];
    }
    
    // Define índices
    if (hold_count > 0) {
        custom_sorted_indices.hold_start = 0;
        custom_sorted_indices.hold_end = hold_count - 1;
    } else {
        custom_sorted_indices.hold_start = -1;
        custom_sorted_indices.hold_end = -1;
    }
    
    if (unassociated_count > 0) {
        custom_sorted_indices.unassociated_start = hold_count;
        custom_sorted_indices.unassociated_end = hold_count + unassociated_count - 1;
    } else {
        custom_sorted_indices.unassociated_start = -1;
        custom_sorted_indices.unassociated_end = -1;
    }
    
    if (toggle_count > 0) {
        custom_sorted_indices.toggle_start = hold_count + unassociated_count;
        custom_sorted_indices.toggle_end = hold_count + unassociated_count + toggle_count - 1;
    } else {
        custom_sorted_indices.toggle_start = -1;
        custom_sorted_indices.toggle_end = -1;
    }
}


// ===== Callback para Mudança de Profile =====

// Declarações forward
static bool custom_on_profile_changed_callback(custom_t* custom, void* user_data);
static bool custom_on_profile_changed_remove_callback(custom_t* custom, void* user_data);
static bool custom_register_callbacks_callback(custom_t* custom, void* user_data);
static bool custom_register_positions_callback(custom_t* custom, void* user_data);

// Função chamada quando o profile ativo muda
static void custom_on_profile_changed(profile_t* old_profile, profile_t* new_profile) {
    // Primeiro: itera sobre todas as teclas custom e remove callbacks do profile anterior
    // Isso garante que toggle/hold que estavam enviando valores parem imediatamente
    // quando o profile muda, antes de ativar as teclas do novo profile
    if (old_profile) {
        kind_iterate_customs(custom_on_profile_changed_remove_callback, old_profile);
    }
    
    // Depois: itera sobre todas as teclas custom e atualiza handlers para o novo profile
    kind_iterate_customs(custom_on_profile_changed_callback, NULL);
    
    // Reconstrói array ordenado após mudança de profile
    custom_rebuild_sorted_array();
}

// Callback para remover teclas do profile anterior
static bool custom_on_profile_changed_remove_callback(custom_t* custom, void* user_data) {
    profile_t* old_profile = (profile_t*)user_data;
    if (!old_profile) return true;
    
    // Obtém o behavior que a tecla tinha no profile anterior
    customs_t old_behavior = old_profile->behaviors[custom->persist_index];
    
    // Se tinha um behavior válido e callback registrado, notifica remoção
    // Isso permite que os submódulos (hold/toggle/unassociated) desativem as teclas que estavam ativas
    if (old_behavior < CUSTOMS_T_COUNT && old_behavior != KEY_CUSTOM_UNASSOCIATED) {
        if (callbacks_registered[old_behavior]) {
            if (registered_callbacks[old_behavior].remove_callback) {
                registered_callbacks[old_behavior].remove_callback(custom->row, custom->col, old_behavior);
            }
        }
    }
    return true;
}

static bool custom_on_profile_changed_callback(custom_t* custom, void* user_data) {
    customs_t new_behavior = custom_get_behavior_by_position(custom->row, custom->col);
    
    // Sempre mantém custom_process_record_user registrado para permitir associação
    // O handler específico será chamado dentro de custom_process_record_user se necessário
    extern bool custom_process_record_user(keyrecord_t *record, keymod_t keymod);
    behavior_register_position_function(custom->row, custom->col, custom_process_record_user);
    
    // Se tem behavior válido e callback registrado, notifica adição
    if (new_behavior < CUSTOMS_T_COUNT) {
        if (callbacks_registered[new_behavior]) {
            // Chama callback de adição (módulo deve tratar como nova tecla)
            if (registered_callbacks[new_behavior].add_callback) {
                registered_callbacks[new_behavior].add_callback(custom->row, custom->col, new_behavior);
            }
        }
    }
    return true;
}

// ===== API de Consulta =====

customs_t custom_get_behavior_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) {
        return KEY_CUSTOM_UNASSOCIATED;
    }
    
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile) {
        return KEY_CUSTOM_UNASSOCIATED;
    }
    
    return active_profile->behaviors[custom->persist_index];
}

customs_t custom_get_behavior_by_keypos(keypos_t key) {
    return custom_get_behavior_by_position(key.row, key.col);
}

uint16_t custom_get_keycode(uint8_t row, uint8_t col) {
    return keymap_key_to_keycode(WIN_BASE, (keypos_t){.row = row, .col = col});
}

// ===== API de Modificação =====

bool custom_set_behavior_by_position(uint8_t row, uint8_t col, customs_t behavior) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) {
        return false;
    }
    
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile) {
        return false;
    }
    
    customs_t current_value = active_profile->behaviors[custom->persist_index];
    if (current_value == behavior) {
        return true; // Já está no valor desejado
    }
    
    // Atualiza o profile working PRIMEIRO
    active_profile->behaviors[custom->persist_index] = behavior;
    
    // Notifica remoção do behavior atual (se houver callback registrado)
    // Indexa diretamente pelo valor do enum
    if (current_value < CUSTOMS_T_COUNT) {
        if (callbacks_registered[current_value] && registered_callbacks[current_value].remove_callback) {
            registered_callbacks[current_value].remove_callback(row, col, current_value);
        }
    }
    
    // Notifica adição do novo behavior (se houver callback registrado)
    // Indexa diretamente pelo valor do enum
    // custom_process_record_user sempre permanece registrado para permitir associação
    if (behavior < CUSTOMS_T_COUNT) {
        if (callbacks_registered[behavior] && registered_callbacks[behavior].add_callback) {
            registered_callbacks[behavior].add_callback(row, col, behavior);
        }
    }
    
    // Atualiza array ordenado usando swap otimizado
    custom_swap_key_in_sorted_array(custom, current_value, behavior);
    
    return true;
}

bool custom_set_behavior_by_keypos(keypos_t key, customs_t behavior) {
    return custom_set_behavior_by_position(key.row, key.col, behavior);
}

// ===== Sistema de Registro de Callbacks =====

bool custom_register_callbacks(customs_t behavior, custom_callbacks_t callbacks) {
    // Valida se o behavior está dentro do range válido
    if (behavior >= CUSTOMS_T_COUNT) {
        return false; // Behavior inválido
    }
    
    // Permite registrar DISABLED também (mesmo que não seja usado normalmente)
    if (callbacks_registered[behavior]) {
        return false; // Já registrado
    }
    
    // Indexa diretamente pelo valor do enum
    registered_callbacks[behavior] = callbacks;
    callbacks_registered[behavior] = true;
    
    // Itera sobre todas as teclas custom e chama add_callback para as que têm este behavior
    kind_iterate_customs(custom_register_callbacks_callback, &behavior);
    
    return true;
}

static bool custom_register_callbacks_callback(custom_t* custom, void* user_data) {
    customs_t behavior = *(customs_t*)user_data;
    customs_t current_behavior = custom_get_behavior_by_position(custom->row, custom->col);
    if (current_behavior == behavior) {
        // Chama callback de adição
        // O handler será chamado dentro de custom_process_record_user quando necessário
        if (registered_callbacks[behavior].add_callback) {
            registered_callbacks[behavior].add_callback(custom->row, custom->col, behavior);
        }
    }
    return true;
}


// ===== Função Auxiliar para Resolução de Handler =====

// Retorna o handler registrado para um behavior (usado por custom_process_record_user)
// Retorna NULL se não houver handler registrado
custom_key_handler_t custom_get_handler_for_behavior(customs_t behavior) {
    if (behavior >= CUSTOMS_T_COUNT) {
        return NULL;
    }
    
    if (callbacks_registered[behavior]) {
        return registered_callbacks[behavior].key_handler;
    }
    
    return NULL;
}

// ===== Processamento de Associação =====

// Processa associação de custom (FN+RCTL+TECLA ou FN+RALT+TECLA) - baseado em posição, não keycode
// Retorna true se processou (consome o evento), false caso contrário
static bool custom_process_record_association(keyrecord_t *record, keymod_t keymod) {
    if (!record->event.pressed) return false;
    
    // Verifica se FN está ativo usando keymod (deve ser KEYMOD_FN_ONLY, KEYMOD_FN_RCTL ou KEYMOD_FN_RALT)
    if (!keymod_has_value(keymod, KEYMOD_FN_ONLY | KEYMOD_FN_RCTL | KEYMOD_FN_RALT)) {
        return false;
    }
    
    // Verifica se a posição é custom
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) {
        return false; // Não é custom
    }
    
    keypos_t key = record->event.key;
    customs_t current_behavior = custom_get_behavior_by_keypos(key);
    customs_t new_behavior;
    bool processed = false;
    
    // FN+RCTL+TECLA → HOLD
    if (keymod_is_only_value(keymod, KEYMOD_FN_RCTL)) {
        if (current_behavior == KEY_CUSTOM_HOLD) {
            new_behavior = KEY_CUSTOM_UNASSOCIATED; // Desativa se já está HOLD
        } else {
            new_behavior = KEY_CUSTOM_HOLD; // Ativa HOLD (remove de TOGGLE se necessário)
        }
        custom_set_behavior_by_keypos(key, new_behavior);
        processed = true;
    }
    // FN+RALT+TECLA → TOGGLE
    else if (keymod_is_only_value(keymod, KEYMOD_FN_RALT)) {
        if (current_behavior == KEY_CUSTOM_TOGGLE) {
            new_behavior = KEY_CUSTOM_UNASSOCIATED; // Desativa se já está TOGGLE
        } else {
            new_behavior = KEY_CUSTOM_TOGGLE; // Ativa TOGGLE (remove de HOLD se necessário)
        }
        custom_set_behavior_by_keypos(key, new_behavior);
        processed = true;
    }
    
    return processed; // Retorna true se processou (consome o evento)
}

// Processa custom (associação + handlers) - baseado em posição, não keycode
// Retorna false se consumiu o evento, true caso contrário
bool custom_process_record_user(keyrecord_t *record, keymod_t keymod) {
    // Obtém posição
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // KC_SPC tem tratamento especial: sempre desativa hold/toggle quando pressionado
    // Isso funciona mesmo se KC_SPC não estiver associado a HOLD ou TOGGLE
    if (row == 5 && col == 6 && record->event.pressed) {  // KC_SPC está em (5, 6)
        extern void deactivate_all_hold(void);
        extern void toggle_deactivate_all(void);
        deactivate_all_hold();
        toggle_deactivate_all();
    }
    
    // Primeiro tenta processar associação (FN+RCTL+TECLA ou FN+RALT+TECLA)
    if (custom_process_record_association(record, keymod)) {
        return false; // Consumido pela associação
    }
    
    // Se não foi associação, resolve handler específico baseado no behavior atual
    // Consulta os callbacks registrados diretamente (não via behavior para evitar recursão)
    customs_t behavior = custom_get_behavior_by_position(row, col);
    
    // Se tem behavior válido, tenta chamar o handler do callback registrado
    if (behavior < CUSTOMS_T_COUNT) {
        // Acessa callbacks registrados diretamente (função auxiliar em custom.c)
        custom_key_handler_t handler = custom_get_handler_for_behavior(behavior);
        if (handler != NULL) {
            // É um handler específico (hold/toggle/unassociated), chama ele
            return handler(record, keymod);
        }
    }
    
    return true; // Passa adiante
}

// ===== Funções de Registro =====

// Registra todas as posições custom na matriz de behavior
void custom_register_positions(void) {
    extern bool custom_process_record_user(keyrecord_t *record, keymod_t keymod);
    // Itera sobre todas as teclas custom e registra função
    kind_iterate_customs(custom_register_positions_callback, custom_process_record_user);
}

static bool custom_register_positions_callback(custom_t* custom, void* user_data) {
    key_function_t function = (key_function_t)user_data;
    // Registra a função custom_process_record_user para todas as posições custom
    behavior_register_position_function(custom->row, custom->col, function);
    return true;
}

// ===== Inicialização =====

// Deve ser chamado durante custom_keyboard_post_init_user
void custom_init(void) {
    // Registra callback para mudança de profile
    settings_register_profile_changed_callback(custom_on_profile_changed);
}

// ===== Funções de Hook QMK =====
// Nota: Callbacks de FN são gerenciados diretamente por cada módulo (hold, toggle, unassociated)
// através de modifiers_register_fn_callback em keymap.c

// Hook keyboard_post_init_user
static void custom_keyboard_post_init_user(void) {
    // Inicializa custom (registra callback de mudança de profile)
    custom_init();
    
    // Inicializa array ordenado com todos os custom_t* (todos em unassociated inicialmente)
    custom_init_sorted_array();
    
    // Registra todas as posições custom na matriz de behavior
    custom_register_positions();
    
    // Reconstrói array ordenado baseado nos behaviors lidos de settings
    custom_rebuild_sorted_array();
}

// ===== Inicialização de Hooks =====

// Registra todos os hooks (chamado em keyboard_post_init_user)
// Usado em keymap.c, mas o compilador pode não ver a referência
__attribute__((used))
void custom_init_hooks(void) {
    hooks_keyboard_post_init_register(custom_keyboard_post_init_user);
    // LEDs são gerenciados pelos submódulos (hold, toggle, unassociated)
    // eeconfig_init já foi registrado em keyboard_pre_init_user
}


