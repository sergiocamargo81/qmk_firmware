#include "persistence.h"

#include <string.h>  // Para memcmp

#include "../settings.h"
#include "../persist.h"
#include "../kind.h"
#include "../behavior.h"  // Para behavior_register_position_function
#include "../keymod.h"  // Para keymod_t
#include "../event_bus.h"  // Para Event Bus
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"

// ===== Variáveis Estáticas =====

// Ponteiro para a tecla persistence na matrix (inicializado durante init)
static persistence_key_t* persistence_key = NULL;

// Timer para controlar feedback visual (3 segundos)
static uint32_t save_timer = 0;

// Resultado do último salvamento (true = sucesso, false = erro)
static bool save_success = false;

// Declaração forward
bool persistence_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// ===== Funções Auxiliares =====

// Compara persisted e working settings
// Retorna true se há diferença, false se são iguais
static bool persistence_has_diff(void) {
    settings_t* persisted = settings_get_persisted();
    settings_t* working = settings_get_working();
    
    if (!persisted || !working) return false;
    
    // Compara usando memcmp (comparação byte a byte)
    return (memcmp(persisted, working, sizeof(settings_t)) != 0);
}

// Obtém o estado atual da persistência
static persistence_state_t persistence_get_state(void) {
    if (persistence_key == NULL) return PERSISTENCE_SAVED;
    return (persistence_state_t)persistence_key->state;
}

// Define o estado atual da persistência
static void persistence_set_state(persistence_state_t state) {
    if (persistence_key != NULL) {
        persistence_key->state = state;
    }
}

// ===== Persistência =====

// Carrega settings da EEPROM
bool persistence_load(void) {
    settings_t* persisted = settings_get_persisted();
    return persist_read_settings(persisted);
}

// Salva settings na EEPROM
bool persistence_save(void) {
    // Define estado SAVING imediatamente
    // Isso garante que o LED fique amarelo imediatamente ao pressionar FN+END
    persistence_set_state(PERSISTENCE_SAVING);
    save_timer = timer_read32();
    // Inicializa save_success como false (será atualizado após salvar)
    save_success = false;

    settings_t* working = settings_get_working();
    working->version = SETTINGS_VERSION;
    if (working->profiles.active_index >= PROFILES_COUNT) {
        working->profiles.active_index = PROFILES_DEFAULT_INDEX;
    }

    // Tenta salvar
    bool write_success = persist_write_settings(working);
    if (!write_success) {
        // Erro ao salvar: armazena resultado para determinar próximo estado
        save_success = false;
        // Estado permanece PERSISTENCE_SAVING, será atualizado após 3 segundos para PERSISTENCE_BLINKING_ERROR
        return false;
    }

    // Tenta carregar para verificar se salvou corretamente
    bool load_success = persistence_load();
    if (!load_success) {
        // Erro ao carregar: armazena resultado para determinar próximo estado
        save_success = false;
        // Estado permanece PERSISTENCE_SAVING, será atualizado após 3 segundos para PERSISTENCE_BLINKING_ERROR
        return false;
    }

    // Salvo com sucesso: armazena resultado para determinar próximo estado
    save_success = true;
    // Estado permanece PERSISTENCE_SAVING, será atualizado após 3 segundos para PERSISTENCE_BLINKING_SAVED
    return true;
}

// Atualiza estado da persistência (chamado em matrix_scan)
void persistence_update_state(void) {
    persistence_state_t state = persistence_get_state();
    
    // Estado PRESSED: preserva enquanto a tecla está pressionada
    // Deve ser verificado primeiro para evitar que outros estados sobrescrevam
    if (state == PERSISTENCE_PRESSED) {
        // O estado será restaurado em persistence_process_key quando a tecla for soltada
        return; // Não altera o estado PRESSED
    }
    
    // Estado SAVING: mantém amarelo por pelo menos 3 segundos antes de mudar para o próximo estado
    if (state == PERSISTENCE_SAVING) {
        // Mantém estado SAVING por pelo menos 3 segundos
        if (timer_elapsed32(save_timer) >= 3000) {
            // Após 3 segundos, determina próximo estado baseado no resultado do salvamento
            if (save_success) {
                persistence_set_state(PERSISTENCE_BLINKING_SAVED);
            } else {
                persistence_set_state(PERSISTENCE_BLINKING_ERROR);
            }
            save_timer = timer_read32();
        }
        return; // Mantém estado SAVING até passar 3 segundos
    }
    
    // Estados de feedback após salvar (3 segundos)
    if (state == PERSISTENCE_BLINKING_SAVED || state == PERSISTENCE_BLINKING_ERROR) {
        // Após 3 segundos, verifica diferença novamente e define estado apropriado
        if (timer_elapsed32(save_timer) >= 3000) {
            // Compara persisted e working e define estado atual
            if (persistence_has_diff()) {
                persistence_set_state(PERSISTENCE_DIFF);
            } else {
                persistence_set_state(PERSISTENCE_SAVED);
            }
        }
        return; // Mantém estado de feedback até passar 3 segundos
    }
    
    // Estados normais: verifica diferença entre persisted e working
    // Só atualiza se não estiver em nenhum dos estados especiais acima
    if (state == PERSISTENCE_SAVED || state == PERSISTENCE_DIFF) {
        if (persistence_has_diff()) {
            persistence_set_state(PERSISTENCE_DIFF);
        } else {
            persistence_set_state(PERSISTENCE_SAVED);
        }
    }
}

// Atualiza indicadores RGB da persistência
void persistence_update_indicators(void) {
    if (persistence_key == NULL || persistence_key->led_index == NO_LED) return;

    persistence_state_t state = persistence_get_state();

    switch (state) {
        case PERSISTENCE_SAVED:
            // Sem diferença: branco pulsante
            {
                uint8_t brightness = calculate_pulse_brightness();
                color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
                rgb_matrix_set_color(persistence_key->led_index, white.r, white.g, white.b);
            }
            break;
        case PERSISTENCE_DIFF:
            // Há diferença: verde pulsante
            {
                uint8_t brightness = calculate_pulse_brightness();
                color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
                rgb_matrix_set_color(persistence_key->led_index, green.r, green.g, green.b);
            }
            break;
        case PERSISTENCE_PRESSED:
            // END está pressionado (sem FN): verde contínuo
            {
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(persistence_key->led_index, green.r, green.g, green.b);
            }
            break;
        case PERSISTENCE_SAVING:
            // Salvando: amarelo sólido (pelo menos 3 segundos)
            {
                color_rgb_t yellow = color_get_rgb(COLOR_YELLOW);
                rgb_matrix_set_color(persistence_key->led_index, yellow.r, yellow.g, yellow.b);
            }
            break;
        case PERSISTENCE_BLINKING_SAVED:
            // Verde sólido após salvar com sucesso (3 segundos)
            {
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(persistence_key->led_index, green.r, green.g, green.b);
            }
            break;
        case PERSISTENCE_BLINKING_ERROR:
            // Vermelho sólido após erro (3 segundos)
            {
                color_rgb_t red = color_get_rgb(COLOR_RED);
                rgb_matrix_set_color(persistence_key->led_index, red.r, red.g, red.b);
            }
            break;
    }
}

// ===== Funções de Hook QMK =====

// Processa KC_END (FN+END para salvar, ou apenas END para feedback visual) - baseado em posição, não keycode
bool persistence_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (persistence_key == NULL || key == NULL || key->kind != KIND_PERSISTENCE) {
        return true;
    }
    
    // Verifica se FN está ativo usando keymod
    // persistence se importa APENAS com FN_ONLY (apenas FN pressionado, sem outros modificadores)
    bool fn_active = keymod_equals(keymod, KEYMOD_FN_ONLY);

    switch (key->state) {
        case PERSISTENCE_SAVED:
            if (fn_active) {
                return false; // Consome o evento
            }
            if (pressed) {
                persistence_set_state(PERSISTENCE_PRESSED);
            }
            return true;
        case PERSISTENCE_DIFF:
            if (fn_active && pressed) {
                persistence_save();
                return false; // Consome o evento
            }
            return true;
        case PERSISTENCE_PRESSED:
            if (pressed) {
                return true;
            }
            if (persistence_has_diff()) {
                persistence_set_state(PERSISTENCE_DIFF);
            } else {
                persistence_set_state(PERSISTENCE_SAVED);
            }
            return true;
    }
    return true;
}

// Hook matrix_scan_user
static void persistence_matrix_scan_user(const event_t* event) {
    if (event->type != EVENT_MATRIX_SCAN) return;
    persistence_update_state();
}

// Hook rgb_matrix_indicators_user
static void persistence_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    persistence_update_indicators();
}

// Hook keyboard_post_init_user
static void persistence_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;
    // Obtém ponteiro para a tecla persistence diretamente do pool
    persistence_key = kind_get_persistence_key();
    
    if (persistence_key != NULL) {
        // Registra a posição de KC_END na matriz de behavior
        behavior_register_position_function(persistence_key->row, persistence_key->col, persistence_process_key);
        
        // Inicializa estado baseado em diferença entre persisted e working
        if (persistence_has_diff()) {
            persistence_key->state = PERSISTENCE_DIFF;
        } else {
            persistence_key->state = PERSISTENCE_SAVED;
        }
    }
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void persistence_init_early_hooks(void) {
    event_bus_subscribe(EVENT_MATRIX_SCAN, persistence_matrix_scan_user);
    event_bus_subscribe_rgb_indicators(persistence_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void persistence_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, persistence_keyboard_post_init_user);
}
