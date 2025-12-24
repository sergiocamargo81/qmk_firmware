#include "persist.h"

#define EEPROM_BEHAVIOR_START       0x00
#define EEPROM_CURRENT_PROFILE_ID   (EEPROM_BEHAVIOR_START + EEPROM_TOTAL_BEHAVIOR_SIZE)
#define EEPROM_DATA_VERSION_OFFSET  (EEPROM_CURRENT_PROFILE_ID + 1)

static void _pack_behaviors(const profile_data_t *profile, uint8_t *packed) {
    memset(packed, 0, EEPROM_PROFILE_SIZE);
    for (uint8_t i = 0; i < BEHAVIOR_KEYS_COUNT; i++) {
        uint8_t byte_index = i / 4;
        uint8_t bit_offset = (i % 4) * 2;
        uint8_t behavior_value = (uint8_t)profile->behaviors[i] & 0b11;
        packed[byte_index] |= (behavior_value << bit_offset);
    }
}

static void _unpack_behaviors(const uint8_t *packed, profile_data_t *profile) {
    for (uint8_t i = 0; i < BEHAVIOR_KEYS_COUNT; i++) {
        uint8_t byte_index = i / 4;
        uint8_t bit_offset = (i % 4) * 2;
        uint8_t behavior_value = (packed[byte_index] >> bit_offset) & 0b11;
        profile->behaviors[i] = (key_behavior_t)behavior_value;
    }
}

static inline uint16_t _profile_eeprom_offset(uint8_t profile_id) {
    if (profile_id >= BEHAVIOR_PROFILES_COUNT) return 0xFFFF;
    return EEPROM_BEHAVIOR_START + (profile_id * EEPROM_PROFILE_SIZE);
}

bool persist_read_profile(uint8_t profile_id, profile_data_t *profile) {
    if (profile_id >= BEHAVIOR_PROFILES_COUNT || !profile) return false;
    uint16_t offset = _profile_eeprom_offset(profile_id);
    uint8_t packed[EEPROM_PROFILE_SIZE] = {0};
    eeprom_read_block((void*)packed, (const void*)(uintptr_t)offset, EEPROM_PROFILE_SIZE);
    _unpack_behaviors(packed, profile);
    profile->id = profile_id;
    return true;
}

bool persist_write_profile(const profile_data_t *profile) {
    if (!profile || profile->id >= BEHAVIOR_PROFILES_COUNT) return false;
    uint16_t offset = _profile_eeprom_offset(profile->id);
    uint8_t packed[EEPROM_PROFILE_SIZE] = {0};
    _pack_behaviors(profile, packed);
    eeprom_update_block((const void*)packed, (void*)(uintptr_t)offset, EEPROM_PROFILE_SIZE);
    return true;
}

uint8_t persist_read_current_profile_id(void) {
    uint8_t profile_id = BEHAVIOR_DEFAULT_PROFILE;
    eeprom_read_block((void*)&profile_id, (const void*)(uintptr_t)EEPROM_CURRENT_PROFILE_ID, 1);
    if (profile_id >= BEHAVIOR_PROFILES_COUNT) return BEHAVIOR_DEFAULT_PROFILE;
    return profile_id;
}

bool persist_write_current_profile_id(uint8_t profile_id) {
    if (profile_id >= BEHAVIOR_PROFILES_COUNT) return false;
    eeprom_update_block((const void*)&profile_id, (void*)(uintptr_t)EEPROM_CURRENT_PROFILE_ID, 1);
    return true;
}

void persist_init_behaviors(void) {
    uint8_t stored_version = 0;
    eeprom_read_block((void*)&stored_version, (const void*)(uintptr_t)EEPROM_DATA_VERSION_OFFSET, 1);

    if (stored_version != EEPROM_DATA_VERSION) {
        uint8_t packed[EEPROM_PROFILE_SIZE] = {0};
        profile_data_t default_profile = {.id = BEHAVIOR_DEFAULT_PROFILE, .behaviors = {0}};

        for (uint8_t profile = 0; profile < BEHAVIOR_PROFILES_COUNT; profile++) {
            default_profile.id = profile;
            _pack_behaviors(&default_profile, packed);
            uint16_t offset = _profile_eeprom_offset(profile);
            eeprom_update_block((const void*)packed, (void*)(uintptr_t)offset, EEPROM_PROFILE_SIZE);
        }

        persist_write_current_profile_id(BEHAVIOR_DEFAULT_PROFILE);
        uint8_t version = EEPROM_DATA_VERSION;
        eeprom_update_block((const void*)&version, (void*)(uintptr_t)EEPROM_DATA_VERSION_OFFSET, 1);
    }
}
