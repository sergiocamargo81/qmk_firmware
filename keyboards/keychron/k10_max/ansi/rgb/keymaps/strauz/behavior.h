#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include "common.h"

bool behavior_init(void);
void behavior_init_defaults(void);
key_behavior_t behavior_get(uint16_t keycode);
bool behavior_is_active(uint16_t keycode);
uint8_t behavior_get_current_profile_id(void);
bool behavior_has_changed(void);
bool behavior_set(uint16_t keycode, key_behavior_t behavior);
void behavior_reset_all(void);
bool behavior_save_to_eeprom(void);
bool behavior_load_profile(uint8_t profile_id);
bool behavior_save_and_load_profile(uint8_t profile_id);
bool process_record_behavior(uint16_t keycode, keyrecord_t *record);
uint8_t behavior_keycode_to_index(uint16_t keycode);

extern profile_t current_profile;

#endif // BEHAVIOR_H
