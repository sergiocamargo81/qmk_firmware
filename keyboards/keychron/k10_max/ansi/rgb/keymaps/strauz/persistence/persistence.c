#include "persistence.h"

#include <string.h>  // Para memcmp

#include "../settings.h"
#include "../persist.h"
#include "../kind.h"
#include "../behavior.h"  // Para behavior_register_position_function
#include "../keymod.h"  // Para keymod_t
#include "../hooks.h"
#include "rgb_matrix.h"  // Para g_rgb_timer
#include "lib8tion.h"     // Para sin8, scale8

// ===== Variáveis Globais =====

persistence_t persistence = {
    .state = PERSISTENCE_IDLE,
    .next_state = PERSISTENCE_IDLE,
    .led_index = NO_LED,
    .save_timer = 0,
    .led_initialized = false,
    .kc_end_row = 0,
    .kc_end_col = 0
};

// Declaração forward
bool persistence_process_record_user(keyrecord_t *record, keymod_t keymod);

// ===== Funções de Registro =====

// Registra a posição de KC_END na matriz de behavior
// Usa posição fixa: row=2, col=15
void persistence_register_position(void) {
    position_t* pos = kind_get_position(2, 15);
    if (pos) {
        // Cache da posição para evitar chamadas repetidas
        persistence.kc_end_row = pos->row;
        persistence.kc_end_col = pos->col;
        behavior_register_position_function(pos->row, pos->col, persistence_process_record_user);
    }
}

// ===== Inicialização =====

// Inicializa LED index para persistência
// Usa posição fixa: row=2, col=15
void persistence_init_led_index(void) {
    position_t* pos = kind_get_position(2, 15);
    if (pos) {
        persistence.led_index = pos->led_index;
    } else {
        persistence.led_index = NO_LED;
    }
    persistence.led_initialized = true;
}

// ===== Persistência =====

// Carrega settings da EEPROM
bool persistence_load(void) {
    settings_t* persisted = settings_get_persisted();
    return persist_read_settings(persisted);
}

// Compara persisted e working settings
// Retorna true se há diferença, false se são iguais
static bool persistence_has_diff(void) {
    settings_t* persisted = settings_get_persisted();
    settings_t* working = settings_get_working();
    
    if (!persisted || !working) return false;
    
    // Compara usando memcmp (comparação byte a byte)
    return (memcmp(persisted, working, sizeof(settings_t)) != 0);
}

// Salva settings na EEPROM
bool persistence_save(void) {
    // Define estado SAVING imediatamente
    // Isso garante que o LED fique amarelo imediatamente ao pressionar FN+END
    persistence.state = PERSISTENCE_SAVING;
    persistence.save_timer = timer_read32();
    
    // Força atualização imediata do LED para amarelo
    if (persistence.led_index != NO_LED) {
        rgb_matrix_set_color(persistence.led_index, 255, 255, 0);  // Amarelo sólido
    }

    settings_t* working = settings_get_working();
    working->version = SETTINGS_VERSION;
    if (working->profiles.active_index >= PROFILES_COUNT) {
        working->profiles.active_index = PROFILES_DEFAULT_INDEX;
    }

    bool success = persist_write_settings(working);
    if (!success) {
        // Erro ao salvar: define próximo estado como vermelho sólido por 3 segundos
        // O estado SAVING será mantido por pelo menos 3 segundos em persistence_update_state()
        persistence.next_state = PERSISTENCE_BLINKING_ERROR;
        return false;
    }

    if (persistence_load()) {
        // Salvo com sucesso: define próximo estado como verde sólido por 3 segundos
        // O estado SAVING será mantido por pelo menos 3 segundos em persistence_update_state()
        persistence.next_state = PERSISTENCE_BLINKING_SAVED;
    } else {
        // Erro ao carregar: define próximo estado como vermelho sólido por 3 segundos
        // O estado SAVING será mantido por pelo menos 3 segundos em persistence_update_state()
        persistence.next_state = PERSISTENCE_BLINKING_ERROR;
        return false;
    }

    return true;
}

// Atualiza estado da persistência (chamado em matrix_scan)
void persistence_update_state(void) {
    // Estado SAVING: mantém amarelo por pelo menos 3 segundos antes de mudar para o próximo estado
    if (persistence.state == PERSISTENCE_SAVING) {
        // Mantém estado SAVING por pelo menos 3 segundos
        if (timer_elapsed32(persistence.save_timer) >= 3000) {
            // Após 3 segundos, muda para o próximo estado e reinicia timer
            persistence.state = persistence.next_state;
            persistence.save_timer = timer_read32();
        }
        return; // Mantém estado SAVING até passar 3 segundos
    }
    
    // Estados de feedback após salvar (3 segundos)
    if (persistence.state == PERSISTENCE_BLINKING_SAVED || 
        persistence.state == PERSISTENCE_BLINKING_ERROR) {
        // Após 3 segundos, verifica diferença novamente e define estado apropriado
        if (timer_elapsed32(persistence.save_timer) >= 3000) {
            // Compara persisted e working e define estado atual
            if (persistence_has_diff()) {
                persistence.state = PERSISTENCE_DIFF;
            } else {
                persistence.state = PERSISTENCE_IDLE;
            }
        }
    } else {
        // Estados normais: verifica diferença entre persisted e working
        if (persistence.state == PERSISTENCE_IDLE || persistence.state == PERSISTENCE_DIFF) {
            if (persistence_has_diff()) {
                persistence.state = PERSISTENCE_DIFF;
            } else {
            persistence.state = PERSISTENCE_IDLE;
            }
        }
    }
}

// Função auxiliar para calcular brilho pulsante
// Retorna valor de 0-255 baseado em seno do tempo
static uint8_t calculate_pulse_brightness(void) {
    // Usa g_rgb_timer para criar efeito de pulsação
    // Velocidade: divide por 8 para pulsação lenta (~2 segundos por ciclo)
    uint8_t time = scale16by8(g_rgb_timer, 1);
    // sin8 retorna 0-255, onde 128 é o meio
    // Ajusta para que o mínimo seja ~30% e máximo seja 100%
    uint8_t sine = sin8(time);
    // Escala de 77 (30% de 255) a 255 (100%)
    return scale8(sine, 178) + 77;
}

// Atualiza indicadores RGB da persistência
void persistence_update_indicators(void) {
    if (persistence.led_index == NO_LED) return;

    switch (persistence.state) {
        case PERSISTENCE_IDLE:
            // Sem diferença: LED desligado
            rgb_matrix_set_color(persistence.led_index, 0, 0, 0);
            break;
        case PERSISTENCE_DIFF:
            // Há diferença: verde pulsante
            {
                uint8_t brightness = calculate_pulse_brightness();
                rgb_matrix_set_color(persistence.led_index, 0, brightness, 0);
            }
            break;
        case PERSISTENCE_SAVING:
            // Salvando: amarelo sólido (pelo menos 3 segundos)
            rgb_matrix_set_color(persistence.led_index, 255, 255, 0);
            break;
        case PERSISTENCE_BLINKING_SAVED:
            // Verde sólido após salvar com sucesso (3 segundos)
            rgb_matrix_set_color(persistence.led_index, 0, 255, 0);
            break;
        case PERSISTENCE_BLINKING_ERROR:
            // Vermelho sólido após erro (3 segundos)
            rgb_matrix_set_color(persistence.led_index, 255, 0, 0);
            break;
    }
}

// ===== Funções de Hook QMK =====

// Processa KC_END (FN+END para salvar) - baseado em posição, não keycode
bool persistence_process_record_user(keyrecord_t *record, keymod_t keymod) {
    if (!record->event.pressed) return true;

    // Verifica se FN está ativo usando keymod
    // keymod é um enum de flags, verifica se há interseção com KEYMOD_FN, KEYMOD_FN_RCTL ou KEYMOD_FN_RALT
    keymod_t accepted_mask = KEYMOD_FN | KEYMOD_FN_RCTL | KEYMOD_FN_RALT;
    if ((keymod & accepted_mask) == 0) {
        return true;
    }

    // Handler já está registrado para esta posição específica, não precisa verificar novamente
    persistence_save();
    return false; // Consome o evento
}

// Hook matrix_scan_user
static void persistence_matrix_scan_user(void) {
    persistence_update_state();
}

// Hook rgb_matrix_indicators_user
static bool persistence_rgb_matrix_indicators_user(void) {
    persistence_update_indicators();
    return true;
}

// Hook keyboard_post_init_user
static void persistence_keyboard_post_init_user(void) {
    // Registra a posição de KC_END
    persistence_register_position();
    
    // Inicializa LED index
    persistence_init_led_index();
    
    // Inicializa estado baseado em diferença entre persisted e working
    if (persistence_has_diff()) {
        persistence.state = PERSISTENCE_DIFF;
        persistence.next_state = PERSISTENCE_DIFF;
    } else {
        persistence.state = PERSISTENCE_IDLE;
        persistence.next_state = PERSISTENCE_IDLE;
    }
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void persistence_init_early_hooks(void) {
    hooks_matrix_scan_register(persistence_matrix_scan_user);
    hooks_rgb_indicators_register(persistence_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de hooks_keyboard_post_init_dispatch)
void persistence_init_hooks(void) {
    hooks_keyboard_post_init_register(persistence_keyboard_post_init_user);
}
