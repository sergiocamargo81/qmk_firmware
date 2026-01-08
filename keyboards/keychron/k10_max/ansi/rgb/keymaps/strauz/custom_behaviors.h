#ifndef CUSTOM_BEHAVIORS_H
#define CUSTOM_BEHAVIORS_H

// ===== Custom Behaviors (gravados na EEPROM) =====
typedef enum {
    CUSTOM_BEHAVIOR_UNASSOCIATED = 0b00,
    CUSTOM_BEHAVIOR_HOLD         = 0b01,
    CUSTOM_BEHAVIOR_TOGGLE       = 0b10,
    CUSTOM_BEHAVIOR_RESERVED     = 0b11, // Reservado para futuro custom
} custom_behaviors_t;

// Máscara para extrair apenas custom behavior (0b00-0b11)
#define CUSTOM_BEHAVIOR_MASK 0b11

// Número de valores possíveis em custom_behaviors_t (4: UNASSOCIATED, HOLD, TOGGLE, RESERVED)
#define CUSTOM_BEHAVIORS_T_COUNT 4

#endif // CUSTOM_BEHAVIORS_H
