#ifndef KEYMOD_H
#define KEYMOD_H

#include QMK_KEYBOARD_H

// ===== Estado dos Modificadores =====

// Enum de flags para representar estados de modificadores
// Valores são potências de 2 para permitir operações bitwise
// Cada valor é um modo único (FN_ONLY não está contido em FN_RCTL, etc)
typedef enum {
    KEYMOD_NONE     = 0x00,  // Nenhum modificador ativo
    KEYMOD_FN_ONLY  = 0x01,  // Apenas FN ativo (sem outros modificadores)
    KEYMOD_FN_RCTL  = 0x02,  // FN + RCTL ativos (modo único, não contém FN_ONLY)
    KEYMOD_FN_RALT  = 0x04,  // FN + RALT ativos (modo único, não contém FN_ONLY)
    KEYMOD_FN_RSFT  = 0x08,  // FN + RSHIFT ativos (modo único, não contém FN_ONLY)
} keymod_t;

// ===== Funções =====

// Obtém o estado dos modificadores no momento do evento
keymod_t keymod_get(keyrecord_t *record);

// Verifica se keymod contém APENAS o valor especificado (sem outros flags)
// value pode ser uma combinação de flags (ex: KEYMOD_FN_RCTL | KEYMOD_FN_RALT)
bool keymod_is_only_value(keymod_t keymod, keymod_t value);

// Verifica se keymod contém o valor especificado (pode ter outros flags também)
// value pode ser uma combinação de flags (ex: KEYMOD_FN_RCTL | KEYMOD_FN_RALT)
bool keymod_has_value(keymod_t keymod, keymod_t value);

// Verifica se keymod está aceito para position
bool keymod_is_accepted_for_position(keymod_t keymod, keymod_t accepted_keymods);

// Verifica se keymod está aceito para custom
bool keymod_is_accepted_for_custom(keymod_t keymod, keymod_t accepted_keymods);

#endif // KEYMOD_H

