#ifndef HOLD_STATE_H
#define HOLD_STATE_H

#include QMK_KEYBOARD_H

// ===== AUTOFIRE CONFIGURATION =====

#define AUTOFIRE_DELAY        500
#define AUTOFIRE_INTERVAL     50
#define LED_DEACTIVATE_TIME   5000
#define AUTOFIRE_KEYS_COUNT   48

// ===== STATE MACHINE =====

typedef enum {
	DISABLED,
	WAITING,
	PRESSING,
	FIRING,
	RESTING
} autofire_state_t;

typedef struct {
	uint16_t keycode;
	uint8_t row;
	uint8_t col;
	uint8_t led_index;
	autofire_state_t state;
	uint32_t timer;
	uint8_t persist_index;
} autofire_key_t;

// ===== PRIVATE FUNCTIONS (used only in keymap.c) =====

bool try_find_autofire_key(uint16_t keycode, autofire_key_t **out_key);
autofire_key_t *find_key_by_keycode(uint16_t keycode);
void init_led_indices(void);
void update_autofire_states(void);
void activate_autofire_key(autofire_key_t *key);
void deactivate_autofire_key(autofire_key_t *key);
void deactivate_all_autofire(void);
void sync_enabled_states(void);
void update_indicators(void);
void toggle_profile_mode(void);
bool is_profile_mode_active(void);

// ===== DATA =====

extern autofire_key_t autofire_keys[];
extern bool autofire_profile_mode;

#endif // HOLD_STATE_H
