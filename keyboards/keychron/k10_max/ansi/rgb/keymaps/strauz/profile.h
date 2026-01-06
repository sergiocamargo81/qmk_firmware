#ifndef PROFILE_H
#define PROFILE_H

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
profile_t* profile_get_active_profile(profiles_t* profiles);

// Define o profile ativo
void profile_set_active_profile(profiles_t* profiles, uint8_t index);

// Verifica se o profile ativo está vazio
// Retorna true se o profile ativo está vazio ou não existe, false caso contrário
bool profile_is_active_profile_empty(void);

#endif // PROFILE_H
