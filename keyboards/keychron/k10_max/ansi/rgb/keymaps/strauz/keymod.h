#ifndef KEYMOD_H
#define KEYMOD_H

#include QMK_KEYBOARD_H

// ===== Estado dos Modificadores =====

// Enum de flags para representar estados de modificadores
// Valores são potências de 2 para permitir operações bitwise
// Cada valor é um modo único (FN_ONLY não está contido em FN_RCTL, etc)
// Nenhum valor é 0 (KEYMOD_NONE = 1)
typedef enum {
    KEYMOD_NONE     = 0x01,  // Nenhum modificador ativo
    KEYMOD_FN_ONLY  = 0x02,  // Apenas FN ativo (sem outros modificadores)
    KEYMOD_FN_RCTL  = 0x04,  // FN + RCTL ativos (modo único, não contém FN_ONLY)
    KEYMOD_FN_RALT  = 0x08,  // FN + RALT ativos (modo único, não contém FN_ONLY)
    KEYMOD_FN_RSFT  = 0x10,  // FN + RSHIFT ativos (modo único, não contém FN_ONLY)
} keymod_t;

// Máscara com todas as flags (aceita qualquer keymod)
#define KEYMOD_ALL  (KEYMOD_NONE | KEYMOD_FN_ONLY | KEYMOD_FN_RCTL | KEYMOD_FN_RALT | KEYMOD_FN_RSFT)

// ===== Funções =====

// Obtém o estado dos modificadores no momento do evento
keymod_t keymod_get(keyrecord_t *record);

// Verifica se keymod é exatamente igual a value (igualdade exata)
// value pode ser uma combinação de flags (ex: KEYMOD_FN_RCTL | KEYMOD_FN_RALT)
bool keymod_equals(keymod_t keymod, keymod_t value);

// Verifica se keymod tem interseção com value (pelo menos uma flag em comum)
// value pode ser uma combinação de flags (ex: KEYMOD_FN_ONLY | KEYMOD_FN_RCTL)
// Retorna true se keymod possui pelo menos uma das flags ativas que value tem ativas
// Útil para verificar se um valor único (keymod) está contido em uma máscara (value)
bool keymod_intersects(keymod_t keymod, keymod_t value);

#endif // KEYMOD_H

