#include "persistence.h"

#include <string.h>  // Para memcmp

#include "../settings.h"
#include "../persist.h"
#include "../kind.h"
#include "../behavior.h"  // Para behavior_register_position_function
#include "../keymod.h"  // Para keymod_t
#include "../hooks.h"
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"

// ===== Variáveis Globais =====

persistence_t persistence = {
    .state = PERSISTENCE_IDLE,
    .led_index = NO_LED,
    .save_timer = 0,
    .save_success = false,
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
    persistence_key_t* pos = kind_get_persistence(2, 15);
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
    persistence_key_t* pos = kind_get_persistence(2, 15);
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
        color_rgb_t yellow = color_get_rgb(COLOR_YELLOW);
        rgb_matrix_set_color(persistence.led_index, yellow.r, yellow.g, yellow.b);  // Amarelo sólido
    }

    settings_t* working = settings_get_working();
    working->version = SETTINGS_VERSION;
    if (working->profiles.active_index >= PROFILES_COUNT) {
        working->profiles.active_index = PROFILES_DEFAULT_INDEX;
    }

    bool success = persist_write_settings(working);
    if (!success) {
        // Erro ao salvar: armazena resultado para determinar próximo estado
        persistence.save_success = false;
        return false;
    }

    if (persistence_load()) {
        // Salvo com sucesso: armazena resultado para determinar próximo estado
        persistence.save_success = true;
    } else {
        // Erro ao carregar: armazena resultado para determinar próximo estado
        persistence.save_success = false;
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
            // Após 3 segundos, determina próximo estado baseado no resultado do salvamento
            if (persistence.save_success) {
                persistence.state = PERSISTENCE_BLINKING_SAVED;
            } else {
                persistence.state = PERSISTENCE_BLINKING_ERROR;
            }
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
    } else if (persistence.state == PERSISTENCE_PRESSED) {
        // Estado PRESSED: preserva enquanto a tecla está pressionada
        // O estado será restaurado em persistence_process_record_user quando a tecla for soltada
        return; // Não altera o estado PRESSED
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


// Atualiza indicadores RGB da persistência
void persistence_update_indicators(void) {
    if (persistence.led_index == NO_LED) return;

    switch (persistence.state) {
        case PERSISTENCE_IDLE:
            // Sem diferença: branco pulsante
            {
                uint8_t brightness = calculate_pulse_brightness();
                color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
                rgb_matrix_set_color(persistence.led_index, white.r, white.g, white.b);
            }
            break;
        case PERSISTENCE_DIFF:
            // Há diferença: verde pulsante
            {
                uint8_t brightness = calculate_pulse_brightness();
                color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
                rgb_matrix_set_color(persistence.led_index, green.r, green.g, green.b);
            }
            break;
        case PERSISTENCE_PRESSED:
            // END está pressionado (sem FN): verde contínuo
            {
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(persistence.led_index, green.r, green.g, green.b);
            }
            break;
        case PERSISTENCE_SAVING:
            // Salvando: amarelo sólido (pelo menos 3 segundos)
            {
                color_rgb_t yellow = color_get_rgb(COLOR_YELLOW);
                rgb_matrix_set_color(persistence.led_index, yellow.r, yellow.g, yellow.b);
            }
            break;
        case PERSISTENCE_BLINKING_SAVED:
            // Verde sólido após salvar com sucesso (3 segundos)
            {
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(persistence.led_index, green.r, green.g, green.b);
            }
            break;
        case PERSISTENCE_BLINKING_ERROR:
            // Vermelho sólido após erro (3 segundos)
            {
                color_rgb_t red = color_get_rgb(COLOR_RED);
                rgb_matrix_set_color(persistence.led_index, red.r, red.g, red.b);
            }
            break;
    }
}

// ===== Funções de Hook QMK =====

// Processa KC_END (FN+END para salvar, ou apenas END para feedback visual) - baseado em posição, não keycode
bool persistence_process_record_user(keyrecord_t *record, keymod_t keymod) {
    bool pressed = record->event.pressed;
    
    // Verifica se FN está ativo usando keymod
    // keymod é um enum de flags, verifica se contém qualquer modo com FN (FN_ONLY, FN_RCTL, FN_RALT, FN_RSFT)
    bool fn_active = keymod_has_value(keymod, KEYMOD_FN_ONLY | KEYMOD_FN_RCTL | KEYMOD_FN_RALT | KEYMOD_FN_RSFT);
    
    if (pressed && fn_active) {
        // FN+END: salva settings
        persistence_save();
        return false; // Consome o evento
    } else {
        // END sem FN: apenas atualiza estado para feedback visual
        if (pressed) {
            persistence.state = PERSISTENCE_PRESSED;
        } else {
            // Ao soltar, restaura estado baseado em diferença
            if (persistence_has_diff()) {
                persistence.state = PERSISTENCE_DIFF;
            } else {
                persistence.state = PERSISTENCE_IDLE;
            }
        }
        return true; // Não consome o evento, permite processamento normal
    }
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
    } else {
        persistence.state = PERSISTENCE_IDLE;
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
