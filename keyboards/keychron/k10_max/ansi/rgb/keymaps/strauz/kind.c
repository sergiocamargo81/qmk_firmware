#include "kind.h"
#include "keymap.h"  // Para WIN_BASE

// ===== Pools Estáticos =====

// Pool de profiles: 10 teclas (P0-P9)
// Todos os valores preenchidos estaticamente, exceto function (preenchido pelos módulos)
// Ordenado por row, depois por col
static profile_key_t g_profile_pool[PROFILE_COUNT] = {
    // Row 2
    {.base = {KIND_PROFILE}, .row = 2, .col = 17, .keycode = KC_P7,  .led_index = 56, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P7
    {.base = {KIND_PROFILE}, .row = 2, .col = 18, .keycode = KC_P8,  .led_index = 57, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P8
    {.base = {KIND_PROFILE}, .row = 2, .col = 19, .keycode = KC_P9,  .led_index = 58, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P9
    // Row 3
    {.base = {KIND_PROFILE}, .row = 3, .col = 17, .keycode = KC_P4,  .led_index = 76, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P4
    {.base = {KIND_PROFILE}, .row = 3, .col = 18, .keycode = KC_P5,  .led_index = 77, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P5
    {.base = {KIND_PROFILE}, .row = 3, .col = 19, .keycode = KC_P6,  .led_index = 78, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P6
    // Row 4
    {.base = {KIND_PROFILE}, .row = 4, .col = 17, .keycode = KC_P1,  .led_index = 92, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P1
    {.base = {KIND_PROFILE}, .row = 4, .col = 18, .keycode = KC_P2,  .led_index = 93, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P2
    {.base = {KIND_PROFILE}, .row = 4, .col = 19, .keycode = KC_P3,  .led_index = 94, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P3
    // Row 5
    {.base = {KIND_PROFILE}, .row = 5, .col = 18, .keycode = KC_P0,  .led_index = 106, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // P0
};

// Pool de numlock: 1 tecla (KC_NUM)
static numlock_t g_numlock_pool[NUMLOCK_COUNT] = {
    {.base = {KIND_NUMLOCK}, .row = 1, .col = 17, .keycode = KC_NUM, .led_index = 36, .accepted_keymods = KEYMOD_NONE, .function = NULL, .state = 0},  // KC_NUM
};

// Pool de persistence: 1 tecla (KC_END)
static persistence_key_t g_persistence_pool[PERSISTENCE_COUNT] = {
    {.base = {KIND_PERSISTENCE}, .row = 2, .col = 15, .keycode = KC_END, .led_index = 54, .accepted_keymods = KEYMOD_FN_ONLY, .function = NULL, .state = 0},  // KC_END
};

// Pool de customs: 48 teclas (persist_index 0-47)
// Todos os valores preenchidos estaticamente, exceto function (preenchido pelos módulos)
// Ordenado por row, depois por col (mantém persist_index original)
// accepted_keymods para custom: aceita NONE, FN_RCTL ou FN_RALT
#define KEYMOD_CUSTOM_MASK  (KEYMOD_NONE | KEYMOD_FN_RCTL | KEYMOD_FN_RALT)
static custom_t g_custom_pool[CUSTOM_COUNT] = {
    // Row 1
    {.base = {KIND_CUSTOM}, .row = 1, .col = 0,  .keycode = KC_GRV,  .led_index = 19, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 44, .function = NULL, .state = 0, .timer = 0},  // 44
    {.base = {KIND_CUSTOM}, .row = 1, .col = 1,  .keycode = KC_1,    .led_index = 20, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 26, .function = NULL, .state = 0, .timer = 0},  // 26
    {.base = {KIND_CUSTOM}, .row = 1, .col = 2,  .keycode = KC_2,    .led_index = 21, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 27, .function = NULL, .state = 0, .timer = 0},  // 27
    {.base = {KIND_CUSTOM}, .row = 1, .col = 3,  .keycode = KC_3,    .led_index = 22, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 28, .function = NULL, .state = 0, .timer = 0},  // 28
    {.base = {KIND_CUSTOM}, .row = 1, .col = 4,  .keycode = KC_4,    .led_index = 23, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 29, .function = NULL, .state = 0, .timer = 0},  // 29
    {.base = {KIND_CUSTOM}, .row = 1, .col = 5,  .keycode = KC_5,    .led_index = 24, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 30, .function = NULL, .state = 0, .timer = 0},  // 30
    {.base = {KIND_CUSTOM}, .row = 1, .col = 6,  .keycode = KC_6,    .led_index = 25, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 31, .function = NULL, .state = 0, .timer = 0},  // 31
    {.base = {KIND_CUSTOM}, .row = 1, .col = 7,  .keycode = KC_7,    .led_index = 26, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 32, .function = NULL, .state = 0, .timer = 0},  // 32
    {.base = {KIND_CUSTOM}, .row = 1, .col = 8,  .keycode = KC_8,    .led_index = 27, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 33, .function = NULL, .state = 0, .timer = 0},  // 33
    {.base = {KIND_CUSTOM}, .row = 1, .col = 9,  .keycode = KC_9,    .led_index = 28, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 34, .function = NULL, .state = 0, .timer = 0},  // 34
    {.base = {KIND_CUSTOM}, .row = 1, .col = 10, .keycode = KC_0,    .led_index = 29, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 35, .function = NULL, .state = 0, .timer = 0},  // 35
    {.base = {KIND_CUSTOM}, .row = 1, .col = 11, .keycode = KC_MINS, .led_index = 30, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 37, .function = NULL, .state = 0, .timer = 0},  // 37
    {.base = {KIND_CUSTOM}, .row = 1, .col = 12, .keycode = KC_EQL,  .led_index = 31, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 38, .function = NULL, .state = 0, .timer = 0},  // 38
    // Row 2
    {.base = {KIND_CUSTOM}, .row = 2, .col = 1,  .keycode = KC_Q,    .led_index = 40, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 16, .function = NULL, .state = 0, .timer = 0},  // 16
    {.base = {KIND_CUSTOM}, .row = 2, .col = 2,  .keycode = KC_W,    .led_index = 41, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 22, .function = NULL, .state = 0, .timer = 0},  // 22
    {.base = {KIND_CUSTOM}, .row = 2, .col = 3,  .keycode = KC_E,    .led_index = 42, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 4,  .function = NULL, .state = 0, .timer = 0},   // 4
    {.base = {KIND_CUSTOM}, .row = 2, .col = 4,  .keycode = KC_R,    .led_index = 43, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 17, .function = NULL, .state = 0, .timer = 0},  // 17
    {.base = {KIND_CUSTOM}, .row = 2, .col = 5,  .keycode = KC_T,    .led_index = 44, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 19, .function = NULL, .state = 0, .timer = 0},  // 19
    {.base = {KIND_CUSTOM}, .row = 2, .col = 6,  .keycode = KC_Y,    .led_index = 45, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 24, .function = NULL, .state = 0, .timer = 0},  // 24
    {.base = {KIND_CUSTOM}, .row = 2, .col = 7,  .keycode = KC_U,    .led_index = 46, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 20, .function = NULL, .state = 0, .timer = 0},  // 20
    {.base = {KIND_CUSTOM}, .row = 2, .col = 8,  .keycode = KC_I,    .led_index = 47, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 8,  .function = NULL, .state = 0, .timer = 0},   // 8
    {.base = {KIND_CUSTOM}, .row = 2, .col = 9,  .keycode = KC_O,    .led_index = 48, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 14, .function = NULL, .state = 0, .timer = 0},  // 14
    {.base = {KIND_CUSTOM}, .row = 2, .col = 10, .keycode = KC_P,    .led_index = 49, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 15, .function = NULL, .state = 0, .timer = 0},  // 15
    {.base = {KIND_CUSTOM}, .row = 2, .col = 11, .keycode = KC_LBRC, .led_index = 50, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 39, .function = NULL, .state = 0, .timer = 0},  // 39
    {.base = {KIND_CUSTOM}, .row = 2, .col = 12, .keycode = KC_RBRC, .led_index = 51, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 40, .function = NULL, .state = 0, .timer = 0},  // 40
    {.base = {KIND_CUSTOM}, .row = 2, .col = 13, .keycode = KC_BSLS, .led_index = 52, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 41, .function = NULL, .state = 0, .timer = 0},  // 41
    // Row 3
    {.base = {KIND_CUSTOM}, .row = 3, .col = 1,  .keycode = KC_A,    .led_index = 60, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 0,  .function = NULL},   // 0
    {.base = {KIND_CUSTOM}, .row = 3, .col = 2,  .keycode = KC_S,    .led_index = 61, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 18, .function = NULL, .state = 0, .timer = 0},  // 18
    {.base = {KIND_CUSTOM}, .row = 3, .col = 3,  .keycode = KC_D,    .led_index = 62, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 3,  .function = NULL},   // 3
    {.base = {KIND_CUSTOM}, .row = 3, .col = 4,  .keycode = KC_F,    .led_index = 63, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 5,  .function = NULL},   // 5
    {.base = {KIND_CUSTOM}, .row = 3, .col = 5,  .keycode = KC_G,    .led_index = 64, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 6,  .function = NULL},   // 6
    {.base = {KIND_CUSTOM}, .row = 3, .col = 6,  .keycode = KC_H,    .led_index = 65, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 7,  .function = NULL},   // 7
    {.base = {KIND_CUSTOM}, .row = 3, .col = 7,  .keycode = KC_J,    .led_index = 66, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 9,  .function = NULL},   // 9
    {.base = {KIND_CUSTOM}, .row = 3, .col = 8,  .keycode = KC_K,    .led_index = 67, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 10, .function = NULL, .state = 0, .timer = 0},  // 10
    {.base = {KIND_CUSTOM}, .row = 3, .col = 9,  .keycode = KC_L,    .led_index = 68, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 11, .function = NULL, .state = 0, .timer = 0},  // 11
    {.base = {KIND_CUSTOM}, .row = 3, .col = 10, .keycode = KC_SCLN, .led_index = 69, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 42, .function = NULL, .state = 0, .timer = 0},  // 42
    {.base = {KIND_CUSTOM}, .row = 3, .col = 11, .keycode = KC_QUOT, .led_index = 70, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 43, .function = NULL, .state = 0, .timer = 0},  // 43
    // Row 4
    {.base = {KIND_CUSTOM}, .row = 4, .col = 2,  .keycode = KC_Z,    .led_index = 80, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 25, .function = NULL, .state = 0, .timer = 0},  // 25
    {.base = {KIND_CUSTOM}, .row = 4, .col = 3,  .keycode = KC_X,    .led_index = 81, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 23, .function = NULL, .state = 0, .timer = 0},  // 23
    {.base = {KIND_CUSTOM}, .row = 4, .col = 4,  .keycode = KC_C,    .led_index = 82, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 2,  .function = NULL},   // 2
    {.base = {KIND_CUSTOM}, .row = 4, .col = 5,  .keycode = KC_V,    .led_index = 83, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 21, .function = NULL, .state = 0, .timer = 0},  // 21
    {.base = {KIND_CUSTOM}, .row = 4, .col = 6,  .keycode = KC_B,    .led_index = 84, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 1,  .function = NULL},   // 1
    {.base = {KIND_CUSTOM}, .row = 4, .col = 7,  .keycode = KC_N,    .led_index = 85, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 13, .function = NULL, .state = 0, .timer = 0},  // 13
    {.base = {KIND_CUSTOM}, .row = 4, .col = 8,  .keycode = KC_M,    .led_index = 86, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 12, .function = NULL, .state = 0, .timer = 0},  // 12
    {.base = {KIND_CUSTOM}, .row = 4, .col = 9,  .keycode = KC_COMM, .led_index = 87, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 45, .function = NULL, .state = 0, .timer = 0},  // 45
    {.base = {KIND_CUSTOM}, .row = 4, .col = 10, .keycode = KC_DOT,  .led_index = 88, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 46, .function = NULL, .state = 0, .timer = 0},  // 46
    {.base = {KIND_CUSTOM}, .row = 4, .col = 11, .keycode = KC_SLSH, .led_index = 89, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 47, .function = NULL, .state = 0, .timer = 0},  // 47
    // Row 5
    {.base = {KIND_CUSTOM}, .row = 5, .col = 6,  .keycode = KC_SPC,  .led_index = 98, .accepted_keymods = KEYMOD_CUSTOM_MASK, .persist_index = 36, .function = NULL, .state = 0, .timer = 0},  // 36 (KC_SPC)
};

// Pool de disabled: teclas que não são position, custom, modifier nem bold
// Led_index usados: positions (36, 54, 56, 57, 58, 76, 77, 78, 92, 93, 94, 106) + customs (19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 98) + modifiers (39, 59, 79, 90, 95, 96, 97, 99, 100, 101, 102) + bold (0, 32, 71)
static disabled_t g_disabled_pool[DISABLED_COUNT] = {
    // ESC removido (agora é bold, led_index 0)
    {.base = {KIND_DISABLED}, .row = 0, .col = 2, .keycode = KC_F1, .led_index = 1, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 3, .keycode = KC_F2, .led_index = 2, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 4, .keycode = KC_F3, .led_index = 3, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 5, .keycode = KC_F4, .led_index = 4, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 6, .keycode = KC_F5, .led_index = 5, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 7, .keycode = KC_F6, .led_index = 6, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 8, .keycode = KC_F7, .led_index = 7, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 9, .keycode = KC_F8, .led_index = 8, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 10, .keycode = KC_F9, .led_index = 9, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 11, .keycode = KC_F10, .led_index = 10, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 12, .keycode = KC_F11, .led_index = 11, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 13, .keycode = KC_F12, .led_index = 12, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 14, .keycode = KC_PSCR, .led_index = 13, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 15, .keycode = 32265, .led_index = 14, .function = NULL, .state = 0},  // KC_CTANA (custom code)
    {.base = {KIND_DISABLED}, .row = 0, .col = 16, .keycode = 30753, .led_index = 15, .function = NULL, .state = 0},  // UG_NEXT (custom code)
    {.base = {KIND_DISABLED}, .row = 0, .col = 17, .keycode = KC_TRANSPARENT, .led_index = 16, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 18, .keycode = KC_TRANSPARENT, .led_index = 17, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 0, .col = 19, .keycode = KC_TRANSPARENT, .led_index = 18, .function = NULL, .state = 0},
    // 19 usado (custom)
    // 20-31 usados (custom)
    // BACKSPACE removido (agora é bold, led_index 32)
    {.base = {KIND_DISABLED}, .row = 1, .col = 14, .keycode = KC_INS, .led_index = 33, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 1, .col = 15, .keycode = KC_HOME, .led_index = 34, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 1, .col = 16, .keycode = KC_PGUP, .led_index = 35, .function = NULL, .state = 0},
    // 36 usado (position)
    {.base = {KIND_DISABLED}, .row = 1, .col = 18, .keycode = KC_PSLS, .led_index = 37, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 1, .col = 19, .keycode = KC_PAST, .led_index = 38, .function = NULL, .state = 0},
    // TAB removido (agora é bold, led_index 39)
    // 40-52 usados (custom)
    {.base = {KIND_DISABLED}, .row = 2, .col = 14, .keycode = KC_DEL, .led_index = 53, .function = NULL, .state = 0},
    // 54 usado (position)
    {.base = {KIND_DISABLED}, .row = 2, .col = 16, .keycode = KC_PGDN, .led_index = 55, .function = NULL, .state = 0},
    // 56-58 usados (position)
    // LSFT(KC_TAB) removido (agora é bold, led_index 59)
    // 60-70 usados (custom)
    // ENTER removido (agora é bold, led_index 71)
    {.base = {KIND_DISABLED}, .row = 3, .col = 13, .keycode = KC_TRANSPARENT, .led_index = 72, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 3, .col = 14, .keycode = KC_PMNS, .led_index = 73, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 3, .col = 15, .keycode = KC_PPLS, .led_index = 74, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 3, .col = 16, .keycode = KC_PENT, .led_index = 75, .function = NULL, .state = 0},
    // 76-78 usados (position)
    // LSHIFT removido (agora é bold, led_index 79)
    // 80-89 usados (custom)
    // 90 usado (modifier - RSFT)
    {.base = {KIND_DISABLED}, .row = 4, .col = 15, .keycode = KC_UP, .led_index = 91, .function = NULL, .state = 0},
    // 92-94 usados (position)
    // LCONTROL, LWIN, LALT removidos (agora são bold, led_index 95, 96, 97)
    // 98 usado (custom)
    // 99 usado (modifier - RALT)
    // RWIN removido (agora é bold, led_index 100)
    // 102 usado (modifier - RCTL)
    {.base = {KIND_DISABLED}, .row = 5, .col = 14, .keycode = KC_LEFT, .led_index = 103, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 5, .col = 15, .keycode = KC_DOWN, .led_index = 104, .function = NULL, .state = 0},
    {.base = {KIND_DISABLED}, .row = 5, .col = 16, .keycode = KC_RGHT, .led_index = 105, .function = NULL, .state = 0},
    // 106 usado (position)
    {.base = {KIND_DISABLED}, .row = 5, .col = 19, .keycode = KC_PDOT, .led_index = 107, .function = NULL, .state = 0},
};

// Pool de modifiers: RSHIFT, RALT, FN, RCTRL, TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN
// Pool de modifiers: RSFT, RALT, FN, RCTL
static modifier_t g_modifier_pool[MODIFIER_COUNT] = {
    {.base = {KIND_MODIFIER}, .row = 4, .col = 13, .keycode = KC_RSFT, .led_index = 90, .function = NULL, .state = 0},  // RSFT
    {.base = {KIND_MODIFIER}, .row = 5, .col = 10, .keycode = KC_RALT, .led_index = 99, .function = NULL, .state = 0},  // RALT
    {.base = {KIND_MODIFIER}, .row = 5, .col = 12, .keycode = MO(WIN_FN), .led_index = 101, .function = NULL, .state = 0},  // FN
    {.base = {KIND_MODIFIER}, .row = 5, .col = 13, .keycode = KC_RCTL, .led_index = 102, .function = NULL, .state = 0},  // RCTL
};

// Pool de other: TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN
static others_t g_others_pool[OTHER_COUNT] = {
    {.base = {KIND_OTHER}, .row = 2, .col = 0, .keycode = KC_TAB, .led_index = 39, .function = NULL, .state = 0},     // TAB
    {.base = {KIND_OTHER}, .row = 3, .col = 0, .keycode = LSFT(KC_TAB), .led_index = 59, .function = NULL, .state = 0}, // LSFT(KC_TAB)
    {.base = {KIND_OTHER}, .row = 4, .col = 0, .keycode = KC_LSFT, .led_index = 79, .function = NULL, .state = 0},    // LSHIFT
    {.base = {KIND_OTHER}, .row = 5, .col = 0, .keycode = KC_LCTL, .led_index = 95, .function = NULL, .state = 0},    // LCONTROL
    {.base = {KIND_OTHER}, .row = 5, .col = 1, .keycode = KC_LWIN, .led_index = 96, .function = NULL, .state = 0},    // LWIN
    {.base = {KIND_OTHER}, .row = 5, .col = 2, .keycode = KC_LALT, .led_index = 97, .function = NULL, .state = 0},    // LALT
    {.base = {KIND_OTHER}, .row = 5, .col = 11, .keycode = KC_RWIN, .led_index = 100, .function = NULL, .state = 0},  // RWIN
};

// Pool de bold: ESC, ENTER, BACKSPACE
static bold_t g_bold_pool[BOLD_COUNT] = {
    {.base = {KIND_BOLD}, .row = 0, .col = 0, .keycode = KC_ESC, .led_index = 0, .function = NULL, .state = 0},      // ESC
    {.base = {KIND_BOLD}, .row = 1, .col = 13, .keycode = KC_BSPC, .led_index = 32, .function = NULL, .state = 0},  // BACKSPACE
    {.base = {KIND_BOLD}, .row = 3, .col = 12, .keycode = KC_ENT, .led_index = 71, .function = NULL, .state = 0},    // ENTER
};

// ===== Grid =====

// Grid único: NULL = célula vazia, caso contrário aponta para base_t dentro de algum objeto
// Inicializado como NULL por padrão (static/global)
static base_t* g_grid[MATRIX_ROWS][MATRIX_COLS] = {NULL};

// ===== Helpers =====

static inline bool in_bounds(uint8_t r, uint8_t c) {
    return (r < MATRIX_ROWS) && (c < MATRIX_COLS);
}

// ===== Funções Públicas =====

// Obtém entrada do grid (O(1))
base_t* kind_get_grid_entry(uint8_t row, uint8_t col) {
    if (!in_bounds(row, col)) return NULL;
    return g_grid[row][col];
}

// Cast seguro para profile_key_t
profile_key_t* kind_get_profile(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_PROFILE) return NULL;
    return (profile_key_t*)entry;
}

// Cast seguro para numlock_t
numlock_t* kind_get_numlock(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_NUMLOCK) return NULL;
    return (numlock_t*)entry;
}

// Cast seguro para persistence_key_t
persistence_key_t* kind_get_persistence(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_PERSISTENCE) return NULL;
    return (persistence_key_t*)entry;
}

// Cast seguro para custom_t
custom_t* kind_get_custom(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_CUSTOM) return NULL;
    return (custom_t*)entry;
}

// Cast seguro para disabled_t
disabled_t* kind_get_disabled(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_DISABLED) return NULL;
    return (disabled_t*)entry;
}

// Cast seguro para modifier_t
modifier_t* kind_get_modifier(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_MODIFIER) return NULL;
    return (modifier_t*)entry;
}

// Cast seguro para bold_t
bold_t* kind_get_bold(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_BOLD) return NULL;
    return (bold_t*)entry;
}

// Cast seguro para others_t
others_t* kind_get_other(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_OTHER) return NULL;
    return (others_t*)entry;
}

// Obtém custom_t por persist_index (busca linear no pool)
custom_t* kind_get_custom_by_index(uint8_t persist_index) {
    if (persist_index >= CUSTOM_COUNT) return NULL;
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        if (g_custom_pool[i].persist_index == persist_index) {
            return &g_custom_pool[i];
        }
    }
    return NULL;
}


// Itera sobre pool de profiles
void kind_iterate_profiles(bool (*callback)(profile_key_t* profile, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < PROFILE_COUNT; i++) {
        if (!callback(&g_profile_pool[i], user_data)) return;
    }
}

// Itera sobre pool de numlock
void kind_iterate_numlock(bool (*callback)(numlock_t* numlock, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < NUMLOCK_COUNT; i++) {
        if (!callback(&g_numlock_pool[i], user_data)) return;
    }
}

// Itera sobre pool de persistence
void kind_iterate_persistence(bool (*callback)(persistence_key_t* persistence, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < PERSISTENCE_COUNT; i++) {
        if (!callback(&g_persistence_pool[i], user_data)) return;
    }
}

// Itera sobre pool de customs
void kind_iterate_customs(bool (*callback)(custom_t* custom, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        if (!callback(&g_custom_pool[i], user_data)) return;
    }
}

// Itera sobre pool de disabled
void kind_iterate_disabled(bool (*callback)(disabled_t* disabled, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < DISABLED_COUNT; i++) {
        if (!callback(&g_disabled_pool[i], user_data)) return;
    }
}

// Itera sobre pool de modifiers
void kind_iterate_modifiers(bool (*callback)(modifier_t* modifier, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < MODIFIER_COUNT; i++) {
        if (!callback(&g_modifier_pool[i], user_data)) return;
    }
}

// Itera sobre pool de bold
void kind_iterate_bold(bool (*callback)(bold_t* bold, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < BOLD_COUNT; i++) {
        if (!callback(&g_bold_pool[i], user_data)) return;
    }
}

// Itera sobre pool de others
void kind_iterate_other(bool (*callback)(others_t* other, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < OTHER_COUNT; i++) {
        if (!callback(&g_others_pool[i], user_data)) return;
    }
}

// Itera sobre todas as entradas do grid
void kind_iterate_grid(bool (*callback)(base_t* entry, void* user_data), void* user_data) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            base_t* entry = g_grid[row][col];
            if (entry != NULL) {
                if (!callback(entry, user_data)) return;
            }
        }
    }
}

// ===== Inicialização =====

// Preenche grid com ponteiros para os pools (chamado em keyboard_post_init_user)
// led_index já está preenchido estaticamente nos arrays
void kind_init_grid(void) {
    // Zera o grid primeiro (garante que não há lixo)
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            g_grid[row][col] = NULL;
        }
    }
    
    // Preenche grid com customs
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = &g_custom_pool[i];
        if (in_bounds(custom->row, custom->col)) {
            // Registra no grid
            g_grid[custom->row][custom->col] = (base_t*)custom;
        }
    }
    
    // Preenche grid com profiles (sobrescreve customs se houver colisão)
    for (uint8_t i = 0; i < PROFILE_COUNT; i++) {
        profile_key_t* profile = &g_profile_pool[i];
        if (in_bounds(profile->row, profile->col)) {
            // Registra no grid (sobrescreve custom se houver colisão)
            g_grid[profile->row][profile->col] = (base_t*)profile;
        }
    }
    
    // Preenche grid com numlock (sobrescreve customs e profiles se houver colisão)
    for (uint8_t i = 0; i < NUMLOCK_COUNT; i++) {
        numlock_t* numlock = &g_numlock_pool[i];
        if (in_bounds(numlock->row, numlock->col)) {
            // Registra no grid (sobrescreve custom e profile se houver colisão)
            g_grid[numlock->row][numlock->col] = (base_t*)numlock;
        }
    }
    
    // Preenche grid com persistence (sobrescreve customs, profiles e numlock se houver colisão)
    for (uint8_t i = 0; i < PERSISTENCE_COUNT; i++) {
        persistence_key_t* persistence = &g_persistence_pool[i];
        if (in_bounds(persistence->row, persistence->col)) {
            // Registra no grid (sobrescreve custom, profile e numlock se houver colisão)
            g_grid[persistence->row][persistence->col] = (base_t*)persistence;
        }
    }
    
    // Preenche grid com modifiers (sobrescreve positions e customs se houver colisão)
    for (uint8_t i = 0; i < MODIFIER_COUNT; i++) {
        modifier_t* modifier = &g_modifier_pool[i];
        if (in_bounds(modifier->row, modifier->col)) {
            // Registra no grid (sobrescreve position e custom se houver colisão)
            g_grid[modifier->row][modifier->col] = (base_t*)modifier;
        }
    }
    
    // Preenche grid com bold (sobrescreve positions, customs e modifiers se houver colisão)
    for (uint8_t i = 0; i < BOLD_COUNT; i++) {
        bold_t* bold = &g_bold_pool[i];
        if (in_bounds(bold->row, bold->col)) {
            // Registra no grid (sobrescreve position, custom e modifier se houver colisão)
            g_grid[bold->row][bold->col] = (base_t*)bold;
        }
    }
    
    // Preenche grid com others (sobrescreve positions, customs, modifiers e bold se houver colisão)
    for (uint8_t i = 0; i < OTHER_COUNT; i++) {
        others_t* others = &g_others_pool[i];
        if (in_bounds(others->row, others->col)) {
            // Registra no grid (sobrescreve profile, numlock, persistence, custom, modifier e bold se houver colisão)
            g_grid[others->row][others->col] = (base_t*)others;
        }
    }
    
    // Preenche grid com disabled (sobrescreve profiles, numlock, persistence, customs, modifiers, bold e others se houver colisão)
    for (uint8_t i = 0; i < DISABLED_COUNT; i++) {
        disabled_t* disabled = &g_disabled_pool[i];
        if (in_bounds(disabled->row, disabled->col)) {
            // Registra no grid (sobrescreve position, custom e modifier se houver colisão)
            g_grid[disabled->row][disabled->col] = (base_t*)disabled;
        }
    }
}

// Registra função para uma entrada do grid
bool kind_register_function(uint8_t row, uint8_t col, key_function_t function) {
    if (!in_bounds(row, col)) return false;
    
    base_t* entry = g_grid[row][col];
    if (entry == NULL) return false;
    
    if (entry->kind == KIND_PROFILE) {
        profile_key_t* profile = (profile_key_t*)entry;
        profile->function = function;
    } else if (entry->kind == KIND_NUMLOCK) {
        numlock_t* numlock = (numlock_t*)entry;
        numlock->function = function;
    } else if (entry->kind == KIND_PERSISTENCE) {
        persistence_key_t* persistence = (persistence_key_t*)entry;
        persistence->function = function;
    } else if (entry->kind == KIND_CUSTOM) {
        custom_t* custom = (custom_t*)entry;
        custom->function = function;
    } else if (entry->kind == KIND_MODIFIER) {
        modifier_t* modifier = (modifier_t*)entry;
        modifier->function = function;
    } else if (entry->kind == KIND_OTHER) {
        others_t* others = (others_t*)entry;
        others->function = function;
    } else if (entry->kind == KIND_DISABLED) {
        disabled_t* disabled = (disabled_t*)entry;
        disabled->function = function;
    } else if (entry->kind == KIND_BOLD) {
        bold_t* bold = (bold_t*)entry;
        bold->function = function;
    } else {
        return false;
    }
    
    return true;
}
