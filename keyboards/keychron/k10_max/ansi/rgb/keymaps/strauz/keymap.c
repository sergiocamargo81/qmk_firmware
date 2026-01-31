#include QMK_KEYBOARD_H

#include "keymap.h"

#include "keychron_common.h"

#include "behavior.h"
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
#include "unused/unused.h"
#include "bold/bold.h"
#include "unused_modifiers/unused_modifiers.h"
#include "settings.h"
#include "profile.h"
#include "custom_behaviors.h"
#include "event_bus.h"
#include "leds/leds.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [WIN_BASE] = LAYOUT_ansi_108(
        KC_ESC,                  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,     KC_PSCR,  KC_CTANA, UG_TOGG,  _______,  _______,  _______,  _______,
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
    if (!process_record_keychron_common(keycode, record)) {
        return false;
    }

    // Ignora eventos não-tecla
    if (record->event.type != KEY_EVENT) {
        return true;
    }

    // Notifica atividade do teclado (para sistema de timeout de LEDs)
    event_bus_publish_void(EVENT_ACTIVITY_DETECTED);

    // Obtém posição (row, col) do evento
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    bool pressed = record->event.pressed;
    
    // Obtém a key associada à posição através do behavior
    base_key_t* key = behavior_get_key_by_position(row, col);
    if (key == NULL) {
        return true; // Não há key associada, passa o evento adiante
    }
    
    // Obtém o keymod atual mantido por behavior
    keymod_t keymod = behavior_get_current_keymod();
    
    // Verifica se keymod é aceito para esta key
    // O teste é direto: verifica se o keymod atual (apenas um valor) está contido nas accepted_keymods (flags)
    if (!keymod_intersects(keymod, key->accepted_keymods)) {
        return true; // keymod não aceito, não consome e retorna
    }
    
    // Chama o handler registrado, passando a struct da key, se está pressionada, e o keymod
    return key->process_key(key, pressed, keymod);
}

// ===== Hooks QMK =====
// Ordem de execução:
// 1. keyboard_pre_init_user()     - ANTES de quantum_init() (primeiro a executar)
// 2. eeconfig_init_user()        - DURANTE quantum_init() (se EEPROM inválida)
// 3. keyboard_post_init_user()   - DEPOIS de keyboard_init() (após hardware inicializado)
// 4. matrix_scan_user()          - PERIODICAMENTE no loop principal (~1000 Hz)
// 5. rgb_matrix_indicators_user() - PERIODICAMENTE durante renderização RGB

// ===== 1. keyboard_pre_init_user =====
// Hook executado ANTES de quantum_init() para garantir que callbacks estejam disponíveis quando necessário
// Necessário porque:
// - eeconfig_init_user é chamado durante quantum_init(), antes de keyboard_post_init_user()
// - matrix_scan_user e rgb_matrix_indicators_user não têm dependências de inicialização
void keyboard_pre_init_user(void) {
    // Inicializa Event Bus primeiro (deve ser chamado antes de eeconfig_init_user)
    event_bus_init();
    
    // Registra hooks que não dependem de inicialização completa
    hold_init_early_hooks();
    toggle_init_early_hooks();
    unassociated_init_early_hooks();
    numlock_init_early_hooks();
    profiles_init_early_hooks();
    persistence_init_early_hooks();
    unused_init_early_hooks();
    bold_init_early_hooks();
    leds_init_early_hooks();
}

// ===== 2. eeconfig_init_user =====
// Hook executado DURANTE quantum_init() se EEPROM não estiver inicializada
// Chamado após keyboard_pre_init_user() mas antes de keyboard_post_init_user()
void eeconfig_init_user(void) {
    event_bus_publish_void(EVENT_EECONFIG_INIT);
}

// ===== 3. keyboard_post_init_user =====
// Hook executado DEPOIS de keyboard_init() após todo hardware estar inicializado
// Inicializa todos os módulos e registra seus handlers no Event Bus
void keyboard_post_init_user(void) {
    // Event Bus já foi inicializado em keyboard_pre_init_user()
    
    // Inicializa grid primeiro (deve ser chamado antes de qualquer módulo que use o grid)
    kind_init_grid();

    // Inicializa modifiers primeiro (outros módulos podem registrar callbacks)
    modifiers_init();
    unused_modifiers_init();

    // Inicializa leds
    leds_init();

    // Registra hooks de todos os módulos (exceto eeconfig_init, já registrados em keyboard_pre_init_user)
    modifiers_init_hooks();
    unused_modifiers_init_hooks();
    leds_init_hooks();
    custom_init_hooks();
    hold_init_hooks();
    toggle_init_hooks();
    unassociated_init_hooks();
    numlock_init_hooks();
    profiles_init_hooks();
    persistence_init_hooks();
    unused_init_hooks();
    bold_init_hooks();

    // Inicializa settings APÓS todos os hooks serem registrados
    // Isso garante que EVENT_SETTINGS_LOADED seja recebido pelos handlers
    settings_init();

    // Registra callbacks de notificação de FN dos submódulos via Event Bus
    // Os submódulos não conhecem modifiers, apenas expõem seus callbacks
    event_bus_subscribe_fn_state_changed(hold_fn_state_callback);
    event_bus_subscribe_fn_state_changed(toggle_fn_state_callback);
    event_bus_subscribe_fn_state_changed(unassociated_fn_state_callback);
    
    // Notifica estado inicial de FN aos submódulos
    // Isso garante sincronização do estado inicial
    // O estado será atualizado no primeiro matrix_scan_user
    // Por enquanto, notifica KEYMOD_NONE (estado inicial)
    event_data_t fn_none_data = {
        .fn_state_changed = {.keymod = KEYMOD_NONE}
    };
    event_bus_publish(EVENT_FN_STATE_CHANGED, &fn_none_data);
    
    // Dispara inicialização de todos os módulos registrados
    event_bus_publish_void(EVENT_KEYBOARD_POST_INIT);

    // NOTE: A sincronização de behaviors é feita automaticamente via EVENT_SETTINGS_LOADED
    // disparado pelo settings_init() durante a inicialização
}

// ===== 4. matrix_scan_user =====
// Hook executado PERIODICAMENTE no loop principal (~1000 Hz)
// Usado para scans periódicos dos módulos (timers, debounce, etc.)
void matrix_scan_user(void) {
    event_bus_publish_void(EVENT_MATRIX_SCAN);
}

// ===== 5. rgb_matrix_indicators_user =====
// Hook executado PERIODICAMENTE durante renderização RGB
// Usado para atualizar LEDs dos módulos (indicadores de estado)
bool rgb_matrix_indicators_user(void) {
    // Decide se deve mostrar cores dos módulos baseado no estado
    if (leds_should_show_modules()) {
        // Estado permite mostrar cores dos módulos
        // Apenas sobrescreve LEDs com valores estipulados pelos módulos
        // LEDs sem valores estipulados mantêm as cores padrão do sistema
        event_bus_publish_void(EVENT_RGB_INDICATORS);
    }
    // Se não deve mostrar módulos, apenas usa cores padrão do teclado

    return true;
}
