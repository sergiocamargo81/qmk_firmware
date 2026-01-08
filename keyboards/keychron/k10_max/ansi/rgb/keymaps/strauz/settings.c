#include "settings.h"

#include "custom_behaviors.h"  // Para CUSTOM_BEHAVIOR_UNASSOCIATED e CUSTOM_BEHAVIORS_T_COUNT
#include "profile.h"
#include "persist.h"
#include "event_bus.h"

#include <string.h>

// ===== Callbacks para Mudança de Profile =====
// Migrado para Event Bus - use event_bus_subscribe_profile_changed() em vez de settings_register_profile_changed_callback()

// ===== Funções Auxiliares =====

// Inicializa profiles_t com valores padrão
static void profiles_init_defaults(profiles_t* profiles) {
    if (!profiles) return;
    
    profiles->active_index = PROFILES_DEFAULT_INDEX;
    
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        profiles->profiles[i].active = (i == PROFILES_DEFAULT_INDEX);
        profiles->profiles[i].index = i;
        memset(profiles->profiles[i].behaviors, CUSTOM_BEHAVIOR_UNASSOCIATED, sizeof(profiles->profiles[i].behaviors));
    }
}

// Espelho exato do persist (nunca alterado durante execução, exceto após gravação)
settings_t persisted = {
    .version = 0,
    .profiles = {0},
};

// Cópia de trabalho em RAM (pode ser modificada)
settings_t working = {
    .version = SETTINGS_VERSION,
    .profiles = {0},
};

settings_t* settings_get_working(void) {
    return &working;
}

settings_t* settings_get_persisted(void) {
    return &persisted;
}

bool settings_init(void) {
    // Carrega da EEPROM
    bool has_valid_data = persist_read_settings(settings_get_persisted());

    if (!has_valid_data) {
        persisted.version = 0;
        profiles_init_defaults(&persisted.profiles);
    }

    memcpy(&working, &persisted, sizeof(settings_t));

    // Se não veio válido, inicializa working (sem gravar automaticamente)
    if (working.version != SETTINGS_VERSION || working.profiles.active_index >= PROFILES_COUNT) {
        working.version = SETTINGS_VERSION;
        profiles_init_defaults(&working.profiles);
    }

    // Anuncia que settings foram carregados com sucesso
    event_bus_publish_void(EVENT_SETTINGS_LOADED);

    return true;
}

void settings_reset_to_defaults(void) {
    // Defaults: tudo disabled em todos os perfis e perfil default selecionado.
    persisted.version = SETTINGS_VERSION;
    profiles_init_defaults(&persisted.profiles);

    memcpy(&working, &persisted, sizeof(settings_t));
}

// ===== Sistema de Callbacks =====
// Migrado para Event Bus

void settings_set_active_profile(uint8_t index) {
    if (index >= PROFILES_COUNT) {
        return;
    }
    
    settings_t* working = settings_get_working();
    if (!working) {
        return;
    }
    
    // Obtém o profile anterior (pode ser NULL se não havia profile ativo válido)
    profile_t* old_profile = NULL;
    if (working->profiles.active_index < PROFILES_COUNT) {
        old_profile = &working->profiles.profiles[working->profiles.active_index];
    }
    
    // Atualiza o profile ativo usando a função de profiles
    profile_set_active_profile(&working->profiles, index);
    
    // Obtém o novo profile ativo (sempre válido após profile_set_active_profile)
    profile_t* new_profile = profile_get_active_profile(&working->profiles);
    
    // Dispara Event Bus
    event_data_t data = {
        .profile_changed = {
            .old_profile = old_profile,
            .new_profile = new_profile
        }
    };
    event_bus_publish(EVENT_PROFILE_CHANGED, &data);
}

