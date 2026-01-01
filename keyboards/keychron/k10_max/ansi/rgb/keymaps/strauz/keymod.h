#ifndef KEYMOD_H
#define KEYMOD_H

// ===== Estado dos Modificadores =====

// Enum de flags para representar estados de modificadores
typedef enum {
    KEYMOD_NONE,     // Nenhum modificador ativo
    KEYMOD_FN,       // Apenas FN ativo
    KEYMOD_FN_RCTL,  // Apenas FN + RCTL ativos
    KEYMOD_FN_RALT,  // Apenas FN + RALT ativos
    KEYMOD_FN_RSFT,  // Apenas FN + RSHIFT ativos
} keymod_t;

#endif // KEYMOD_H

