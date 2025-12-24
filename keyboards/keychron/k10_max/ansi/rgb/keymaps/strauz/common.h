#ifndef COMMON_H
#define COMMON_H

#include QMK_KEYBOARD_H

typedef enum {
    KEY_BEHAVIOR_DISABLED = 0b00,
    KEY_BEHAVIOR_HOLD = 0b01,
    KEY_BEHAVIOR_TOGGLE = 0b10,
} key_behavior_t;

#define BEHAVIOR_KEYS_COUNT 48
#define BEHAVIOR_PROFILES_COUNT 10
#define BEHAVIOR_DEFAULT_PROFILE 0

typedef struct {
    uint8_t id;
    key_behavior_t behaviors[BEHAVIOR_KEYS_COUNT];
} profile_data_t;

typedef struct {
    profile_data_t data;
    bool changed;
} profile_t;

#endif // COMMON_H
