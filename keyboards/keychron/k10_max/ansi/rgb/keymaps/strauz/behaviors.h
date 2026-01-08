#ifndef BEHAVIORS_H
#define BEHAVIORS_H

#include "custom_behaviors.h"  // Para CUSTOM_BEHAVIOR_MASK usado em behaviors_t

// ===== Behaviors Especiais (não gravados na EEPROM, determinados por posição/keycode) =====
typedef enum {
    KEY_BEHAVIOR_CUSTOM      = 0b000, // Indica que é um custom (usa custom_behaviors_t)
    KEY_BEHAVIOR_NUMLOCK     = 0b001,
    KEY_BEHAVIOR_PERSISTENCE = 0b010,
    KEY_BEHAVIOR_PROFILES    = 0b011,
} behaviors_t;

#endif // BEHAVIORS_H

