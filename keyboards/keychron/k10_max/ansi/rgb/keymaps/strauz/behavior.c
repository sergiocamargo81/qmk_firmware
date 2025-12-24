#include "behavior.h"
#include "keymap.h"
#include "persist.h"

static const uint16_t behavior_keycodes[BEHAVIOR_KEYS_COUNT] = {
    KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, KC_H,
    KC_I, KC_J, KC_K, KC_L, KC_M, KC_N, KC_O, KC_P,
    KC_Q, KC_R, KC_S, KC_T, KC_U, KC_V, KC_W, KC_X,
    KC_Y, KC_Z, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6,
    KC_7, KC_8, KC_9, KC_0, KC_SPC, KC_MINS, KC_EQL, KC_LBRC,
    KC_RBRC, KC_BSLS, KC_SCLN, KC_QUOT, KC_GRV, KC_COMM, KC_DOT, KC_SLSH,
};

profile_t current_profile = {
    .data = {.id = BEHAVIOR_DEFAULT_PROFILE, .behaviors = {0}},
    .changed = false
};

uint8_t behavior_keycode_to_index(uint16_t keycode) {
    int left = 0, right = BEHAVIOR_KEYS_COUNT - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        uint16_t mid_keycode = behavior_keycodes[mid];
        if (mid_keycode == keycode) return mid;
        else if (mid_keycode < keycode) left = mid + 1;
        else right = mid - 1;
    }
    return 0xFF;
}

key_behavior_t behavior_get(uint16_t keycode) {
    uint8_t index = behavior_keycode_to_index(keycode);
    if (index == 0xFF) return KEY_BEHAVIOR_DISABLED;
    return current_profile.data.behaviors[index];
}

bool behavior_set(uint16_t keycode, key_behavior_t behavior) {
    uint8_t index = behavior_keycode_to_index(keycode);
    if (index == 0xFF) return false;
    key_behavior_t current = current_profile.data.behaviors[index];
    if (current != behavior) {
        current_profile.data.behaviors[index] = behavior;
        current_profile.changed = true;
    }
    return true;
}

bool behavior_save_and_load_profile(uint8_t profile_id) {
    if (profile_id >= BEHAVIOR_PROFILES_COUNT) return false;
    if (current_profile.changed) {
        persist_write_profile(&current_profile.data);
        persist_write_current_profile_id(current_profile.data.id);
    }
    bool success = persist_read_profile(profile_id, &current_profile.data);
    if (success) {
        current_profile.data.id = profile_id;
        current_profile.changed = false;
    }
    return success;
}

bool behavior_save_to_eeprom(void) {
    if (!current_profile.changed) return true;
    bool success = persist_write_profile(&current_profile.data);
    if (success) {
        success = persist_write_current_profile_id(current_profile.data.id);
        current_profile.changed = false;
    }
    return success;
}

void behavior_init_defaults(void) {
    memset(current_profile.data.behaviors, 0, sizeof(current_profile.data.behaviors));
    current_profile.data.id = BEHAVIOR_DEFAULT_PROFILE;
    current_profile.changed = true;
}

bool behavior_init(void) {
    persist_init_behaviors();
    uint8_t profile_id = persist_read_current_profile_id();
    return persist_read_profile(profile_id, &current_profile.data);
}

uint8_t behavior_get_current_profile_id(void) { return current_profile.data.id; }
bool behavior_has_changed(void) { return current_profile.changed; }
bool behavior_is_active(uint16_t keycode) { return behavior_get(keycode) != KEY_BEHAVIOR_DISABLED; }
void behavior_reset_all(void) {
    memset(current_profile.data.behaviors, 0, sizeof(current_profile.data.behaviors));
    current_profile.changed = true;
}

bool process_record_behavior(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        if (keycode >= PROFILE_0 && keycode <= PROFILE_9) {
            uint8_t new_profile_id = keycode - PROFILE_0;
            behavior_save_and_load_profile(new_profile_id);
            return false;
        }
    }
    return true;
}
