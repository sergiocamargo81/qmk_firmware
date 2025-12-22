#include "state.h"
#include "eeprom.h"

// ===== AUTOFIRE KEYS ARRAY =====

autofire_key_t autofire_keys[] = {
	{KC_A, 3, 1, NO_LED, DISABLED, 0, 0},
	{KC_B, 4, 6, NO_LED, DISABLED, 0, 1},
	{KC_C, 4, 4, NO_LED, DISABLED, 0, 2},
	{KC_D, 3, 3, NO_LED, DISABLED, 0, 3},
	{KC_E, 2, 3, NO_LED, DISABLED, 0, 4},
	{KC_F, 3, 4, NO_LED, DISABLED, 0, 5},
	{KC_G, 3, 5, NO_LED, DISABLED, 0, 6},
	{KC_H, 3, 6, NO_LED, DISABLED, 0, 7},
	{KC_I, 2, 8, NO_LED, DISABLED, 0, 8},
	{KC_J, 3, 7, NO_LED, DISABLED, 0, 9},
	{KC_K, 3, 8, NO_LED, DISABLED, 0, 10},
	{KC_L, 3, 9, NO_LED, DISABLED, 0, 11},
	{KC_M, 4, 8, NO_LED, DISABLED, 0, 12},
	{KC_N, 4, 7, NO_LED, DISABLED, 0, 13},
	{KC_O, 2, 9, NO_LED, DISABLED, 0, 14},
	{KC_P, 2, 10, NO_LED, DISABLED, 0, 15},
	{KC_Q, 2, 1, NO_LED, DISABLED, 0, 16},
	{KC_R, 2, 4, NO_LED, DISABLED, 0, 17},
	{KC_S, 3, 2, NO_LED, DISABLED, 0, 18},
	{KC_T, 2, 5, NO_LED, DISABLED, 0, 19},
	{KC_U, 2, 7, NO_LED, DISABLED, 0, 20},
	{KC_V, 4, 5, NO_LED, DISABLED, 0, 21},
	{KC_W, 2, 2, NO_LED, DISABLED, 0, 22},
	{KC_X, 4, 3, NO_LED, DISABLED, 0, 23},
	{KC_Y, 2, 6, NO_LED, DISABLED, 0, 24},
	{KC_Z, 4, 2, NO_LED, DISABLED, 0, 25},
	{KC_1, 1, 1, NO_LED, DISABLED, 0, 26},
	{KC_2, 1, 2, NO_LED, DISABLED, 0, 27},
	{KC_3, 1, 3, NO_LED, DISABLED, 0, 28},
	{KC_4, 1, 4, NO_LED, DISABLED, 0, 29},
	{KC_5, 1, 5, NO_LED, DISABLED, 0, 30},
	{KC_6, 1, 6, NO_LED, DISABLED, 0, 31},
	{KC_7, 1, 7, NO_LED, DISABLED, 0, 32},
	{KC_8, 1, 8, NO_LED, DISABLED, 0, 33},
	{KC_9, 1, 9, NO_LED, DISABLED, 0, 34},
	{KC_0, 1, 10, NO_LED, DISABLED, 0, 35},
	{KC_SPC, 5, 6, NO_LED, DISABLED, 0, 36},
	{KC_MINUS, 1, 11, NO_LED, DISABLED, 0, 37},
	{KC_EQL, 1, 12, NO_LED, DISABLED, 0, 38},
	{KC_LBRC, 2, 11, NO_LED, DISABLED, 0, 39},
	{KC_RBRC, 2, 12, NO_LED, DISABLED, 0, 40},
	{KC_BSLS, 2, 13, NO_LED, DISABLED, 0, 41},
	{KC_SCLN, 3, 10, NO_LED, DISABLED, 0, 42},
	{KC_QUOT, 3, 11, NO_LED, DISABLED, 0, 43},
	{KC_GRV, 1, 0, NO_LED, DISABLED, 0, 44},
	{KC_COMM, 4, 9, NO_LED, DISABLED, 0, 45},
	{KC_DOT, 4, 10, NO_LED, DISABLED, 0, 46},
	{KC_SLSH, 4, 11, NO_LED, DISABLED, 0, 47},
};

bool autofire_profile_mode = true;

// ===== BINARY SEARCH =====

bool try_find_autofire_key(uint16_t keycode, autofire_key_t **out_key) {
	int32_t left = 0;
	int32_t right = AUTOFIRE_KEYS_COUNT - 1;

	while (left <= right) {
		int32_t mid = left + (right - left) / 2;
		uint16_t mid_keycode = autofire_keys[mid].keycode;

		if (mid_keycode == keycode) {
			if (autofire_keys[mid].state != DISABLED) {
				*out_key = &autofire_keys[mid];
				return true;
			}
			*out_key = NULL;
			return false;
		}

		if (mid_keycode < keycode) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}

	*out_key = NULL;
	return false;
}

autofire_key_t *find_key_by_keycode(uint16_t keycode) {
	int32_t left = 0;
	int32_t right = AUTOFIRE_KEYS_COUNT - 1;

	while (left <= right) {
		int32_t mid = left + (right - left) / 2;
		uint16_t mid_keycode = autofire_keys[mid].keycode;

		if (mid_keycode == keycode) {
			return &autofire_keys[mid];
		}

		if (mid_keycode < keycode) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}

	return NULL;
}

// ===== LED INITIALIZATION =====

void init_led_indices(void) {
	for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
		autofire_keys[i].led_index = g_led_config.matrix_co[autofire_keys[i].row][autofire_keys[i].col];
	}
}

// ===== STATE MACHINE UPDATE =====

void update_autofire_states(void) {
	uint32_t now = timer_read32();

	for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
		autofire_key_t *key = &autofire_keys[i];

		if (key->state == DISABLED) {
			continue;
		}

		switch (key->state) {
		case PRESSING: {
			uint32_t elapsed = timer_elapsed32(key->timer);
			if (elapsed >= AUTOFIRE_DELAY) {
				key->state = FIRING;
				key->timer = now;
				tap_code(key->keycode);
			}
			break;
		}

		case FIRING: {
			uint32_t repeat_elapsed = timer_elapsed32(key->timer);
			if (repeat_elapsed >= AUTOFIRE_INTERVAL) {
				tap_code(key->keycode);
				key->timer = now;
			}
			break;
		}

		case RESTING: {
			uint32_t rest_elapsed = timer_elapsed32(key->timer);
			if (rest_elapsed >= LED_DEACTIVATE_TIME) {
				key->state = WAITING;
			}
			break;
		}

		default:
			break;
		}
	}
}

// ===== ACTIVATION/DEACTIVATION =====

void activate_autofire_key(autofire_key_t *key) {
	uint32_t now = timer_read32();
	key->state = PRESSING;
	key->timer = now;
}

void deactivate_autofire_key(autofire_key_t *key) {
	uint32_t now = timer_read32();

	if (key->state == FIRING) {
		key->state = RESTING;
		key->timer = now;
		return;
	}

	if (key->state == PRESSING) {
		key->state = WAITING;
		return;
	}
}

void deactivate_all_autofire(void) {
	uint32_t now = timer_read32();

	for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
		if (autofire_keys[i].state == FIRING) {
			autofire_keys[i].state = RESTING;
			autofire_keys[i].timer = now;
		}
	}
}

// ===== SYNC STATE =====

void sync_enabled_states(void) {
	for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
		sync_enabled_key_to_state(&autofire_keys[i]);
	}
}

// ===== RGB UPDATE =====

void update_indicators(void) {
	rgb_matrix_set_color_all(RGB_OFF);

	for (uint8_t i = 0; i < AUTOFIRE_KEYS_COUNT; i++) {
		autofire_key_t *key = &autofire_keys[i];

		if (key->led_index != NO_LED) {
			switch (key->state) {
			case WAITING:
				rgb_matrix_set_color(key->led_index, 128, 128, 128);
				break;
			case PRESSING:
				rgb_matrix_set_color(key->led_index, 0, 255, 0);
				break;
			case FIRING:
				rgb_matrix_set_color(key->led_index, 255, 0, 0);
				break;
			case RESTING:
				rgb_matrix_set_color(key->led_index, 255, 255, 0);
				break;
			case DISABLED:
				rgb_matrix_set_color(key->led_index, RGB_OFF);
				break;
			}
		}
	}
}

// ===== PROFILE MODE =====

void toggle_profile_mode(void) {
	autofire_profile_mode = !autofire_profile_mode;
}

bool is_profile_mode_active(void) {
	return autofire_profile_mode;
}
