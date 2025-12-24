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
#include "numlock/numlock.h"
#include "profiles/profiles.h"
#include "persistence/persistence.h"
#include "settings.h"
#include "profiles.h"
#include "customs.h"

// ===== Função Auxiliar para Estado dos Modificadores =====

// Obtém o estado dos modificadores no momento do evento
// Retorna um keymod_t enum conforme os modificadores ativos
// Otimização: se FN não estiver ativo, não calcula RCTL e RALT
// Prioridade: RCTL tem prioridade sobre RALT (se ambos estão ativos, retorna KEYMOD_FN_RCTL)
static keymod_t get_keymod(keyrecord_t *record) {
    // Obtém modificadores efetivos no momento do evento
    uint8_t effective_mods = get_mods() | get_weak_mods() | get_oneshot_mods();

    // Se o keycode for um modificador e estiver sendo pressionado, adiciona aos mods efetivos
    // (porque get_mods() ainda não foi atualizado com este modificador)
    uint8_t source_layer = read_source_layers_cache(record->event.key);
    if (source_layer < 255) {
    uint16_t keycode = keymap_key_to_keycode(source_layer, record->event.key);
    if (IS_MODIFIER_KEYCODE(keycode) && record->event.pressed) {
        effective_mods |= MOD_BIT(keycode);
    }
    }

    // Verifica se outros modificadores estão ativos (LSHIFT, LCTL, LALT, RSHIFT, LWIN, RWIN)
    // Se algum estiver ativo, retorna INVALID
    if (effective_mods & (MOD_BIT(KC_LSFT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LALT) | 
                          MOD_BIT(KC_RSFT) | MOD_BIT(KC_LWIN) | MOD_BIT(KC_RWIN))) {
        return KEYMOD_INVALID;
    }

    // Verifica se RCTL ou RALT estão ativos
    bool rctl_active = (effective_mods & MOD_BIT(KC_RCTL)) != 0;
    bool ralt_active = (effective_mods & MOD_BIT(KC_RALT)) != 0;
    
    // Se RCTL ou RALT estão ativos, verifica se FN também está ativo
    if (rctl_active || ralt_active) {
        // Verifica se FN layer está ativo (source layer ou layer state)
        bool fn_active = (source_layer == WIN_FN) || layer_state_is(WIN_FN);
        
        if (fn_active) {
            // FN está ativo com RCTL ou RALT
            if (rctl_active) {
        return KEYMOD_FN_RCTL;
    }
            if (ralt_active) {
                return KEYMOD_FN_RALT;
            }
        } else {
            // RCTL ou RALT ativos sem FN = inválido
            return KEYMOD_INVALID;
        }
    }
    
    // Verifica se FN layer está ativo
    // Verifica tanto a layer source quanto a layer state (para teclas não mapeadas na layer FN)
    bool fn_active = (source_layer == WIN_FN) || layer_state_is(WIN_FN);
    
    // Se a tecla atual for MO(WIN_FN), considera FN ativo
    if (source_layer < 255) {
        uint16_t keycode = keymap_key_to_keycode(source_layer, record->event.key);
        if (keycode == MO(WIN_FN)) {
            fn_active = true;
        }
    }
    
    // Verifica se MO(WIN_FN) está sendo pressionado diretamente na matriz
    // MO(WIN_FN) está na posição (5, 9) no keymap
    // Isso garante que FN seja detectado mesmo se a layer não estiver ativa ainda
    if (!fn_active) {
        if (matrix_is_on(5, 9)) {
            fn_active = true;
        }
    }
    
    // Verifica se a layer FN está ativa (mais confiável que verificar posições individuais)
    // Isso é necessário porque quando você pressiona MO(WIN_FN) + outra tecla,
    // a layer pode estar ativa mesmo que source_layer não seja WIN_FN
    if (!fn_active) {
        fn_active = layer_state_is(WIN_FN);
    }
    
    // Se FN não estiver ativo, retorna NONE
    if (!fn_active) {
        return KEYMOD_NONE;
    }

    // Apenas FN está ativo (sem RCTL ou RALT)
    return KEYMOD_FN;
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [WIN_BASE] = LAYOUT_ansi_108(
        KC_ESC,             KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,     KC_F12,     KC_PSCR,  KC_CTANA, UG_NEXT,  _______,  _______,  _______,  _______,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,     KC_BSPC,    KC_INS,   KC_HOME,  KC_PGUP,  KC_NUM,   KC_PSLS,  KC_PAST,  KC_PMNS,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,    KC_BSLS,    KC_DEL,   KC_END,   KC_PGDN,  KC_P7,    KC_P8,    KC_P9,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,                                   KC_P4,    KC_P5,    KC_P6,    KC_PPLS,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,              KC_UP,              KC_P1,    KC_P2,    KC_P3,
        KC_LCTL,  KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  KC_RWIN,  MO(WIN_FN), KC_RCTL,    KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,              KC_PDOT,  KC_PENT),

    [WIN_FN] = LAYOUT_ansi_108(
        _______,            KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,    KC_VOLU,    _______,  _______,  UG_TOGG,  _______,  _______,  _______,  _______,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,  _______,
        UG_TOGG,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,  _______,  _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                                  _______,  _______,  _______,  _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,              _______,            _______,  _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,  _______,  _______,  _______,            _______,  _______)
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
    // Todos os módulos (profiles, persistence, custom, numlock) registram suas funções aqui
    key_function_t handler;
    keymod_t supported_keymod;
    if (!behavior_resolve_handler_by_position(row, col, &handler, &supported_keymod)) {
        return true; // Não há handler associado, passa o evento adiante
    }
    
    // Verifica se é tecla custom (precisa de keymod para associação mesmo com supported_keymod == KEYMOD_NONE)
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL) {
        return true; // Entry não encontrado, passa adiante
    }
    
    bool is_custom = (entry->kind == KIND_CUSTOM);
    bool is_position = (entry->kind == KIND_POSITION);
    
    // Calcula keymod - SEMPRE calcula para custom e position_t (igual ao custom)
    keymod_t keymod = KEYMOD_NONE;
    if (is_custom || is_position) {
        keymod = get_keymod(record);
        // Se keymod for INVALID, passa adiante
        if (keymod == KEYMOD_INVALID) {
            return true;
        }
        
        // Para position_t, usa a mesma lógica de custom: verifica bitwise
        if (is_position) {
            // Para position_t, verifica se há interseção entre keymod e supported_keymod
            // keymod é um enum de flags, pode conter múltiplos bits
            // Usa a mesma lógica de custom
            if (supported_keymod != 0) {
                // Verifica se há algum bit em comum entre keymod e supported_keymod
                if ((keymod & supported_keymod) == 0) {
                    return true; // Nenhum bit em comum, keymod não está na lista de aceitos, passa adiante
                }
            }
        } else if (is_custom) {
            // Para custom, verifica se keymod contém algum dos valores aceitos
            // supported_keymod é uma máscara de bits: cada bit indica se o valor é aceito
            // KEYMOD_NONE=0x01, KEYMOD_FN=0x02, KEYMOD_FN_RCTL=0x04, KEYMOD_FN_RALT=0x08
            // keymod pode conter múltiplos bits setados (combinação de flags)
            // Verifica se há interseção entre keymod e supported_keymod
            if (supported_keymod != 0) {
                // Verifica se há algum bit em comum entre keymod e supported_keymod
                if ((keymod & supported_keymod) == 0) {
                    return true; // Nenhum bit em comum, keymod não está na lista de aceitos, passa adiante
                }
            }
        }
    }
    
    // Chama o handler registrado, passando record e keymod (não keycode)
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
    numlock_init_early_hooks();
    profiles_init_early_hooks();
    persistence_init_early_hooks();
}

// Hook keyboard_post_init_user: inicialização dos módulos
// Segundo hook executado - chamado no final de keyboard_init() após todo hardware estar inicializado
void keyboard_post_init_user(void) {
    // Inicializa grid primeiro (deve ser chamado antes de qualquer módulo que use o grid)
    kind_init_grid();
    
    // Inicializa settings (deve ser chamado antes de qualquer módulo que dependa de settings)
    settings_init();
    
    // Registra hooks de todos os módulos (exceto eeconfig_init, já registrados em keyboard_pre_init_user)
    custom_init_hooks();
    hold_init_hooks();
    toggle_init_hooks();
    numlock_init_hooks();
    profiles_init_hooks();
    persistence_init_hooks();
    
    // Dispara inicialização de todos os módulos registrados
    hooks_keyboard_post_init_dispatch();
}

// Função auxiliar para verificar se um profile está vazio
static bool profile_is_empty(profile_t* profile) {
    if (!profile) return true;
    for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
        if (profile->behaviors[i] != KEY_CUSTOM_DISABLED) {
            return false;
        }
    }
    return true;
}

// Hook matrix_scan_user: scans periódicos dos módulos
// Terceiro hook executado - chamado periodicamente no loop principal (~1000 Hz)
void matrix_scan_user(void) {
    hooks_matrix_scan_dispatch();
}

// Hook rgb_matrix_indicators_user: indicadores RGB dos módulos
// Quarto hook executado - chamado periodicamente durante renderização RGB
bool rgb_matrix_indicators_user(void) {
    // Verifica se o profile não está vazio
    settings_t* working = settings_get_working();
    profile_t* active_profile = profiles_get_active_profile(&working->profiles);
    bool profile_empty = (active_profile == NULL) || profile_is_empty(active_profile);
    
    if (!profile_empty) {
        // Profile não está vazio: desliga todos os LEDs primeiro
        // Isso sobrescreve o padrão RGB que foi renderizado antes
        rgb_matrix_set_color_all(0, 0, 0);
    }
    
    // Chama todos os indicadores registrados (sempre, independente do estado do profile)
    // Indicadores de numlock, end e profiles devem funcionar sempre
    // Eles serão chamados DEPOIS de desligar tudo (se profile não estiver vazio)
    // ou diretamente (se profile estiver vazio)
    hooks_rgb_indicators_dispatch();
    
    return true;
}
