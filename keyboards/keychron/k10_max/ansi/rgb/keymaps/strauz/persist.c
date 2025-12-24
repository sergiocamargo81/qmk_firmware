#include "persist.h"

#include "eeconfig.h"
#include "customs.h"  // Para customs_t e KEY_CUSTOM_MASK

#include <string.h>

static uint32_t _crc32_update(uint32_t crc, uint8_t data) {
    crc ^= data;
    for (uint8_t i = 0; i < 8; i++) {
        if (crc & 1) {
            crc = (crc >> 1) ^ 0xEDB88320u;
        } else {
            crc = (crc >> 1);
        }
    }
    return crc;
}

// CRC-32 (Ethernet/ZIP): init=0xFFFFFFFF, poly=0xEDB88320 (refletido), xorout=0xFFFFFFFF
static uint32_t persist_crc32(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFFu;

    for (size_t i = 0; i < len; i++) {
        crc = _crc32_update(crc, p[i]);
    }

    return crc ^ 0xFFFFFFFFu;
}

static uint32_t _read_u32_le(const uint8_t *p) {
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void _write_u32_le(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

// Pack: settings_t -> datablock[0..123] (sem CRC)
static void _settings_pack_nocrc(const settings_t *settings, uint8_t *out) {
    uint8_t *ptr = out;

    // Magic
    *ptr++ = PERSIST_MAGIC_0;
    *ptr++ = PERSIST_MAGIC_1;

    // Version
    *ptr++ = settings->version;

    // Profile index
    *ptr++ = settings->profiles.active_index;

    // Compacta perfis
    for (uint8_t p = 0; p < PROFILES_COUNT; p++) {
        uint8_t profile_packed[SETTINGS_PROFILE_PACKED_SIZE];
        memset(profile_packed, 0, sizeof(profile_packed));

        for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
            uint8_t byte_index = i / 4;
            uint8_t bit_offset = (i % 4) * 2;
            // Grava apenas custom (HOLD, TOGGLE) - mascarando outros tipos
            uint8_t behavior_value = (uint8_t)settings->profiles.profiles[p].behaviors[i] & KEY_CUSTOM_MASK;
            profile_packed[byte_index] |= (uint8_t)(behavior_value << bit_offset);
        }

        memcpy(ptr, profile_packed, sizeof(profile_packed));
        ptr += sizeof(profile_packed);
    }
}

static void _settings_unpack(const uint8_t *blob, settings_t *settings) {
    const uint8_t *ptr = blob;

    // Pula magic
    ptr += 2;

    settings->version = *ptr++;
    uint8_t profile_index = *ptr++;

    // Inicializa profiles
    for (uint8_t p = 0; p < PROFILES_COUNT; p++) {
        settings->profiles.profiles[p].active = (p == profile_index);
        settings->profiles.profiles[p].index = p;
        
        for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
            uint8_t byte_index = i / 4;
            uint8_t bit_offset = (i % 4) * 2;
            // Lê apenas custom (HOLD, TOGGLE)
            uint8_t behavior_value = (ptr[byte_index] >> bit_offset) & KEY_CUSTOM_MASK;
            settings->profiles.profiles[p].behaviors[i] = (customs_t)behavior_value;
        }
        ptr += SETTINGS_PROFILE_PACKED_SIZE;
    }
    
    // Define profile ativo
    settings->profiles.active_index = profile_index;
}

bool persist_read_settings(settings_t *settings) {
    if (!settings) return false;

#if !defined(EECONFIG_USER_DATA_SIZE) || (EECONFIG_USER_DATA_SIZE <= 0)
    return false;
#else
    uint8_t datablock[EECONFIG_USER_DATA_SIZE];
    memset(datablock, 0, sizeof(datablock));

    // Keychron fork: lê o datablock inteiro
    eeconfig_read_user_datablock(datablock);

    // Magic
    if (datablock[0] != PERSIST_MAGIC_0 || datablock[1] != PERSIST_MAGIC_1) return false;

    // Version do payload
    if (datablock[2] != SETTINGS_VERSION) return false;

    // CRC
    uint32_t stored_crc = _read_u32_le(&datablock[PERSIST_CRC_OFFSET]);
    uint32_t calc_crc   = persist_crc32(datablock, PERSIST_NOCRC_SIZE);
    if (stored_crc != calc_crc) return false;

    _settings_unpack(datablock, settings);

    if (settings->profiles.active_index >= PROFILES_COUNT) return false;

    return true;
#endif
}

bool persist_write_settings(const settings_t *settings) {
    if (!settings) return false;

#if !defined(EECONFIG_USER_DATA_SIZE) || (EECONFIG_USER_DATA_SIZE <= 0)
    return false;
#else
    uint8_t datablock[EECONFIG_USER_DATA_SIZE];
    memset(datablock, 0, sizeof(datablock));

    _settings_pack_nocrc(settings, datablock);

    uint32_t crc = persist_crc32(datablock, PERSIST_NOCRC_SIZE);
    _write_u32_le(&datablock[PERSIST_CRC_OFFSET], crc);

    // Keychron fork: grava o datablock inteiro
    eeconfig_update_user_datablock(datablock);

    return true;
#endif
}

