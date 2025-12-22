#ifndef HOLD_EEPROM_H
#define HOLD_EEPROM_H

#include QMK_KEYBOARD_H
#include "state.h"

// ===== EEPROM CONFIGURATION =====

#define AUTOFIRE_BYTES_FOR_STORAGE 6
#define EECONFIG_AUTOFIRE_ENABLED (EECONFIG_USER)
#define EEPROM_INIT_FLAG_ADDR (EECONFIG_AUTOFIRE_ENABLED + AUTOFIRE_BYTES_FOR_STORAGE)

// ===== PRIVATE FUNCTIONS (used only in keymap.c) =====

void load_enabled_keys(void);
void save_enabled_keys(void);
void toggle_key_enabled(autofire_key_t *key);
void reset_all_enabled_keys(void);
void sync_enabled_key_to_state(autofire_key_t *key);

#endif // HOLD_EEPROM_H
