#include QMK_KEYBOARD_H

#include "keymap.h"

#include "keychron_common.h"

#include "behavior.h"
#include "hooks.h"
#include "keymod.h"
#include "kind.h"
#include "custom/custom.h"
#include "custom/hold/hold.h"
#include "custom/toggle/toggle.h"
#include "custom/unassociated/unassociated.h"
#include "numlock/numlock.h"
#include "profiles/profiles.h"
#include "persistence/persistence.h"
#include "modifiers/modifiers.h"
#include "disabled/disabled.h"
#include "bold/bold.h"
#include "others/others.h"
#include "settings.h"
#include "profile.h"
#include "customs.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [WIN_BASE] = LAYOUT_ansi_108(
        KC_ESC,                  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,     KC_PSCR,  KC_CTANA, UG_NEXT,  _______,  _______,  _______,  _______,
        KC_GRV,        KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,     KC_BSPC,    KC_INS,   KC_HOME,  KC_PGUP,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,        KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,    KC_BSLS,    KC_DEL,   KC_END,   KC_PGDN,  KC_P7,    KC_P8,    KC_P9,
        LSFT(KC_TAB),  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,                                   KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,                 KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,              KC_UP,              KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,       KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  KC_RWIN,  MO(WIN_FN), KC_RCTL,    KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT,  KC_PENT),

    [WIN_FN] = LAYOUT_ansi_108(
        _______,                 KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,    KC_VOLU,    _______,  _______,  UG_TOGG,  _______,  _______,  _______,  _______,
        _______,       BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,
        UG_TOGG,       UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,
        _______,       UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                                  _______,  _______,  _______,  _______,
        _______,                 _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,              _______,            _______,  _______,  _______,
        _______,       _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,            _______,  _______)
};
// clang-format on

// Hook process_record_user: processa eventos de teclas e encaminha para os handlers registrados
// Executado quando uma tecla é pressionada ou liberada
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Ignora eventos não-tecla
    if (record->event.type != KEY_EVENT) {
        return true;
    }
    
    // Obtém posição (row, col) do evento
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    
    // Resolve o handler associado à posição através do grid
    key_function_t handler;
    keymod_t accepted_keymods;
    if (!behavior_resolve_handler_by_position(row, col, &handler, &accepted_keymods)) {
        return true; // Não há handler associado, passa o evento adiante
    }
    
    // Obtém entry do grid para verificar tipo
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL) {
        return true; // Entry não encontrado, passa adiante
    }
    
    // Calcula keymod apenas se necessário (custom ou position)
    // Modifiers e others não precisam de keymod, apenas passam o evento
    keymod_t keymod = KEYMOD_NONE;
    kind_t entry_kind = entry->kind;
    
    if (entry_kind == KIND_CUSTOM || entry_kind == KIND_PROFILE || entry_kind == KIND_NUMLOCK || entry_kind == KIND_PERSISTENCE) {
        keymod = keymod_get(record);
        
        // Verifica se keymod é aceito
        bool keymod_accepted;
        if (entry_kind == KIND_PROFILE || entry_kind == KIND_NUMLOCK || entry_kind == KIND_PERSISTENCE) {
            keymod_accepted = keymod_is_accepted_for_position(keymod, accepted_keymods);
        } else {
            keymod_accepted = keymod_is_accepted_for_custom(keymod, accepted_keymods);
        }
        
        if (!keymod_accepted) {
            return true; // keymod não aceito, passa adiante
        }
    } else if (entry_kind == KIND_MODIFIER || entry_kind == KIND_OTHER || entry_kind == KIND_DISABLED || entry_kind == KIND_BOLD) {
        // Modifiers, others, disabled e bold não precisam de keymod, sempre passa KEYMOD_NONE
        keymod = KEYMOD_NONE;
    }
    
    // Chama o handler registrado, passando record e keymod
    return handler(record, keymod);
}

// ===== Hooks QMK =====
// Ordem de execução: eeconfig_init_user → keyboard_post_init_user → matrix_scan_user → rgb_matrix_indicators_user

// Hook eeconfig_init_user: reset de EEPROM quando inválida
// Primeiro hook executado - chamado durante quantum_init() se EEPROM não estiver inicializada
void eeconfig_init_user(void) {
    hooks_eeconfig_init_dispatch();
}

// Hook keyboard_pre_init_user: registra hooks que podem ser registrados antes da inicialização completa
// Executado antes de quantum_init() para garantir que callbacks estejam disponíveis quando necessário
// Necessário porque:
// - eeconfig_init_user é chamado durante quantum_init(), antes de keyboard_post_init_user()
// - matrix_scan_user e rgb_matrix_indicators_user não têm dependências de inicialização
void keyboard_pre_init_user(void) {
    // Registra hooks que não dependem de inicialização completa
    hold_init_early_hooks();
    toggle_init_early_hooks();
    unassociated_init_early_hooks();
    numlock_init_early_hooks();
    profiles_init_early_hooks();
    persistence_init_early_hooks();
    disabled_init_early_hooks();
    bold_init_early_hooks();
}

// Hook keyboard_post_init_user: inicialização dos módulos
// Segundo hook executado - chamado no final de keyboard_init() após todo hardware estar inicializado
void keyboard_post_init_user(void) {
    // Inicializa grid primeiro (deve ser chamado antes de qualquer módulo que use o grid)
    kind_init_grid();
    
    // Inicializa settings (deve ser chamado antes de qualquer módulo que dependa de settings)
    settings_init();
    
    // Inicializa modifiers primeiro (outros módulos podem registrar callbacks)
    modifiers_init();
    others_init();
    
    // Registra hooks de todos os módulos (exceto eeconfig_init, já registrados em keyboard_pre_init_user)
    modifiers_init_hooks();
    others_init_hooks();
    custom_init_hooks();
    hold_init_hooks();
    toggle_init_hooks();
    unassociated_init_hooks();
    numlock_init_hooks();
    profiles_init_hooks();
    persistence_init_hooks();
    disabled_init_hooks();
    bold_init_hooks();
    
    // Registra callbacks de notificação de FN dos submódulos
    // Os submódulos não conhecem modifiers, apenas expõem seus callbacks
    modifiers_register_fn_callback((modifiers_fn_state_callback_t)hold_get_fn_callback());
    modifiers_register_fn_callback((modifiers_fn_state_callback_t)toggle_get_fn_callback());
    modifiers_register_fn_callback((modifiers_fn_state_callback_t)unassociated_get_fn_callback());
    
    // Notifica estado inicial de FN aos submódulos
    // Isso garante sincronização do estado inicial
    // O estado será atualizado no primeiro matrix_scan_user
    // Por enquanto, notifica KEYMOD_NONE (estado inicial)
    hold_get_fn_callback()(KEYMOD_NONE);
    toggle_get_fn_callback()(KEYMOD_NONE);
    unassociated_get_fn_callback()(KEYMOD_NONE);
    
    // Dispara inicialização de todos os módulos registrados
    hooks_keyboard_post_init_dispatch();
}

// Hook matrix_scan_user: scans periódicos dos módulos
// Terceiro hook executado - chamado periodicamente no loop principal (~1000 Hz)
void matrix_scan_user(void) {
    hooks_matrix_scan_dispatch();
}

// Hook rgb_matrix_indicators_user: indicadores RGB dos módulos
// Quarto hook executado - chamado periodicamente durante renderização RGB
bool rgb_matrix_indicators_user(void) {
    // Se o profile ativo estiver vazio, não renderiza cores dos módulos
    // Isso permite que as cores padrão do teclado sejam exibidas
    if (profile_is_active_profile_empty()) {
        return true;
    }
    
    // Apenas sobrescreve LEDs com valores estipulados pelos módulos
    // LEDs sem valores estipulados mantêm as cores padrão do sistema
    hooks_rgb_indicators_dispatch();
    
    return true;
}
