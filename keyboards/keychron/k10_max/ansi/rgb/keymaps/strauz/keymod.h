#ifndef KEYMOD_H
#define KEYMOD_H

// ===== Estado dos Modificadores =====

// Enum de flags para representar estados de modificadores
typedef enum {
    KEYMOD_INVALID  = 0x00,  // Inválido (0 ou outros modificadores ativos)
    KEYMOD_NONE     = 0x01,  // Nenhum modificador ativo (bit 0)
    KEYMOD_FN       = 0x02,  // Apenas FN ativo (bit 1)
    KEYMOD_FN_RCTL  = 0x04,  // Apenas FN + RCTL ativos (bit 2)
    KEYMOD_FN_RALT  = 0x08,  // Apenas FN + RALT ativos (bit 3)
} keymod_t;

#endif // KEYMOD_H

