#ifndef PROFILES_H
#define PROFILES_H

#include QMK_KEYBOARD_H
#include "customs.h"  // Para customs_t

// ===== Defines =====

#define PROFILE_KEYS_COUNT 48
#define PROFILES_KEYS_COUNT PROFILE_KEYS_COUNT
#define PROFILES_COUNT 10
#define PROFILES_DEFAULT_INDEX 0

// ===== Types/structs =====

// Profile individual
typedef struct {
    bool active;                                  // Se é o profile ativo
    uint8_t index;                                // Índice do profile (0-9)
    customs_t behaviors[PROFILE_KEYS_COUNT];  // Comportamentos das teclas
} profile_t;

// Conjunto de profiles
typedef struct {
    uint8_t active_index;                     // Índice do profile ativo (0-9)
    profile_t profiles[PROFILES_COUNT];       // Array de profiles
} profiles_t;

// ===== Functions =====

// Retorna o profile ativo
profile_t* profiles_get_active_profile(profiles_t* profiles);

// Define o profile ativo
void profiles_set_active_profile(profiles_t* profiles, uint8_t index);

#endif // PROFILES_H

