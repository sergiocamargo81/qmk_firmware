#include "profiles.h"

#include <string.h>

// Retorna o profile ativo
profile_t* profiles_get_active_profile(profiles_t* profiles) {
    if (!profiles) return NULL;
    if (profiles->active_index >= PROFILES_COUNT) return NULL;
    return &profiles->profiles[profiles->active_index];
}

// Define o profile ativo
void profiles_set_active_profile(profiles_t* profiles, uint8_t index) {
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

