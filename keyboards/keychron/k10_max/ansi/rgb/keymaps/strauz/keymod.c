#include "keymod.h"
#include "keymap.h"  // Para WIN_FN
#include "modifiers/modifiers.h"  // Para modifiers_is_fn_pressed
#include QMK_KEYBOARD_H

// ===== Função para Estado dos Modificadores =====

// Obtém o estado dos modificadores no momento do evento
// Retorna um keymod_t enum conforme os modificadores ativos
// Prioridade: RCTL > RALT > RSFT
keymod_t keymod_get(keyrecord_t *record) {
    // Lê source_layer uma única vez no início
    uint8_t source_layer = read_source_layers_cache(record->event.key);
    
    // Cacheia keycode uma única vez (usado em múltiplos lugares)
    uint16_t keycode = 0;
    bool keycode_valid = false;
    if (source_layer < 255) {
        keycode = keymap_key_to_keycode(source_layer, record->event.key);
        keycode_valid = true;
    }
    
    // Verifica se FN layer está ativo
    // Caso mais comum primeiro: se source_layer já é WIN_FN, FN está ativo
    bool fn_active = (source_layer == WIN_FN);
    
    // Se ainda não detectou FN, verifica outras condições (menos comuns)
    if (!fn_active) {
        // Cacheia layer_state_is(WIN_FN) apenas se necessário
        fn_active = layer_state_is(WIN_FN);
        
        // Se ainda não detectou, verifica se a tecla atual é MO(WIN_FN)
        if (!fn_active && keycode_valid && keycode == MO(WIN_FN)) {
            fn_active = true;
        }
        
        // Última verificação: se MO(WIN_FN) está sendo pressionado diretamente
        // MO(WIN_FN) está na posição (5, 12) no keymap
        // Usa modifiers_is_fn_pressed() para verificar via state ao invés de matrix_is_on
        if (!fn_active) {
            extern bool modifiers_is_fn_pressed(void);
            fn_active = modifiers_is_fn_pressed();
        }
    }
    
    // Se FN não estiver ativo, retorna NONE (early return)
    if (!fn_active) {
        return KEYMOD_NONE;
    }

    // Obtém modificadores efetivos no momento do evento
    uint8_t effective_mods = get_mods() | get_weak_mods() | get_oneshot_mods();

    // Se o keycode for um modificador e estiver sendo pressionado, adiciona aos mods efetivos
    // (porque get_mods() ainda não foi atualizado com este modificador)
    if (keycode_valid && IS_MODIFIER_KEYCODE(keycode) && record->event.pressed) {
        effective_mods |= MOD_BIT(keycode);
    }

    // Verifica modificadores em ordem de prioridade: RCTL > RALT > RSFT
    // Retorna um modo único (não combinação de flags)
    if (effective_mods & MOD_BIT(KC_RCTL)) {
        return KEYMOD_FN_RCTL; // RCTL tem prioridade
    }
    
    if (effective_mods & MOD_BIT(KC_RALT)) {
        return KEYMOD_FN_RALT; // RALT tem prioridade sobre RSFT
    }
    
    if (effective_mods & MOD_BIT(KC_RSFT)) {
        return KEYMOD_FN_RSFT;
    }

    // Apenas FN está ativo (sem RCTL, RALT ou RSFT)
    return KEYMOD_FN_ONLY;
}

// Verifica se keymod contém APENAS o valor especificado (sem outros flags)
// value pode ser uma combinação de flags (ex: KEYMOD_FN_RCTL | KEYMOD_FN_RALT)
bool keymod_is_only_value(keymod_t keymod, keymod_t value) {
    return (keymod == value);
}

// Verifica se keymod contém o valor especificado (pode ter outros flags também)
// value pode ser uma combinação de flags (ex: KEYMOD_FN_ONLY | KEYMOD_FN_RCTL)
// Retorna true se keymod corresponde a qualquer um dos valores em value
bool keymod_has_value(keymod_t keymod, keymod_t value) {
    if (value == KEYMOD_NONE) {
        return (keymod == KEYMOD_NONE);
    }
    // Verifica se keymod corresponde a qualquer um dos flags em value
    // Exemplo: se value = KEYMOD_FN_ONLY | KEYMOD_FN_RCTL, retorna true se keymod for KEYMOD_FN_ONLY ou KEYMOD_FN_RCTL
    return (keymod & value) != 0;
}

// Verifica se keymod está aceito para position
bool keymod_is_accepted_for_position(keymod_t keymod, keymod_t accepted_keymods) {
    if (accepted_keymods == KEYMOD_NONE) {
        return true; // Aceita qualquer keymod
    }
    // Verifica se há interseção entre keymod e accepted_keymods
    return keymod_has_value(keymod, accepted_keymods);
}

// Verifica se keymod está aceito para custom
bool keymod_is_accepted_for_custom(keymod_t keymod, keymod_t accepted_keymods) {
    if (accepted_keymods == KEYMOD_NONE) {
        return true; // Aceita qualquer keymod
    }
    
    // KEYMOD_CUSTOM_MASK aceita KEYMOD_NONE, KEYMOD_FN_RCTL ou KEYMOD_FN_RALT
    keymod_t custom_mask = KEYMOD_NONE | KEYMOD_FN_RCTL | KEYMOD_FN_RALT;
    
    if (accepted_keymods == custom_mask) {
        // É KEYMOD_CUSTOM_MASK, verifica se keymod corresponde a algum dos valores aceitos
        return keymod_has_value(keymod, custom_mask);
    }
    
    // É um keymod único ou combinação, verifica se há interseção
    return keymod_has_value(keymod, accepted_keymods);
}
