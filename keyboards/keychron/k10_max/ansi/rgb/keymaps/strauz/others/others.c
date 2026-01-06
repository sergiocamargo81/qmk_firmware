#include "others.h"

#include "../kind.h"
#include "../behavior.h"
#include "../colors.h"
#include "../pulse.h"
#include "../hooks.h"
#include "../keymod.h"
#include "rgb_matrix.h"

// ===== Defines =====
typedef uint8_t others_state_t;
enum {
    OTHERS_IDLE,
    OTHERS_PRESSED
};

// ===== Variáveis Globais =====

// Cache de ponteiros para others do grid (inicializados durante init)
static others_t* tab_other = NULL;
static others_t* lsft_tab_other = NULL;
static others_t* lshift_other = NULL;
static others_t* lctl_other = NULL;
static others_t* lwin_other = NULL;
static others_t* lalt_other = NULL;
static others_t* rwin_other = NULL;
static bool others_initialized = false;

// ===== Funções Auxiliares =====

// Atualiza LEDs dos others
static void update_others_leds(void) {
    if (!others_initialized) {
        return;
    }
    
    // Itera sobre todos os others e atualiza LEDs
    others_t* others_list[OTHER_COUNT] = {
        tab_other, lsft_tab_other, lshift_other, lctl_other,
        lwin_other, lalt_other, rwin_other
    };
    
    for (uint8_t i = 0; i < OTHER_COUNT; i++) {
        others_t* other = others_list[i];
        if (other == NULL) continue;
        
        // Todos os others: roxo contínuo sempre (não muda quando pressionada)
        color_rgb_t purple = color_get_rgb(COLOR_PURPLE);
        rgb_matrix_set_color(other->led_index, purple.r, purple.g, purple.b);
    }
}

// ===== Inicialização =====

void others_init(void) {
    // Obtém ponteiros para others do grid
    tab_other = kind_get_other(2, 0);  // TAB
    lsft_tab_other = kind_get_other(3, 0);  // LSFT(KC_TAB)
    lshift_other = kind_get_other(4, 0);  // LSHIFT
    lctl_other = kind_get_other(5, 0);  // LCONTROL
    lwin_other = kind_get_other(5, 1);  // LWIN
    lalt_other = kind_get_other(5, 2);  // LALT
    rwin_other = kind_get_other(5, 11);  // RWIN
    
    // Inicializa estados na matrix
    if (tab_other) tab_other->state = OTHERS_IDLE;
    if (lsft_tab_other) lsft_tab_other->state = OTHERS_IDLE;
    if (lshift_other) lshift_other->state = OTHERS_IDLE;
    if (lctl_other) lctl_other->state = OTHERS_IDLE;
    if (lwin_other) lwin_other->state = OTHERS_IDLE;
    if (lalt_other) lalt_other->state = OTHERS_IDLE;
    if (rwin_other) rwin_other->state = OTHERS_IDLE;
    
    // Registra handlers para todos os others
    extern bool others_process_record_user(keyrecord_t *record, keymod_t keymod);
    if (tab_other != NULL) {
        behavior_register_position_function(tab_other->row, tab_other->col, others_process_record_user);
    }
    if (lsft_tab_other != NULL) {
        behavior_register_position_function(lsft_tab_other->row, lsft_tab_other->col, others_process_record_user);
    }
    if (lshift_other != NULL) {
        behavior_register_position_function(lshift_other->row, lshift_other->col, others_process_record_user);
    }
    if (lctl_other != NULL) {
        behavior_register_position_function(lctl_other->row, lctl_other->col, others_process_record_user);
    }
    if (lwin_other != NULL) {
        behavior_register_position_function(lwin_other->row, lwin_other->col, others_process_record_user);
    }
    if (lalt_other != NULL) {
        behavior_register_position_function(lalt_other->row, lalt_other->col, others_process_record_user);
    }
    if (rwin_other != NULL) {
        behavior_register_position_function(rwin_other->row, rwin_other->col, others_process_record_user);
    }
    
    others_initialized = true;
}

// ===== Processamento de Eventos =====

// Processa eventos de others
// Retorna false se consumiu o evento, true caso contrário
bool others_process_record_user(keyrecord_t *record, keymod_t keymod) {
    if (!others_initialized) {
        return true;
    }
    
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    bool pressed = record->event.pressed;
    
    // Atualiza estado na matrix baseado no evento
    others_t* other = kind_get_other(row, col);
    if (other != NULL) {
        other->state = pressed ? OTHERS_PRESSED : OTHERS_IDLE;
    }
    
    // Não consome o evento, permite que seja processado normalmente
    return true;
}

// Hook rgb_matrix_indicators_user: atualiza LEDs dos others
static bool others_rgb_matrix_indicators_user(void) {
    update_others_leds();
    return true;
}

// ===== Registro de Hooks =====

void others_init_hooks(void) {
    // Registra hook de rgb_indicators para atualizar LEDs
    hooks_rgb_indicators_register(others_rgb_matrix_indicators_user);
}
