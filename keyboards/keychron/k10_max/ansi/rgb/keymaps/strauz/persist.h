#ifndef BEHAVIOR_PERSIST_H
#define BEHAVIOR_PERSIST_H

#include QMK_KEYBOARD_H
#include <eeprom.h>
#include "common.h"

#define EEPROM_PROFILE_SIZE 12
#define EEPROM_TOTAL_BEHAVIOR_SIZE (EEPROM_PROFILE_SIZE * BEHAVIOR_PROFILES_COUNT)
#define EEPROM_DATA_VERSION 1

bool persist_read_profile(uint8_t profile_id, profile_data_t *profile);
bool persist_write_profile(const profile_data_t *profile);
uint8_t persist_read_current_profile_id(void);
bool persist_write_current_profile_id(uint8_t profile_id);
void persist_init_behaviors(void);

#endif // BEHAVIOR_PERSIST_H
