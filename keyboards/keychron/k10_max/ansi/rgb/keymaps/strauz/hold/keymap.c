#include "keymap.h"
#include "state.h"
#include "../behavior.h"

bool process_record_hold(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        uint8_t current_layer = get_highest_layer(layer_state);
        uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();
        if (current_layer == WIN_FN && (mods & MOD_BIT(KC_RCTL))) {
            uint16_t base_keycode = keymap_key_to_keycode(WIN_BASE, record->event.key);
            autofire_key_t *key = find_key_by_keycode(base_keycode);
            if (key) { toggle_key_enabled(key); return false; }
        }
    }

    if (keycode == SZ_EECLR && record->event.pressed) {
        eeconfig_init();
        eeconfig_init_hold();
        return false;
    }

    if (keycode == SZ_RESET && record->event.pressed) {
        reset_all_enabled_keys();
        return true;
    }

    if (keycode == KC_SPC) {
        if (record->event.pressed) {
            deactivate_all_autofire();
            autofire_key_t *space_key = find_key_by_keycode(keycode);
            if (space_key && space_key->state != DISABLED) activate_autofire_key(space_key);
        } else {
            autofire_key_t *space_key = find_key_by_keycode(keycode);
            if (space_key) deactivate_autofire_key(space_key);
        }
        return true;
    }

    autofire_key_t *key = NULL;
    if (try_find_autofire_key(keycode, &key)) {
        if (record->event.pressed) activate_autofire_key(key);
        else deactivate_autofire_key(key);
        return true;
    }
    return true;
}

void matrix_scan_hold(void) { update_autofire_states(); }
bool rgb_matrix_indicators_hold(void) {
    if (!is_profile_mode_active()) return true;
    update_indicators();
    return false;
}
void keyboard_post_init_hold(void) { init_led_indices(); sync_enabled_states(); }
void eeconfig_init_hold(void) { reset_all_enabled_keys(); }
