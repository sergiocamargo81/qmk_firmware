#include "modifiers.h"

#include "../keymap.h"  // Para WIN_FN
#include "../kind.h"    // Para kind_get_modifier
#include "../keymod.h"  // Para keymod_t e keymod_get
#include "../colors.h"  // Para cores centralizadas
#include "../pulse.h"   // Para calculate_pulse_brightness
#include "../behavior.h"  // Para behavior_register_position_function, behavior_update_keymod
#include "../event_bus.h" // Para Event Bus
#include "rgb_matrix.h"
#include "action.h"  // Para keyrecord_t

// ===== Defines =====
typedef uint8_t modifier_state_t;
enum {
    MODIFIER_IDLE,
    MODIFIER_PRESSED
};

// ===== Variáveis Globais =====

// Sistema de callbacks migrado para Event Bus
static keymod_t fn_keymod_state = KEYMOD_NONE;
static keymod_t fn_keymod_prev_state = KEYMOD_NONE;

// Cache de ponteiros para modifiers do grid (inicializados durante init)
static modifier_t* fn_modifier = NULL;
static modifier_t* ralt_modifier = NULL;
static modifier_t* rctl_modifier = NULL;
static modifier_t* rsft_modifier = NULL;
static bool modifiers_initialized = false;

// ===== Funções Auxiliares =====

// Detecta keymod atual baseado nos estados na matrix de FN e modificadores
// Prioridade: RCTL > RALT > RSFT
static keymod_t detect_fn_keymod(void) {
    if (fn_modifier == NULL || fn_modifier->state != MODIFIER_PRESSED) {
        return KEYMOD_NONE;
    }
    
    // Verifica modificadores em ordem de prioridade: RCTL > RALT > RSFT
    if (rctl_modifier != NULL && rctl_modifier->state == MODIFIER_PRESSED) {
        return KEYMOD_FN_RCTL;
    }
    
    if (ralt_modifier != NULL && ralt_modifier->state == MODIFIER_PRESSED) {
        return KEYMOD_FN_RALT;
    }
    
    if (rsft_modifier != NULL && rsft_modifier->state == MODIFIER_PRESSED) {
        return KEYMOD_FN_RSFT;
    }
    
    // Apenas FN está ativo
    return KEYMOD_FN_ONLY;
}

// Notifica todos os callbacks registrados sobre mudança de estado de FN
static void notify_fn_state_change(keymod_t keymod) {
    // Atualiza keymod em behavior
    behavior_update_keymod(keymod);
    
    // Dispara Event Bus
    event_data_t data = {
        .fn_state_changed = {
            .keymod = keymod
        }
    };
    event_bus_publish(EVENT_FN_STATE_CHANGED, &data);
}

// Verifica se uma tecla modifier está pressionada (usa campo state da matrix)
static bool is_modifier_pressed(modifier_t* modifier) {
    if (modifier == NULL) return false;
    return (modifier->state == MODIFIER_PRESSED);
}

// Atualiza LEDs dos modificadores
static void update_modifier_leds(void) {
    if (!modifiers_initialized) {
        return;
    }
    
    // Itera sobre todos os modifiers e atualiza LEDs
    modifier_t* modifiers[MODIFIERS_COUNT] = {
        fn_modifier, ralt_modifier, rctl_modifier, rsft_modifier
    };
    
    for (uint8_t i = 0; i < MODIFIERS_COUNT; i++) {
        modifier_t* mod = modifiers[i];
        if (mod == NULL) continue;
        
        // FN tem comportamento especial baseado no keymod
        if (mod == fn_modifier) {
            if (keymod_equals(fn_keymod_state, KEYMOD_FN_ONLY)) {
                // Apenas FN: amarelo sólido
                color_rgb_t yellow = color_get_rgb(COLOR_YELLOW);
                rgb_matrix_set_color(mod->led_index, yellow.r, yellow.g, yellow.b);
            } else if (keymod_equals(fn_keymod_state, KEYMOD_FN_RCTL)) {
                // FN+RCTRL: azul sólido
                color_rgb_t blue = color_get_rgb(COLOR_BLUE);
                rgb_matrix_set_color(mod->led_index, blue.r, blue.g, blue.b);
            } else if (keymod_equals(fn_keymod_state, KEYMOD_FN_RALT)) {
                // FN+RALT: verde sólido
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(mod->led_index, green.r, green.g, green.b);
            } else {
                // FN não está pressionada: roxo pulsante
                uint8_t brightness = calculate_pulse_brightness();
                color_rgb_t purple = color_apply_brightness(COLOR_PURPLE, brightness);
                rgb_matrix_set_color(mod->led_index, purple.r, purple.g, purple.b);
            }
        } else if (mod == rctl_modifier) {
            // RCTRL: azul quando FN+RCTRL está ativo, verde contínuo quando pressionada, roxo pulsante quando não
            if (keymod_equals(fn_keymod_state, KEYMOD_FN_RCTL)) {
                // FN+RCTRL: azul sólido (sobrepõe comportamento padrão)
                color_rgb_t blue = color_get_rgb(COLOR_BLUE);
                rgb_matrix_set_color(mod->led_index, blue.r, blue.g, blue.b);
            } else if (is_modifier_pressed(mod)) {
                // Tecla está pressionada: verde contínuo
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(mod->led_index, green.r, green.g, green.b);
            } else {
                // Tecla não está pressionada: roxo pulsante
                uint8_t brightness = calculate_pulse_brightness();
                color_rgb_t purple = color_apply_brightness(COLOR_PURPLE, brightness);
                rgb_matrix_set_color(mod->led_index, purple.r, purple.g, purple.b);
            }
        } else if (mod == ralt_modifier) {
            // RALT: verde quando FN+RALT está ativo, verde contínuo quando pressionada, roxo pulsante quando não
            if (keymod_equals(fn_keymod_state, KEYMOD_FN_RALT)) {
                // FN+RALT: verde sólido (sobrepõe comportamento padrão)
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(mod->led_index, green.r, green.g, green.b);
            } else if (is_modifier_pressed(mod)) {
                // Tecla está pressionada: verde contínuo
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(mod->led_index, green.r, green.g, green.b);
            } else {
                // Tecla não está pressionada: roxo pulsante
                uint8_t brightness = calculate_pulse_brightness();
                color_rgb_t purple = color_apply_brightness(COLOR_PURPLE, brightness);
                rgb_matrix_set_color(mod->led_index, purple.r, purple.g, purple.b);
            }
        } else if (mod == rsft_modifier) {
            // RSFT: verde contínuo quando pressionada, roxo pulsante quando não
            if (is_modifier_pressed(mod)) {
                // Tecla está pressionada: verde contínuo
                color_rgb_t green = color_get_rgb(COLOR_GREEN);
                rgb_matrix_set_color(mod->led_index, green.r, green.g, green.b);
            } else {
                // Tecla não está pressionada: roxo pulsante
                uint8_t brightness = calculate_pulse_brightness();
                color_rgb_t purple = color_apply_brightness(COLOR_PURPLE, brightness);
                rgb_matrix_set_color(mod->led_index, purple.r, purple.g, purple.b);
            }
        }
    }
}

// ===== API Pública =====

// Sistema de callbacks migrado para Event Bus
// Use event_bus_subscribe_fn_state_changed() em vez de modifiers_register_fn_callback()

bool modifiers_is_fn_pressed(void) {
    return keymod_intersects(fn_keymod_state, KEYMOD_FN_ONLY | KEYMOD_FN_RCTL | KEYMOD_FN_RALT | KEYMOD_FN_RSFT);
}

// ===== Inicialização =====

void modifiers_init(void) {
    // Inicializa estado
    fn_keymod_state = KEYMOD_NONE;
    fn_keymod_prev_state = KEYMOD_NONE;
    
    // Atualiza keymod inicial em behavior
    behavior_update_keymod(KEYMOD_NONE);
    
    // Obtém ponteiros para modifiers diretamente do pool
    fn_modifier = kind_get_fn_key();
    ralt_modifier = kind_get_ralt_key();
    rctl_modifier = kind_get_rctl_key();
    rsft_modifier = kind_get_rsft_key();
    
    // Inicializa estados na matrix
    if (fn_modifier) fn_modifier->state = MODIFIER_IDLE;
    if (ralt_modifier) ralt_modifier->state = MODIFIER_IDLE;
    if (rctl_modifier) rctl_modifier->state = MODIFIER_IDLE;
    if (rsft_modifier) rsft_modifier->state = MODIFIER_IDLE;
    
    // Registra handlers para modifiers de interesse (FN, RSHIFT, RALT, RCTRL)
    if (fn_modifier != NULL) {
        behavior_register_position_function(fn_modifier->row, fn_modifier->col, modifiers_process_key);
    }
    if (rsft_modifier != NULL) {
        behavior_register_position_function(rsft_modifier->row, rsft_modifier->col, modifiers_process_key);
    }
    if (ralt_modifier != NULL) {
        behavior_register_position_function(ralt_modifier->row, ralt_modifier->col, modifiers_process_key);
    }
    if (rctl_modifier != NULL) {
        behavior_register_position_function(rctl_modifier->row, rctl_modifier->col, modifiers_process_key);
    }
    
    modifiers_initialized = true;
}

// ===== Processamento de Eventos =====

// Processa eventos de modifiers (FN, RSHIFT, RALT, RCTRL)
// Retorna false se consumiu o evento, true caso contrário
bool modifiers_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (!modifiers_initialized) {
        return true;
    }
    
    if (key == NULL || key->kind != KIND_MODIFIER) {
        return true;
    }
    
    // Atualiza estado na matrix baseado no evento
    modifier_t* modifier = (modifier_t*)key;
    if (modifier != NULL) {
        modifier_state_t old_state = modifier->state;
        modifier->state = pressed ? MODIFIER_PRESSED : MODIFIER_IDLE;
        
        // Se houve mudança de estado em qualquer modifier de interesse, recalcula keymod e notifica
        // Sempre recalcula quando há mudança, mesmo que seja em outros modifiers
        // Isso garante que combinações como FN+RCTRL sejam detectadas corretamente
        if (old_state != modifier->state) {
            keymod_t new_keymod = detect_fn_keymod();
            // Sempre atualiza o estado, mesmo se for o mesmo valor
            // Isso garante que o estado está sincronizado
            if (!keymod_equals(new_keymod, fn_keymod_state)) {
                fn_keymod_state = new_keymod;
                notify_fn_state_change(fn_keymod_state);
            }
            fn_keymod_prev_state = fn_keymod_state;
        }
    }
    
    // Não consome o evento, permite que seja processado normalmente
    return true;
}

// Hook rgb_matrix_indicators_user: atualiza LEDs dos modificadores
static void modifiers_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    update_modifier_leds();
}

// ===== Registro de Hooks =====

void modifiers_init_hooks(void) {
    // Registra hook de rgb_indicators para atualizar LEDs
    event_bus_subscribe_rgb_indicators(modifiers_rgb_matrix_indicators_user);
}
