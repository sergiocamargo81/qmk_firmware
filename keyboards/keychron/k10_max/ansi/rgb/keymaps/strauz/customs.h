#ifndef CUSTOMS_H
#define CUSTOMS_H

// ===== Custom (gravados na EEPROM) =====
typedef enum {
    KEY_CUSTOM_UNASSOCIATED = 0b00,
    KEY_CUSTOM_HOLD         = 0b01,
    KEY_CUSTOM_TOGGLE       = 0b10,
    KEY_CUSTOM_RESERVED     = 0b11, // Reservado para futuro custom
} customs_t;

// Máscara para extrair apenas custom (0b00-0b11)
#define KEY_CUSTOM_MASK 0b11

// Número de valores possíveis em customs_t (4: UNASSOCIATED, HOLD, TOGGLE, RESERVED)
#define CUSTOMS_T_COUNT 4

#endif // CUSTOMS_H

