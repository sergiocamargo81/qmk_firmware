#include "eeprom.h"

// ===== EEPROM STORAGE (PRIVATE) =====

static uint8_t enabled_keys_bitfield[AUTOFIRE_BYTES_FOR_STORAGE] = {0};

// ===== BIT MANIPULATION (PRIVATE) =====

static bool is_key_enabled(autofire_key_t *key) {
	if (!key) return false;
	uint8_t byte_index = key->persist_index / 8;
	uint8_t bit_index = key->persist_index % 8;
	return (enabled_keys_bitfield[byte_index] & (1 << bit_index)) != 0;
}

static void set_key_enabled(autofire_key_t *key, bool enabled) {
	if (!key) return;
	uint8_t byte_index = key->persist_index / 8;
	uint8_t bit_index = key->persist_index % 8;
	if (enabled) {
		enabled_keys_bitfield[byte_index] |= (1 << bit_index);
	} else {
		enabled_keys_bitfield[byte_index] &= ~(1 << bit_index);
	}
}

// ===== EEPROM OPERATIONS =====

void load_enabled_keys(void) {
	eeprom_read_block(enabled_keys_bitfield,
		(void*)EECONFIG_AUTOFIRE_ENABLED,
		sizeof(enabled_keys_bitfield));

	uint8_t init_flag = eeprom_read_byte((void*)EEPROM_INIT_FLAG_ADDR);

	if (init_flag != 0xAA) {
		// First boot
		memset(enabled_keys_bitfield, 0, sizeof(enabled_keys_bitfield));
		save_enabled_keys();
		eeprom_update_byte((void*)EEPROM_INIT_FLAG_ADDR, 0xAA);
	}
}

void save_enabled_keys(void) {
	eeprom_write_block(enabled_keys_bitfield,
		(void*)EECONFIG_AUTOFIRE_ENABLED,
		sizeof(enabled_keys_bitfield));
}

void sync_enabled_key_to_state(autofire_key_t *key) {
	if (!key) return;
	bool is_enabled = is_key_enabled(key);
	if (!is_enabled) {
		key->state = DISABLED;
	} else if (key->state == DISABLED) {
		key->state = WAITING;
	}
}

void toggle_key_enabled(autofire_key_t *key) {
	if (!key) return;
	bool currently_enabled = is_key_enabled(key);
	set_key_enabled(key, !currently_enabled);
	save_enabled_keys();
	sync_enabled_key_to_state(key);
}

void reset_all_enabled_keys(void) {
	memset(enabled_keys_bitfield, 0, sizeof(enabled_keys_bitfield));
	save_enabled_keys();
	eeprom_update_byte((void*)EEPROM_INIT_FLAG_ADDR, 0xAA);

	for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
		autofire_keys[i].state = DISABLED;
	}
}
