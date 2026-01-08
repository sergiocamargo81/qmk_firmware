#include "profile.h"
#include "settings.h"

#include <string.h>

// Retorna o profile ativo
profile_t* profile_get_active_profile(profiles_t* profiles) {
    if (!profiles) return NULL;
    if (profiles->active_index >= PROFILES_COUNT) return NULL;
    return &profiles->profiles[profiles->active_index];
}

// Define o profile ativo
void profile_set_active_profile(profiles_t* profiles, uint8_t index) {
    if (!profiles) return;
    if (index >= PROFILES_COUNT) return;
    
    // Desativa o profile ativo
    if (profiles->active_index < PROFILES_COUNT) {
        profiles->profiles[profiles->active_index].active = false;
    }
    
    // Ativa o novo profile
    profiles->active_index = index;
    profiles->profiles[index].active = true;
}

// Verifica se o profile ativo está vazio
bool profile_is_active_profile_empty(void) {
    extern settings_t* settings_get_working(void);
    settings_t* working = settings_get_working();
    if (!working) return true;
    
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile) return true;
    
    // Verifica se todos os behaviors são CUSTOM_BEHAVIOR_UNASSOCIATED
    for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
        if (active_profile->behaviors[i] != CUSTOM_BEHAVIOR_UNASSOCIATED) {
            return false;
        }
    }
    return true;
}
