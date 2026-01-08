#include "custom.h"

#include "../profile.h"
#include "../settings.h"
#include "../persist.h"  // Para persist_read_settings, PERSIST_MAGIC_0, PERSIST_MAGIC_1, SETTINGS_VERSION
#include "../kind.h"
#include "../keymap.h"
#include "../keymod.h"  // Para keymod_t
#include "../behavior.h"
#include "../event_bus.h"  // Para Event Bus
#include "../custom_behaviors.h"  // Para custom_behaviors_t

// ===== Handlers Registrados =====

// Array de handlers indexado pela posição de cada custom_behaviors_t
// Tamanho igual ao número de elementos em custom_behaviors_t (4)
// [0] = CUSTOM_BEHAVIOR_UNASSOCIATED
// [1] = CUSTOM_BEHAVIOR_HOLD
// [2] = CUSTOM_BEHAVIOR_TOGGLE
// [3] = CUSTOM_BEHAVIOR_RESERVED
// Usado apenas para processamento de teclas (key_handler)
// ADDED/REMOVED são notificados via Event Bus
static custom_key_handler_t registered_handlers[CUSTOM_BEHAVIORS_T_COUNT] = {NULL};
static bool handlers_registered[CUSTOM_BEHAVIORS_T_COUNT] = {false};

// ===== Funções de Sincronização de Cache =====

// Atualiza o cache de uma tecla específica
// Sempre sincroniza com o valor em active_profile->behaviors[persist_index]
static void custom_sync_cache(custom_t* custom) {
    if (custom == NULL) return;
    
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    
    if (active_profile) {
        custom->custom_behavior = active_profile->behaviors[custom->persist_index];
    } else {
        custom->custom_behavior = CUSTOM_BEHAVIOR_UNASSOCIATED;
    }
}

// Callback para iteração de sincronização de cache
static bool custom_sync_cache_callback(custom_t* custom, void* user_data) {
    custom_sync_cache(custom);
    return true;
}

// Atualiza o cache de todas as teclas custom
// Útil quando o profile muda ou na inicialização
void custom_sync_all_caches(void) {
    kind_iterate_customs(custom_sync_cache_callback, NULL);
}

// ===== Callback para Mudança de Profile =====

// Declarações forward
static bool custom_on_profile_changed_callback(custom_t* custom, void* user_data);
static bool custom_on_profile_changed_remove_callback(custom_t* custom, void* user_data);
static bool custom_register_positions_callback(custom_t* custom, void* user_data);
static bool custom_check_behavior_changes(custom_t* custom, void* user_data);

// Função chamada quando settings são carregados
static void custom_settings_loaded_handler(const event_t* event) {
    if (event->type != EVENT_SETTINGS_LOADED) return;

    // Itera todas as teclas custom e verifica mudanças de behavior
    kind_iterate_customs(custom_check_behavior_changes, NULL);
}

// Callback para verificar mudanças de behavior em cada tecla
static bool custom_check_behavior_changes(custom_t* custom, void* user_data) {
    custom_behaviors_t old_behavior = custom->custom_behavior;

    // Sincroniza cache com profile carregado
    custom_sync_cache(custom);

    custom_behaviors_t new_behavior = custom->custom_behavior;

    // Se behavior mudou, anuncia mudança individual
    if (old_behavior != new_behavior) {
        event_data_t data = {
            .custom_behavior_changed = {
                .row = custom->row,
                .col = custom->col,
                .old_behavior = old_behavior,
                .new_behavior = new_behavior
            }
        };
        event_bus_publish(EVENT_CUSTOM_BEHAVIOR_CHANGED, &data);
    }

    return true;
}

// Função chamada quando o profile ativo muda
static void custom_on_profile_changed(const event_t* event) {
    if (event->type != EVENT_PROFILE_CHANGED) return;
    profile_t* old_profile = (profile_t*)event->data.profile_changed.old_profile;

    // Primeiro: itera sobre todas as teclas custom e remove callbacks do profile anterior
    // Isso garante que toggle/hold que estavam enviando valores parem imediatamente
    // quando o profile muda, antes de ativar as teclas do novo profile
    if (old_profile) {
        kind_iterate_customs(custom_on_profile_changed_remove_callback, old_profile);
    }

    // Depois: itera sobre todas as teclas custom e atualiza handlers para o novo profile
    kind_iterate_customs(custom_on_profile_changed_callback, NULL);
    (void)event; // new_profile não é usado diretamente
}

// Callback para remover teclas do profile anterior
static bool custom_on_profile_changed_remove_callback(custom_t* custom, void* user_data) {
    profile_t* old_profile = (profile_t*)user_data;
    if (!old_profile) return true;
    
    // Obtém o behavior que a tecla tinha no profile anterior
    custom_behaviors_t old_behavior = old_profile->behaviors[custom->persist_index];
    
    // Se tinha um behavior válido, dispara evento de remoção via Event Bus
    // Isso permite que os submódulos (hold/toggle/unassociated) desativem as teclas que estavam ativas
    if (old_behavior < CUSTOM_BEHAVIORS_T_COUNT && old_behavior != CUSTOM_BEHAVIOR_UNASSOCIATED) {
        event_data_t data = {
            .custom_behavior = {
                .row = custom->row,
                .col = custom->col,
                .behavior = old_behavior
            }
        };
        event_bus_publish(EVENT_CUSTOM_BEHAVIOR_REMOVED, &data);
    }
    return true;
}

static bool custom_on_profile_changed_callback(custom_t* custom, void* user_data) {
    // Atualiza cache primeiro (sincroniza com o novo profile)
    custom_sync_cache(custom);

    // Agora usa o cache em vez de fazer lookup
    custom_behaviors_t new_behavior = custom->custom_behavior;

    // Sempre mantém custom_process_key registrado para permitir associação
    // O handler específico será chamado dentro de custom_process_key se necessário
    behavior_register_position_function(custom->row, custom->col, custom_process_key);

    // Se tem behavior válido (não UNASSOCIATED), dispara evento de adição via Event Bus
    // IMPORTANTE: Sempre dispara evento mesmo para UNASSOCIATED para garantir
    // que os submódulos atualizem seus estados corretamente
    if (new_behavior < CUSTOM_BEHAVIORS_T_COUNT) {
        event_data_t data = {
            .custom_behavior = {
                .row = custom->row,
                .col = custom->col,
                .behavior = new_behavior
            }
        };
        event_bus_publish(EVENT_CUSTOM_BEHAVIOR_ADDED, &data);
    }
    return true;
}

// ===== API de Consulta =====

custom_behaviors_t custom_get_behavior_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) {
        return CUSTOM_BEHAVIOR_UNASSOCIATED;
    }
    
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile) {
        return CUSTOM_BEHAVIOR_UNASSOCIATED;
    }
    
    return active_profile->behaviors[custom->persist_index];
}

custom_behaviors_t custom_get_behavior_by_keypos(keypos_t key) {
    return custom_get_behavior_by_position(key.row, key.col);
}

uint16_t custom_get_keycode(uint8_t row, uint8_t col) {
    return keymap_key_to_keycode(WIN_BASE, (keypos_t){.row = row, .col = col});
}

// ===== API de Modificação =====

bool custom_set_behavior_by_position(uint8_t row, uint8_t col, custom_behaviors_t behavior) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) {
        return false;
    }
    
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile) {
        return false;
    }
    
    custom_behaviors_t current_value = active_profile->behaviors[custom->persist_index];
    if (current_value == behavior) {
        return true; // Já está no valor desejado
    }
    
    // Atualiza o profile working PRIMEIRO
    active_profile->behaviors[custom->persist_index] = behavior;
    
    // Atualiza o cache imediatamente após modificar o behavior
    custom_sync_cache(custom);
    
    // Notifica remoção do behavior atual (se houver callback registrado)
    // Indexa diretamente pelo valor do enum
    if (current_value < CUSTOM_BEHAVIORS_T_COUNT) {
        // Dispara evento de remoção via Event Bus
        if (current_value < CUSTOM_BEHAVIORS_T_COUNT) {
            event_data_t data = {
                .custom_behavior = {
                    .row = row,
                    .col = col,
                    .behavior = current_value
                }
            };
            event_bus_publish(EVENT_CUSTOM_BEHAVIOR_REMOVED, &data);
        }
    }
    
    // Notifica adição do novo behavior (se houver callback registrado)
    // Indexa diretamente pelo valor do enum
    // custom_process_key sempre permanece registrado para permitir associação
    if (behavior < CUSTOM_BEHAVIORS_T_COUNT) {
        // Dispara evento de adição via Event Bus
        if (behavior < CUSTOM_BEHAVIORS_T_COUNT) {
            event_data_t data = {
                .custom_behavior = {
                    .row = row,
                    .col = col,
                    .behavior = behavior
                }
            };
            event_bus_publish(EVENT_CUSTOM_BEHAVIOR_ADDED, &data);
        }
    }
    
    return true;
}

bool custom_set_behavior_by_keypos(keypos_t key, custom_behaviors_t behavior) {
    return custom_set_behavior_by_position(key.row, key.col, behavior);
}

// ===== Sistema de Registro de Handlers =====

// Registra handler para um behavior (usado apenas para processamento de teclas)
// ADDED/REMOVED são notificados via Event Bus
// Retorna true se sucesso, false se behavior inválido ou já registrado
bool custom_register_handler(custom_behaviors_t behavior, custom_key_handler_t handler) {
    // Valida se o behavior está dentro do range válido
    if (behavior >= CUSTOM_BEHAVIORS_T_COUNT) {
        return false; // Behavior inválido
    }
    
    if (handlers_registered[behavior]) {
        return false; // Já registrado
    }
    
    // Registra handler
    registered_handlers[behavior] = handler;
    handlers_registered[behavior] = true;
    
    // Itera sobre todas as teclas custom e dispara evento ADDED para as que têm este behavior
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        if (custom->custom_behavior == behavior) {
            event_data_t data = {
                .custom_behavior = {
                    .row = custom->row,
                    .col = custom->col,
                    .behavior = behavior
                }
            };
            event_bus_publish(EVENT_CUSTOM_BEHAVIOR_ADDED, &data);
        }
    }
    
    return true;
}

// ===== Função Auxiliar para Resolução de Handler =====

// Retorna o handler registrado para um behavior (usado por custom_process_key)
// Retorna NULL se não houver handler registrado
custom_key_handler_t custom_get_handler_for_behavior(custom_behaviors_t behavior) {
    if (behavior >= CUSTOM_BEHAVIORS_T_COUNT) {
        return NULL;
    }
    
    if (handlers_registered[behavior]) {
        return registered_handlers[behavior];
    }
    
    return NULL;
}

// ===== Processamento de Associação =====

// Processa custom (associação + handlers) - baseado em posição, não keycode
// Retorna false se consumiu o evento, true caso contrário
bool custom_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (key == NULL || key->kind != KIND_CUSTOM) {
        return true;
    }
    
    // Primeiro tenta processar associação (FN+RCTL+TECLA ou FN+RALT+TECLA)
    if (pressed) {
        // Verifica se FN está ativo usando keymod (deve ser KEYMOD_FN_ONLY, KEYMOD_FN_RCTL ou KEYMOD_FN_RALT)
        if (keymod_intersects(keymod, KEYMOD_FN_ONLY | KEYMOD_FN_RCTL | KEYMOD_FN_RALT)) {
            custom_t* custom = (custom_t*)key;
            // Usa o cache diretamente
            custom_behaviors_t current_behavior = custom->custom_behavior;
            custom_behaviors_t new_behavior;
            bool processed = false;
            
            // FN+RCTL+TECLA → HOLD
            if (keymod_equals(keymod, KEYMOD_FN_RCTL)) {
                if (current_behavior == CUSTOM_BEHAVIOR_HOLD) {
                    new_behavior = CUSTOM_BEHAVIOR_UNASSOCIATED; // Desativa se já está HOLD
                } else {
                    new_behavior = CUSTOM_BEHAVIOR_HOLD; // Ativa HOLD (remove de TOGGLE se necessário)
                }
                custom_set_behavior_by_position(custom->row, custom->col, new_behavior);
                processed = true;
            }
            // FN+RALT+TECLA → TOGGLE
            else if (keymod_equals(keymod, KEYMOD_FN_RALT)) {
                if (current_behavior == CUSTOM_BEHAVIOR_TOGGLE) {
                    new_behavior = CUSTOM_BEHAVIOR_UNASSOCIATED; // Desativa se já está TOGGLE
                } else {
                    new_behavior = CUSTOM_BEHAVIOR_TOGGLE; // Ativa TOGGLE (remove de HOLD se necessário)
                }
                custom_set_behavior_by_position(custom->row, custom->col, new_behavior);
                processed = true;
            }
            
            if (processed) {
                return false; // Consumido pela associação
            }
        }
    }
    
    // Se não foi associação, resolve handler específico baseado no behavior atual
    // Consulta o cache diretamente (não via behavior para evitar recursão)
    custom_t* custom = (custom_t*)key;
    custom_behaviors_t behavior = custom->custom_behavior;
    
    // Se tem behavior válido, tenta chamar o handler do callback registrado
    if (behavior < CUSTOM_BEHAVIORS_T_COUNT) {
        // Acessa callbacks registrados diretamente (função auxiliar em custom.c)
        custom_key_handler_t handler = custom_get_handler_for_behavior(behavior);
        if (handler != NULL) {
            // É um handler específico (hold/toggle/unassociated), chama ele
            return handler(key, pressed, keymod);
        }
    }
    
    return true; // Passa adiante
}

// ===== Funções de Registro =====

// Registra todas as posições custom na matriz de behavior
void custom_register_positions(void) {
    // Itera sobre todas as teclas custom e registra função
    kind_iterate_customs(custom_register_positions_callback, custom_process_key);
}

static bool custom_register_positions_callback(custom_t* custom, void* user_data) {
    process_key_t function = (process_key_t)user_data;
    // Registra a função custom_process_key para todas as posições custom
    behavior_register_position_function(custom->row, custom->col, function);
    return true;
}

// ===== Inicialização =====

// Deve ser chamado durante custom_keyboard_post_init_user
void custom_init(void) {
    // Registra callbacks para mudanças
    event_bus_subscribe(EVENT_SETTINGS_LOADED, custom_settings_loaded_handler);
    event_bus_subscribe_profile_changed(custom_on_profile_changed);
}

// ===== Funções de Hook QMK =====
// Nota: Callbacks de FN são gerenciados via Event Bus em keymap.c

// Hook keyboard_post_init_user
static void custom_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;

    // Inicializa custom (registra callback de mudança de profile)
    custom_init();

    // Registra todas as posições custom na matriz de behavior
    custom_register_positions();

    // IMPORTANTE: Não sincroniza cache aqui!
    // A sincronização deve acontecer APÓS todos os handlers serem registrados
    // Isso será feito em custom_on_profile_changed_callback() quando chamado
    // durante a mudança para o profile padrão na inicialização
}

// ===== Inicialização de Hooks =====

// Registra todos os hooks (chamado em keyboard_post_init_user)
// Usado em keymap.c, mas o compilador pode não ver a referência
__attribute__((used))
void custom_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, custom_keyboard_post_init_user);
    event_bus_subscribe(EVENT_SETTINGS_LOADED, custom_settings_loaded_handler);
    // LEDs são gerenciados pelos submódulos (hold, toggle, unassociated)
}


