#include "mlclick.h"

#include "../behavior.h"
#include "../event_bus.h"
#include "../keymod.h"
#include "../kind.h"
#include "../colors.h"
#include "../pulse.h"

mlclick_t* mlclick_position = NULL;

static void mlclick_update_states(void) {
    if (mlclick_position == NULL) return;

    uint32_t now = timer_read32();

    switch (mlclick_position->state) {
        case MLCLICK_PRESSING:
            if (timer_elapsed32(mlclick_position->timer) >= MLCLICK_PRESS_DELAY) {
                mlclick_position->state = MLCLICK_FIRING;
                mlclick_position->timer = now;
                tap_code(mlclick_position->keycode);
            }
            break;
        case MLCLICK_FIRING:
            if (timer_elapsed32(mlclick_position->timer) >= MLCLICK_INTERVAL) {
                mlclick_position->timer = now;
                tap_code(mlclick_position->keycode);
            }
            break;
        case MLCLICK_RESTING:
            if (timer_elapsed32(mlclick_position->timer) >= MLCLICK_RESTING_TIME) {
                mlclick_position->state = MLCLICK_WAITING;
            }
            break;
        case MLCLICK_TOGGLE_FIRING:
            if (timer_elapsed32(mlclick_position->timer) >= MLCLICK_INTERVAL) {
                tap_code(mlclick_position->keycode);
                mlclick_position->timer = now;
            }
            break;
        default:
            break;
    }
}

static void mlclick_matrix_scan_user(const event_t* event) {
    if (event->type != EVENT_MATRIX_SCAN) return;
    mlclick_update_states();
}

bool mlclick_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (key == NULL || key->kind != KIND_MLCLICK) {
        return true;
    }

    mlclick_t* mlclick = (mlclick_t*)key;

    if (pressed) {
        switch (mlclick->state) {
            case MLCLICK_WAITING:
            case MLCLICK_RESTING:
                if (keymod_equals(keymod, KEYMOD_FN_ONLY)) {
                    mlclick->state = MLCLICK_TOGGLE_FIRING;
                    tap_code(mlclick->keycode);
                    return true;
                }
                mlclick->state = MLCLICK_PRESSING;
                mlclick->timer = timer_read32();
                return true;
            case MLCLICK_TOGGLE_FIRING:
                mlclick->state = MLCLICK_WAITING;
                return true;
            default:
                return true;
        }
    }

    if (mlclick->state == MLCLICK_PRESSING) {
        mlclick->state = MLCLICK_WAITING;
        return true;
    }

    if (mlclick->state == MLCLICK_FIRING) {
        mlclick->state = MLCLICK_RESTING;
        mlclick->timer = timer_read32();
        return true;
    }

    return true;
}

static void mlclick_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    if (mlclick_position == NULL) return;

    switch (mlclick_position->state) {
        case MLCLICK_WAITING:
            uint8_t brightness = calculate_pulse_brightness();
            color_rgb_t blue = color_apply_brightness(COLOR_BLUE, brightness);
            rgb_matrix_set_color(mlclick_position->led_index, blue.r, blue.g, blue.b);
            break;
        case MLCLICK_PRESSING:
            color_rgb_t green = color_get_rgb(COLOR_GREEN);
            rgb_matrix_set_color(mlclick_position->led_index, green.r, green.g, green.b);
            break;
        case MLCLICK_FIRING:
        case MLCLICK_TOGGLE_FIRING:
            color_rgb_t red = color_get_rgb(COLOR_RED);
            rgb_matrix_set_color(mlclick_position->led_index, red.r, red.g, red.b);
            break;
        case MLCLICK_RESTING:
            color_rgb_t yellow = color_get_rgb(COLOR_YELLOW);
            rgb_matrix_set_color(mlclick_position->led_index, yellow.r, yellow.g, yellow.b);
            break;
        default: 
            break;
    }
}

static void mlclick_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;

    mlclick_position = kind_get_mlclick_key();
    if (mlclick_position == NULL) return;

    mlclick_position->state = MLCLICK_WAITING;
    mlclick_position->timer = timer_read32();
    mlclick_register_position();
}

void mlclick_register_position(void) {
    if (!mlclick_position) return;
    behavior_register_position_function(mlclick_position->row, mlclick_position->col, mlclick_process_key);
}

void mlclick_init_early_hooks(void) {
    event_bus_subscribe(EVENT_MATRIX_SCAN, mlclick_matrix_scan_user);
    event_bus_subscribe_rgb_indicators(mlclick_rgb_matrix_indicators_user);
}

void mlclick_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, mlclick_keyboard_post_init_user);
}
