#ifndef PERSIST_H
#define PERSIST_H

#include QMK_KEYBOARD_H

#include "settings.h"
#include "profiles.h"

// ===== Packing sizes =====
// 2 bits por tecla => bytes por perfil:
#define SETTINGS_PROFILE_PACKED_SIZE ((PROFILES_KEYS_COUNT * 2 + 7) / 8)

// Payload (sem magic, sem CRC):
// [version(1)] [profile_index(1)] [profiles_packed(PROFILES_COUNT * PROFILE_PACKED_SIZE)]
#define PERSIST_SETTINGS_SIZE (1 + 1 + (PROFILES_COUNT * SETTINGS_PROFILE_PACKED_SIZE))

// Datablock layout (128 bytes):
// [magic(2)] + payload(122) = 124 bytes
// [crc32(4)] => 128 bytes
#define PERSIST_MAGIC_0 0x53 // 'S'
#define PERSIST_MAGIC_1 0x5A // 'Z'

#define PERSIST_NOCRC_SIZE (2 + PERSIST_SETTINGS_SIZE) // 124
#define PERSIST_CRC_SIZE 4
#define PERSIST_BLOB_SIZE (PERSIST_NOCRC_SIZE + PERSIST_CRC_SIZE) // 128
#define PERSIST_CRC_OFFSET PERSIST_NOCRC_SIZE

#if !defined(EECONFIG_USER_DATA_SIZE) || (EECONFIG_USER_DATA_SIZE <= 0)
#    error "EECONFIG_USER_DATA_SIZE must be defined and > 0"
#endif

#if (EECONFIG_USER_DATA_SIZE < PERSIST_BLOB_SIZE)
#    error "EECONFIG_USER_DATA_SIZE is smaller than persist blob size (128)"
#endif

bool persist_read_settings(settings_t *settings);
bool persist_write_settings(const settings_t *settings);

#endif // PERSIST_H

