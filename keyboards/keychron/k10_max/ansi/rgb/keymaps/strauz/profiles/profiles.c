#include "profiles.h"

#include "../behavior.h"
#include "../kind.h"
#include "../settings.h"
#include "../customs.h"  // Para KEY_CUSTOM_DISABLED
#include "../keymod.h"  // Para keymod_t
#include "../hooks.h"
#include "../pulse.h"

// Declaração forward
bool profiles_process_record_user(keyrecord_t *record, keymod_t keymod);

// ===== Array Fixo de Posições =====
// Array de posições fixas para P0-P9 (profile_id 0-9)
// Consulta kind.c pela API para obter led_index e outros valores
typedef struct {
    uint8_t row;
    uint8_t col;
} profile_position_t;

static const profile_position_t PROFILE_POSITIONS[PROFILES_COUNT] = {
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

// ===== Variáveis Globais =====
profiles_state_t profiles_state = {
    .led_states = {PROFILES_LED_EMPTY, PROFILES_LED_EMPTY, PROFILES_LED_EMPTY, PROFILES_LED_EMPTY, PROFILES_LED_EMPTY,
                   PROFILES_LED_EMPTY, PROFILES_LED_EMPTY, PROFILES_LED_EMPTY, PROFILES_LED_EMPTY, PROFILES_LED_EMPTY},
    .led_initialized = false
};

// ===== Funções de Registro =====

void profiles_register_positions(void) {
    // Registra função para cada posição fixa do array
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        const profile_position_t* pos = &PROFILE_POSITIONS[i];
        // Verifica se a posição existe no grid antes de registrar
        position_t* position = kind_get_position(pos->row, pos->col);
        if (position != NULL) {
            behavior_register_position_function(pos->row, pos->col, profiles_process_record_user);
        }
    }
}

// ===== Funções de Estado =====
void profiles_init_led_indices(void) {
    // Não precisa mais inicializar led_index, será obtido diretamente de kind.c
    profiles_state.led_initialized = true;
}

void profiles_sync_from_settings(void) {
    settings_t* working = settings_get_working();
    if (!working) return;
    profiles_update_led_states();
}

void profiles_update_led_states(void) {
    settings_t* working = settings_get_working();
    if (!working) return;
    
    profiles_t* profiles = &working->profiles;
    // Itera sobre cada posição fixa do array (coordenadas locais)
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        const profile_position_t* pos = &PROFILE_POSITIONS[i];
        
        // Obtém dados de kind.c pela API (verifica se posição existe)
        position_t* position = kind_get_position(pos->row, pos->col);
        if (position == NULL) {
            continue; // Posição não encontrada, tenta próxima
        }
        
        // Atualiza estado do LED baseado no profile correspondente
        // Apenas profile ativo é indicado, outros ficam desligados
        profile_t* profile = &profiles->profiles[i];
        if (profile->active) {
            profiles_state.led_states[i] = PROFILES_LED_ACTIVE;
        } else {
            profiles_state.led_states[i] = PROFILES_LED_EMPTY;  // Usado para "desligado"
        }
    }
}

void profiles_update_indicators(void) {
    // Itera sobre cada posição fixa do array e consulta kind.c pela API
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        const profile_position_t* pos = &PROFILE_POSITIONS[i];
        
        // Consulta kind.c pela posição fixa usando API
        position_t* position = kind_get_position(pos->row, pos->col);
        if (position == NULL || position->led_index == NO_LED) continue;
        
        // Obtém o state armazenado (já inicializado em profiles_state.led_states)
        profiles_led_state_t state = profiles_state.led_states[i];
        
        // Apenas profile ativo é indicado (verde pulsante)
        // Quando não ativo, não faz nada - permite que o efeito padrão do teclado seja aplicado
        if (state == PROFILES_LED_ACTIVE) {
            uint8_t brightness = calculate_pulse_brightness();
            rgb_matrix_set_color(position->led_index, 0, brightness, 0);
        }
        // Quando não ativo (PROFILES_LED_EMPTY), não faz nada
        // O efeito padrão do teclado será aplicado automaticamente
        // Se o RGB estiver desligado, a tecla ficará desligada naturalmente
    }
}

// ===== Funções de Hook QMK =====

// Processa KC_P0-KC_P9 (FN+P0-P9 para trocar profile) - baseado em posição fixa
bool profiles_process_record_user(keyrecord_t *record, keymod_t keymod) {
    if (!record->event.pressed) return true;

    // Verifica se FN está ativo usando keymod
    // keymod é um enum de flags, verifica se há interseção com KEYMOD_FN
    if ((keymod & KEYMOD_FN) == 0) {
        return true;
    }

    // Obtém posição (row, col)
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // Verifica se a posição corresponde a alguma posição fixa do array
    for (uint8_t i = 0; i < PROFILES_COUNT; i++) {
        const profile_position_t* pos = &PROFILE_POSITIONS[i];
        if (pos->row == row && pos->col == col) {
            // Verifica se a posição existe no grid usando kind_get_position
            position_t* position = kind_get_position(pos->row, pos->col);
            if (position == NULL) {
                continue; // Posição não encontrada, tenta próxima
            }
            
            // Encontrou o profile correspondente à posição
            // Obtém settings
            settings_t* working = settings_get_working();
            if (!working) return true;
            
            // Ativa o novo profile e desativa o atual (sempre faz a troca)
            // Usa settings_set_active_profile que notifica automaticamente os callbacks
            // O callback profiles_on_profile_changed será chamado automaticamente
            settings_set_active_profile(i);
            
            return false; // Consome o evento
        }
    }

    return true; // Passa adiante
}

// Hook rgb_matrix_indicators_user
static bool profiles_rgb_matrix_indicators_user(void) {
    // Sempre atualiza os estados antes de atualizar os indicadores
    // Isso garante que os estados estejam sempre sincronizados
    profiles_update_led_states();
    // Atualiza os LEDs
    profiles_update_indicators();
    return true;
}

// Callback para mudança de perfil
static void profiles_on_profile_changed(profile_t* old_profile, profile_t* new_profile) {
    // Atualiza estados dos LEDs sempre que o perfil muda
    profiles_update_led_states();
    // Força atualização imediata dos indicadores
    profiles_update_indicators();
}

// Hook keyboard_post_init_user
static void profiles_keyboard_post_init_user(void) {
    // Registra todas as posições de KC_P0-KC_P9 na matriz de behavior
    profiles_register_positions();
    
    profiles_init_led_indices();
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

