#include "profiles.h"

#include "../behavior.h"
#include "../kind.h"
#include "../settings.h"
#include "../custom_behaviors.h"  // Para CUSTOM_BEHAVIOR_UNASSOCIATED
#include "../keymod.h"  // Para keymod_t
#include "../event_bus.h"  // Para Event Bus
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"

// Declaração forward
bool profiles_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// ===== Array de Posições da Matrix =====
// Array de ponteiros para profile_key_t* da matrix (P0-P9, profile_id 0-9)
// O índice do array corresponde ao profile_id (0-9)
static profile_key_t* profile_positions[PROFILES_COUNT] = {NULL};

// ===== Funções de Registro =====

// Callback para registrar posições via iteração
static bool profiles_register_positions_callback(profile_key_t* profile, void* user_data) {
    (void)user_data;
    if (profile == NULL) return true;
    
    // Obtém profile_id do campo profile_index
    uint8_t profile_id = profile->profile_index;
    if (profile_id >= PROFILES_COUNT) return true;
    
    // Armazena ponteiro no array indexado por profile_id
    profile_positions[profile_id] = profile;
    
    // Registra função para esta posição
        behavior_register_position_function(profile->row, profile->col, profiles_process_key);
    
    return true; // Continua iteração
}

void profiles_register_positions(void) {
    // Itera sobre todas as teclas profile e registra posições
    // kind_iterate_profiles garante ordem 0-9 (P0-P9)
    kind_iterate_profiles(profiles_register_positions_callback, NULL);
}

// ===== Funções Auxiliares =====

// Verifica se um profile está vazio
static bool profile_is_empty(profile_t* profile) {
    if (!profile) return true;
    for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
        if (profile->behaviors[i] != CUSTOM_BEHAVIOR_UNASSOCIATED) {
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
bool profiles_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (key == NULL || key->kind != KIND_PROFILE) {
        return true;
    }
    
    // Encontra o profile_key_t* correspondente
    profile_key_t* position = (profile_key_t*)key;
    
    // Obtém profile_id do campo profile_index
    uint8_t profile_index = position->profile_index;
    
    // Verifica se a posição está registrada
    if (profile_index >= PROFILES_COUNT || profile_positions[profile_index] != position) {
        return true; // Não é uma tecla de profile conhecida
    }
    
    // Atualiza estado de pressionado/solto
    if (pressed) {
        // Salva o estado atual antes de marcar como PRESSED
        // O estado será restaurado quando a tecla for solta
        position->state = PROFILE_PRESSED;
        
        // Verifica se FN está ativo usando keymod
        // keymod é um enum de flags, verifica se contém KEYMOD_FN_ONLY
        if (keymod_equals(keymod, KEYMOD_FN_ONLY)) {
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
static void profiles_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    // Atualiza os LEDs primeiro (preserva PROFILE_PRESSED se estiver definido)
    profiles_update_indicators();
    // Depois atualiza os estados (mas não sobrescreve PROFILE_PRESSED)
    profiles_update_states();
}

// Callback para mudança de perfil
static void profiles_on_profile_changed(const event_t* event) {
    if (event->type != EVENT_PROFILE_CHANGED) return;
    // Atualiza estados na matrix sempre que o perfil muda
    profiles_update_states();
    // Força atualização imediata dos indicadores
    profiles_update_indicators();
    (void)event; // Variáveis do evento não são usadas diretamente
}

// Hook keyboard_post_init_user
static void profiles_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;
    // Registra todas as posições de KC_P0-KC_P9 na matriz de behavior
    profiles_register_positions();
    
    profiles_sync_from_settings();
    
    // Força atualização inicial dos LEDs
    profiles_update_indicators();
    
    // Registra callback para atualizar LEDs quando o perfil mudar
    event_bus_subscribe_profile_changed(profiles_on_profile_changed);
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void profiles_init_early_hooks(void) {
    event_bus_subscribe_rgb_indicators(profiles_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void profiles_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, profiles_keyboard_post_init_user);
}

