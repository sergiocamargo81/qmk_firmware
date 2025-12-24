#include "custom.h"

#include "../profiles.h"
#include "../settings.h"
#include "../persist.h"  // Para persist_read_settings, PERSIST_MAGIC_0, PERSIST_MAGIC_1, SETTINGS_VERSION
#include "../kind.h"
#include "../keymap.h"
#include "../behavior.h"
#include "../hooks.h"
#include "../customs.h"  // Para customs_t
#include "eeconfig.h"  // Para EECONFIG_USER_DATABLOCK, eeconfig_update_user_datablock
#include "eeprom.h"  // Para eeprom_read_block
#include <string.h>  // Para memcmp, memset

// ===== Callbacks Registrados =====

// Array de callbacks indexado pela posição de cada customs_t
// Tamanho igual ao número de elementos em customs_t (4)
// [0] = KEY_CUSTOM_DISABLED (não usado, mas mantido para indexação direta)
// [1] = KEY_CUSTOM_HOLD
// [2] = KEY_CUSTOM_TOGGLE
// [3] = KEY_CUSTOM_RESERVED
static custom_callbacks_t registered_callbacks[CUSTOMS_T_COUNT] = {0};
static bool callbacks_registered[CUSTOMS_T_COUNT] = {false};

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
}

// Callback para remover teclas do profile anterior
static bool custom_on_profile_changed_remove_callback(custom_t* custom, void* user_data) {
    profile_t* old_profile = (profile_t*)user_data;
    if (!old_profile) return true;
    
    // Obtém o behavior que a tecla tinha no profile anterior
    customs_t old_behavior = old_profile->behaviors[custom->custom_index];
    
    // Se tinha um behavior válido e callback registrado, notifica remoção
    // Isso permite que os submódulos (hold/toggle) desativem as teclas que estavam ativas
    if (old_behavior < CUSTOMS_T_COUNT && old_behavior != KEY_CUSTOM_DISABLED) {
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
        return KEY_CUSTOM_DISABLED;
    }
    
    settings_t* working = settings_get_working();
    profile_t* active_profile = profiles_get_active_profile(&working->profiles);
    if (!active_profile) {
        return KEY_CUSTOM_DISABLED;
    }
    
    return active_profile->behaviors[custom->custom_index];
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
    profile_t* active_profile = profiles_get_active_profile(&working->profiles);
    if (!active_profile) {
        return false;
    }
    
    customs_t current_value = active_profile->behaviors[custom->custom_index];
    if (current_value == behavior) {
        return true; // Já está no valor desejado
    }
    
    // Atualiza o profile working PRIMEIRO
    active_profile->behaviors[custom->custom_index] = behavior;
    
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
    
    // O matrix_scan_user cuidará de desativar o padrão RGB quando o profile não estiver vazio
    // Não fazemos nada aqui para evitar problemas de ordem de execução
    
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
    
    // Verifica se FN está ativo usando keymod (valores: KEYMOD_FN, KEYMOD_FN_RCTL ou KEYMOD_FN_RALT)
    if (keymod != KEYMOD_FN && keymod != KEYMOD_FN_RCTL && keymod != KEYMOD_FN_RALT) {
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
    if (keymod == KEYMOD_FN_RCTL) {
        if (current_behavior == KEY_CUSTOM_HOLD) {
            new_behavior = KEY_CUSTOM_DISABLED; // Desativa se já está HOLD
        } else {
            new_behavior = KEY_CUSTOM_HOLD; // Ativa HOLD (remove de TOGGLE se necessário)
        }
        custom_set_behavior_by_keypos(key, new_behavior);
        processed = true;
    }
    // FN+RALT+TECLA → TOGGLE
    else if (keymod == KEYMOD_FN_RALT) {
        if (current_behavior == KEY_CUSTOM_TOGGLE) {
            new_behavior = KEY_CUSTOM_DISABLED; // Desativa se já está TOGGLE
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
    if (behavior < CUSTOMS_T_COUNT && behavior != KEY_CUSTOM_DISABLED) {
        // Acessa callbacks registrados diretamente (função auxiliar em custom.c)
        custom_key_handler_t handler = custom_get_handler_for_behavior(behavior);
        if (handler != NULL) {
            // É um handler específico (hold/toggle), chama ele
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

// Hook keyboard_post_init_user
static void custom_keyboard_post_init_user(void) {
    // Inicializa custom (registra callback de mudança de profile)
    custom_init();
    
    // Registra todas as posições custom na matriz de behavior
    custom_register_positions();
}

// ===== Inicialização de Hooks =====

// Registra todos os hooks (chamado em keyboard_post_init_user)
// Usado em keymap.c, mas o compilador pode não ver a referência
__attribute__((used))
void custom_init_hooks(void) {
    hooks_keyboard_post_init_register(custom_keyboard_post_init_user);
    // eeconfig_init já foi registrado em keyboard_pre_init_user
}


