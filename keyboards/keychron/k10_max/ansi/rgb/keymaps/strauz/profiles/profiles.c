#include "profiles.h"

#include "../behavior.h"
#include "../kind.h"
#include "../settings.h"
#include "../customs.h"  // Para KEY_CUSTOM_UNASSOCIATED
#include "../keymod.h"  // Para keymod_t
#include "../hooks.h"
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"

// Declaração forward
bool profiles_process_record_user(keyrecord_t *record, keymod_t keymod);

// ===== Array de Posições da Matrix =====
// Array de ponteiros para profile_key_t* da matrix (P0-P9, profile_id 0-9)
static profile_key_t* profile_positions[PROFILES_COUNT] = {NULL};

// ===== Funções de Registro =====

void profiles_register_positions(void) {
    // Coordenadas fixas para P0-P9 (profile_id 0-9)
    static const struct {
        uint8_t row;
        uint8_t col;
    } coords[PROFILES_COUNT] = {
        {.row = 5, .col = 18},
        {.row = 4, .col = 17},
        {.row = 4, .col = 18},
        {.row = 4, .col = 19},
        {.row = 3, .col = 17},
        {.row = 3, .col = 18},
        {.row = 3, .col = 19},
        {.row = 2, .col = 17},
        {.row = 2, .col = 18},
        {.row = 2, .col = 19},
    };
    
    // Inicializa array de ponteiros e registra função para cada posição
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        uint8_t row = coords[i].row;
        uint8_t col = coords[i].col;
        // Obtém profile_key_t da matrix
        profile_key_t* position = kind_get_profile(row, col);
        if (position != NULL) {
            profile_positions[i] = position;
            behavior_register_position_function(row, col, profiles_process_record_user);
        }
    }
}

// ===== Funções Auxiliares =====

// Verifica se um profile está vazio
static bool profile_is_empty(profile_t* profile) {
    if (!profile) return true;
    for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
        if (profile->behaviors[i] != KEY_CUSTOM_UNASSOCIATED) {
            return false;
        }
    }
    return true;
}

// ===== Funções de Estado =====
void profiles_sync_from_settings(void) {
    settings_t* working = settings_get_working();
    if (!working) return;
    profiles_update_states();
}

void profiles_update_states(void) {
    settings_t* working = settings_get_working();
    if (!working) return;
    
    profiles_t* profiles = &working->profiles;
    // Itera sobre cada profile_key_t* do array
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        profile_key_t* position = profile_positions[i];
        if (position == NULL) {
            continue; // Posição não encontrada, tenta próxima
        }
        
        // Se a tecla está pressionada (PROFILE_PRESSED), não atualiza o estado
        // O estado será restaurado quando a tecla for solta
        if (position->state == PROFILE_PRESSED) {
            continue;
        }
        
        // Atualiza estado na matrix baseado no profile correspondente
        profile_t* profile = &profiles->profiles[i];
        if (profile->active) {
            position->state = PROFILE_ACTIVE;
        } else {
            // Verifica se o profile está vazio ou não vazio
            if (profile_is_empty(profile)) {
                position->state = PROFILE_EMPTY;
            } else {
                position->state = PROFILE_NONEMPTY;
            }
        }
    }
}

void profiles_update_indicators(void) {
    // Itera sobre cada profile_key_t* do array
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        profile_key_t* position = profile_positions[i];
        if (position == NULL) continue;
        
        // Obtém o state armazenado na matrix (position->state)
        profile_state_t state = position->state;
        
        // Se a tecla está pressionada, mostra verde sólido
        if (state == PROFILE_PRESSED) {
            color_rgb_t green = color_get_rgb(COLOR_GREEN);
            rgb_matrix_set_color(position->led_index, green.r, green.g, green.b);
            continue;
        }
        
        // Se não está pressionada, mostra a cor de estado
        // Calcula brilho pulsante (0-255)
        uint8_t brightness = calculate_pulse_brightness();
        
        // Renderiza LED baseado no estado
        switch (state) {
            case PROFILE_ACTIVE: {
                // Profile ativo: verde pulsante
                color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
                rgb_matrix_set_color(position->led_index, green.r, green.g, green.b);
                break;
            }
            case PROFILE_EMPTY: {
                // Profile vazio: branco pulsante
                color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
                rgb_matrix_set_color(position->led_index, white.r, white.g, white.b);
                break;
            }
            case PROFILE_NONEMPTY: {
                // Profile não vazio (mas não ativo): laranja pulsante
                color_rgb_t orange = color_apply_brightness(COLOR_ORANGE_BURNT, brightness);
                rgb_matrix_set_color(position->led_index, orange.r, orange.g, orange.b);
                break;
            }
            default:
                break;
        }
    }
}

// ===== Funções de Hook QMK =====

// Processa KC_P0-KC_P9 (FN+P0-P9 para trocar profile) - baseado em profile_key_t*
bool profiles_process_record_user(keyrecord_t *record, keymod_t keymod) {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    bool pressed = record->event.pressed;
    
    // Encontra o profile_key_t* correspondente
    profile_key_t* position = NULL;
    uint8_t profile_index = 0;
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        if (profile_positions[i] != NULL && 
            profile_positions[i]->row == row && 
            profile_positions[i]->col == col) {
            position = profile_positions[i];
            profile_index = i;
            break;
        }
    }
    
    if (position == NULL) {
        return true; // Não é uma tecla de profile
    }
    
    // Atualiza estado de pressionado/solto
    if (pressed) {
        // Salva o estado atual antes de marcar como PRESSED
        // O estado será restaurado quando a tecla for solta
        position->state = PROFILE_PRESSED;
        
        // Verifica se FN está ativo usando keymod
        // keymod é um enum de flags, verifica se contém KEYMOD_FN_ONLY
        if (keymod_is_only_value(keymod, KEYMOD_FN_ONLY)) {
            // Obtém settings
            settings_t* working = settings_get_working();
            if (working != NULL) {
                // Ativa o novo profile e desativa o atual (sempre faz a troca)
                // Usa settings_set_active_profile que notifica automaticamente os callbacks
                // O callback profiles_on_profile_changed será chamado automaticamente
                settings_set_active_profile(profile_index);
            }
            return false; // Consome o evento
        }
    } else {
        // Tecla foi solta: restaura o estado baseado no profile
        settings_t* working = settings_get_working();
        if (working != NULL) {
            profiles_t* profiles = &working->profiles;
            profile_t* profile = &profiles->profiles[profile_index];
            if (profile->active) {
                position->state = PROFILE_ACTIVE;
            } else {
                if (profile_is_empty(profile)) {
                    position->state = PROFILE_EMPTY;
                } else {
                    position->state = PROFILE_NONEMPTY;
                }
            }
        }
    }

    return true; // Passa adiante (não consome quando não é FN)
}

// Hook rgb_matrix_indicators_user
static bool profiles_rgb_matrix_indicators_user(void) {
    // Atualiza os LEDs primeiro (preserva PROFILE_PRESSED se estiver definido)
    profiles_update_indicators();
    // Depois atualiza os estados (mas não sobrescreve PROFILE_PRESSED)
    profiles_update_states();
    return true;
}

// Callback para mudança de perfil
static void profiles_on_profile_changed(profile_t* old_profile, profile_t* new_profile) {
    // Atualiza estados na matrix sempre que o perfil muda
    profiles_update_states();
    // Força atualização imediata dos indicadores
    profiles_update_indicators();
}

// Hook keyboard_post_init_user
static void profiles_keyboard_post_init_user(void) {
    // Registra todas as posições de KC_P0-KC_P9 na matriz de behavior
    profiles_register_positions();
    
    profiles_sync_from_settings();
    
    // Força atualização inicial dos LEDs
    profiles_update_indicators();
    
    // Registra callback para atualizar LEDs quando o perfil mudar
    settings_register_profile_changed_callback(profiles_on_profile_changed);
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void profiles_init_early_hooks(void) {
    hooks_rgb_indicators_register(profiles_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void profiles_init_hooks(void) {
    hooks_keyboard_post_init_register(profiles_keyboard_post_init_user);
}

