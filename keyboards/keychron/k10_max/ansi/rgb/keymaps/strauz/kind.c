#include "kind.h"
#include "keymap.h"  // Para WIN_BASE
#include "keymod.h"  // Para KEYMOD_ALL

// ===== Pools Estáticos =====

// Pool de profiles: 10 teclas (P0-P9)
// Todos os valores preenchidos estaticamente, exceto process_key (preenchido pelos módulos)
// Ordenado por profile_id (0-9) para corresponder ao índice do array
static profile_key_t g_profile_pool[PROFILE_COUNT] = {
    // Profile 0
    {.kind = KIND_PROFILE, .row = 5, .col = 18, .keycode = KC_P0, .led_index = 106, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 0},  // P0
    // Profile 1
    {.kind = KIND_PROFILE, .row = 4, .col = 17, .keycode = KC_P1, .led_index = 92, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 1},  // P1
    // Profile 2
    {.kind = KIND_PROFILE, .row = 4, .col = 18, .keycode = KC_P2, .led_index = 93, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 2},  // P2
    // Profile 3
    {.kind = KIND_PROFILE, .row = 4, .col = 19, .keycode = KC_P3, .led_index = 94, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 3},  // P3
    // Profile 4
    {.kind = KIND_PROFILE, .row = 3, .col = 17, .keycode = KC_P4, .led_index = 76, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 4},  // P4
    // Profile 5
    {.kind = KIND_PROFILE, .row = 3, .col = 18, .keycode = KC_P5, .led_index = 77, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 5},  // P5
    // Profile 6
    {.kind = KIND_PROFILE, .row = 3, .col = 19, .keycode = KC_P6, .led_index = 78, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 6},  // P6
    // Profile 7
    {.kind = KIND_PROFILE, .row = 2, .col = 17, .keycode = KC_P7, .led_index = 56, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 7},  // P7
    // Profile 8
    {.kind = KIND_PROFILE, .row = 2, .col = 18, .keycode = KC_P8, .led_index = 57, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 8},  // P8
    // Profile 9
    {.kind = KIND_PROFILE, .row = 2, .col = 19, .keycode = KC_P9, .led_index = 58, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0, .profile_index = 9},  // P9
};

// Pool de numlock: 1 tecla (KC_NUM)
static numlock_t g_numlock_pool[NUMLOCK_COUNT] = {
    {.kind = KIND_NUMLOCK, .row = 1, .col = 17, .keycode = KC_NUM, .led_index = 36, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},  // KC_NUM
};

// Pool de persistence: 1 tecla (KC_END)
static persistence_key_t g_persistence_pool[PERSISTENCE_COUNT] = {
    {.kind = KIND_PERSISTENCE, .row = 2, .col = 15, .keycode = KC_END, .led_index = 54, .accepted_keymods = KEYMOD_NONE | KEYMOD_FN_ONLY, .process_key = NULL, .state = 0},  // KC_END
};

// Pool de customs: 48 teclas (persist_index 0-47)
// Todos os valores preenchidos estaticamente, exceto process_key (preenchido pelos módulos)
// Ordenado por row, depois por col (mantém persist_index original)
// accepted_keymods para custom: aceita NONE, FN_RCTL ou FN_RALT
#define KEYMOD_CUSTOM_MASK  (KEYMOD_NONE | KEYMOD_FN_RCTL | KEYMOD_FN_RALT)
static custom_t g_custom_pool[CUSTOM_COUNT] = {
    // Row 1
    {.kind = KIND_CUSTOM, .row = 1, .col = 0, .keycode = KC_GRV, .led_index = 19, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 44, .timer = 0, .custom_behavior = CUSTOM_BEHAVIOR_UNASSOCIATED},  // 44
    {.kind = KIND_CUSTOM, .row = 1, .col = 1, .keycode = KC_1, .led_index = 20, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 26, .timer = 0},  // 26
    {.kind = KIND_CUSTOM, .row = 1, .col = 2, .keycode = KC_2, .led_index = 21, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 27, .timer = 0},  // 27
    {.kind = KIND_CUSTOM, .row = 1, .col = 3, .keycode = KC_3, .led_index = 22, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 28, .timer = 0},  // 28
    {.kind = KIND_CUSTOM, .row = 1, .col = 4, .keycode = KC_4, .led_index = 23, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 29, .timer = 0},  // 29
    {.kind = KIND_CUSTOM, .row = 1, .col = 5, .keycode = KC_5, .led_index = 24, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 30, .timer = 0},  // 30
    {.kind = KIND_CUSTOM, .row = 1, .col = 6, .keycode = KC_6, .led_index = 25, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 31, .timer = 0},  // 31
    {.kind = KIND_CUSTOM, .row = 1, .col = 7, .keycode = KC_7, .led_index = 26, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 32, .timer = 0},  // 32
    {.kind = KIND_CUSTOM, .row = 1, .col = 8, .keycode = KC_8, .led_index = 27, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 33, .timer = 0},  // 33
    {.kind = KIND_CUSTOM, .row = 1, .col = 9, .keycode = KC_9, .led_index = 28, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 34, .timer = 0},  // 34
    {.kind = KIND_CUSTOM, .row = 1, .col = 10, .keycode = KC_0, .led_index = 29, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 35, .timer = 0},  // 35
    {.kind = KIND_CUSTOM, .row = 1, .col = 11, .keycode = KC_MINS, .led_index = 30, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 37, .timer = 0},  // 37
    {.kind = KIND_CUSTOM, .row = 1, .col = 12, .keycode = KC_EQL, .led_index = 31, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 38, .timer = 0},  // 38
    // Row 2
    {.kind = KIND_CUSTOM, .row = 2, .col = 1, .keycode = KC_Q, .led_index = 40, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 16, .timer = 0},  // 16
    {.kind = KIND_CUSTOM, .row = 2, .col = 2, .keycode = KC_W, .led_index = 41, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 22, .timer = 0},  // 22
    {.kind = KIND_CUSTOM, .row = 2, .col = 3, .keycode = KC_E, .led_index = 42, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 4, .timer = 0},   // 4
    {.kind = KIND_CUSTOM, .row = 2, .col = 4, .keycode = KC_R, .led_index = 43, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 17, .timer = 0},  // 17
    {.kind = KIND_CUSTOM, .row = 2, .col = 5, .keycode = KC_T, .led_index = 44, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 19, .timer = 0},  // 19
    {.kind = KIND_CUSTOM, .row = 2, .col = 6, .keycode = KC_Y, .led_index = 45, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 24, .timer = 0},  // 24
    {.kind = KIND_CUSTOM, .row = 2, .col = 7, .keycode = KC_U, .led_index = 46, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 20, .timer = 0},  // 20
    {.kind = KIND_CUSTOM, .row = 2, .col = 8, .keycode = KC_I, .led_index = 47, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 8, .timer = 0},   // 8
    {.kind = KIND_CUSTOM, .row = 2, .col = 9, .keycode = KC_O, .led_index = 48, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 14, .timer = 0},  // 14
    {.kind = KIND_CUSTOM, .row = 2, .col = 10, .keycode = KC_P, .led_index = 49, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 15, .timer = 0},  // 15
    {.kind = KIND_CUSTOM, .row = 2, .col = 11, .keycode = KC_LBRC, .led_index = 50, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 39, .timer = 0},  // 39
    {.kind = KIND_CUSTOM, .row = 2, .col = 12, .keycode = KC_RBRC, .led_index = 51, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 40, .timer = 0},  // 40
    {.kind = KIND_CUSTOM, .row = 2, .col = 13, .keycode = KC_BSLS, .led_index = 52, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 41, .timer = 0},  // 41
    // Row 3
    {.kind = KIND_CUSTOM, .row = 3, .col = 1, .keycode = KC_A, .led_index = 60, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 0, .timer = 0},   // 0
    {.kind = KIND_CUSTOM, .row = 3, .col = 2, .keycode = KC_S, .led_index = 61, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 18, .timer = 0},  // 18
    {.kind = KIND_CUSTOM, .row = 3, .col = 3, .keycode = KC_D, .led_index = 62, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 3, .timer = 0},   // 3
    {.kind = KIND_CUSTOM, .row = 3, .col = 4, .keycode = KC_F, .led_index = 63, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 5, .timer = 0},   // 5
    {.kind = KIND_CUSTOM, .row = 3, .col = 5, .keycode = KC_G, .led_index = 64, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 6, .timer = 0},   // 6
    {.kind = KIND_CUSTOM, .row = 3, .col = 6, .keycode = KC_H, .led_index = 65, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 7, .timer = 0},   // 7
    {.kind = KIND_CUSTOM, .row = 3, .col = 7, .keycode = KC_J, .led_index = 66, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 9, .timer = 0},   // 9
    {.kind = KIND_CUSTOM, .row = 3, .col = 8, .keycode = KC_K, .led_index = 67, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 10, .timer = 0},  // 10
    {.kind = KIND_CUSTOM, .row = 3, .col = 9, .keycode = KC_L, .led_index = 68, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 11, .timer = 0},  // 11
    {.kind = KIND_CUSTOM, .row = 3, .col = 10, .keycode = KC_SCLN, .led_index = 69, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 42, .timer = 0},  // 42
    {.kind = KIND_CUSTOM, .row = 3, .col = 11, .keycode = KC_QUOT, .led_index = 70, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 43, .timer = 0},  // 43
    // Row 4
    {.kind = KIND_CUSTOM, .row = 4, .col = 2, .keycode = KC_Z, .led_index = 80, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 25, .timer = 0},  // 25
    {.kind = KIND_CUSTOM, .row = 4, .col = 3, .keycode = KC_X, .led_index = 81, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 23, .timer = 0},  // 23
    {.kind = KIND_CUSTOM, .row = 4, .col = 4, .keycode = KC_C, .led_index = 82, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 2, .timer = 0},   // 2
    {.kind = KIND_CUSTOM, .row = 4, .col = 5, .keycode = KC_V, .led_index = 83, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 21, .timer = 0},  // 21
    {.kind = KIND_CUSTOM, .row = 4, .col = 6, .keycode = KC_B, .led_index = 84, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 1, .timer = 0},   // 1
    {.kind = KIND_CUSTOM, .row = 4, .col = 7, .keycode = KC_N, .led_index = 85, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 13, .timer = 0},  // 13
    {.kind = KIND_CUSTOM, .row = 4, .col = 8, .keycode = KC_M, .led_index = 86, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 12, .timer = 0},  // 12
    {.kind = KIND_CUSTOM, .row = 4, .col = 9, .keycode = KC_COMM, .led_index = 87, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 45, .timer = 0},  // 45
    {.kind = KIND_CUSTOM, .row = 4, .col = 10, .keycode = KC_DOT, .led_index = 88, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 46, .timer = 0},  // 46
    {.kind = KIND_CUSTOM, .row = 4, .col = 11, .keycode = KC_SLSH, .led_index = 89, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 47, .timer = 0},  // 47
    // Row 5
    {.kind = KIND_CUSTOM, .row = 5, .col = 6, .keycode = KC_SPC, .led_index = 98, .accepted_keymods = KEYMOD_CUSTOM_MASK, .process_key = NULL, .state = 0, .persist_index = 36, .timer = 0},  // 36 (KC_SPC)
};

// Pool de unused: teclas que não são position, custom, modifier nem bold
// Led_index usados: positions (36, 54, 56, 57, 58, 76, 77, 78, 92, 93, 94, 106) + customs (19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 98) + modifiers (39, 59, 79, 90, 95, 96, 97, 99, 100, 101, 102) + bold (0, 32, 71)
static unused_t g_unused_pool[UNUSED_COUNT] = {
    // ESC removido (agora é bold, led_index 0)
    {.kind = KIND_UNUSED, .row = 0, .col = 2, .keycode = KC_F1, .led_index = 1, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 3, .keycode = KC_F2, .led_index = 2, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 4, .keycode = KC_F3, .led_index = 3, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 5, .keycode = KC_F4, .led_index = 4, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 6, .keycode = KC_F5, .led_index = 5, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 7, .keycode = KC_F6, .led_index = 6, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 8, .keycode = KC_F7, .led_index = 7, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 9, .keycode = KC_F8, .led_index = 8, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 10, .keycode = KC_F9, .led_index = 9, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 11, .keycode = KC_F10, .led_index = 10, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 12, .keycode = KC_F11, .led_index = 11, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 13, .keycode = KC_F12, .led_index = 12, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 14, .keycode = KC_PSCR, .led_index = 13, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 15, .keycode = KC_TRANSPARENT, .led_index = 14, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 16, .keycode = 30753, .led_index = 15, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},  // UG_NEXT (custom code)
    {.kind = KIND_UNUSED, .row = 0, .col = 17, .keycode = KC_TRANSPARENT, .led_index = 16, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 18, .keycode = KC_TRANSPARENT, .led_index = 17, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 0, .col = 19, .keycode = KC_TRANSPARENT, .led_index = 18, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    // 19 usado (custom)
    // 20-31 usados (custom)
    // BACKSPACE removido (agora é bold, led_index 32)
    {.kind = KIND_UNUSED, .row = 1, .col = 14, .keycode = KC_INS, .led_index = 33, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 1, .col = 15, .keycode = KC_HOME, .led_index = 34, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 1, .col = 16, .keycode = KC_PGUP, .led_index = 35, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    // 36 usado (position)
    {.kind = KIND_UNUSED, .row = 1, .col = 18, .keycode = KC_PSLS, .led_index = 37, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 1, .col = 19, .keycode = KC_PAST, .led_index = 38, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    // TAB removido (agora é bold, led_index 39)
    // 40-52 usados (custom)
    {.kind = KIND_UNUSED, .row = 2, .col = 14, .keycode = KC_DEL, .led_index = 53, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    // 54 usado (position)
    {.kind = KIND_UNUSED, .row = 2, .col = 16, .keycode = KC_PGDN, .led_index = 55, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    // 56-58 usados (position)
    // LSFT(KC_TAB) removido (agora é bold, led_index 59)
    // 60-70 usados (custom)
    // ENTER removido (agora é bold, led_index 71)
    {.kind = KIND_UNUSED, .row = 3, .col = 13, .keycode = KC_TRANSPARENT, .led_index = 72, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 3, .col = 14, .keycode = KC_PMNS, .led_index = 73, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 3, .col = 15, .keycode = KC_PPLS, .led_index = 74, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 3, .col = 16, .keycode = KC_PENT, .led_index = 75, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    // 76-78 usados (position)
    // LSHIFT removido (agora é bold, led_index 79)
    // 80-89 usados (custom)
    // 90 usado (modifier - RSFT)
    {.kind = KIND_UNUSED, .row = 4, .col = 15, .keycode = KC_UP, .led_index = 91, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    // 92-94 usados (position)
    // LCONTROL, LWIN, LALT removidos (agora são bold, led_index 95, 96, 97)
    // 98 usado (custom)
    // 99 usado (modifier - RALT)
    // RWIN removido (agora é bold, led_index 100)
    // 102 usado (modifier - RCTL)
    {.kind = KIND_UNUSED, .row = 5, .col = 14, .keycode = KC_LEFT, .led_index = 103, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 5, .col = 15, .keycode = KC_DOWN, .led_index = 104, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    {.kind = KIND_UNUSED, .row = 5, .col = 16, .keycode = KC_RGHT, .led_index = 105, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
    // 106 usado (position)
    {.kind = KIND_UNUSED, .row = 5, .col = 19, .keycode = KC_PDOT, .led_index = 107, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},
};

// Pool de modifiers: RSHIFT, RALT, FN, RCTRL, TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN
// Pool de modifiers: RSFT, RALT, FN, RCTL
static modifier_t g_modifier_pool[MODIFIER_COUNT] = {
    {.kind = KIND_MODIFIER, .row = 4, .col = 13, .keycode = KC_RSFT, .led_index = 90, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},  // RSFT
    {.kind = KIND_MODIFIER, .row = 5, .col = 10, .keycode = KC_RALT, .led_index = 99, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},  // RALT
    {.kind = KIND_MODIFIER, .row = 5, .col = 12, .keycode = MO(WIN_FN), .led_index = 101, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},  // FN
    {.kind = KIND_MODIFIER, .row = 5, .col = 13, .keycode = KC_RCTL, .led_index = 102, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},  // RCTL
};

// Pool de unused_modifiers: TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN
static unused_modifier_t g_unused_modifiers_pool[UNUSED_MODIFIERS_COUNT] = {
    {.kind = KIND_UNUSED_MODIFIERS, .row = 2, .col = 0, .keycode = KC_TAB, .led_index = 39, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},     // TAB
    {.kind = KIND_UNUSED_MODIFIERS, .row = 3, .col = 0, .keycode = LSFT(KC_TAB), .led_index = 59, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0}, // LSFT(KC_TAB)
    {.kind = KIND_UNUSED_MODIFIERS, .row = 4, .col = 0, .keycode = KC_LSFT, .led_index = 79, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},    // LSHIFT
    {.kind = KIND_UNUSED_MODIFIERS, .row = 5, .col = 0, .keycode = KC_LCTL, .led_index = 95, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},    // LCONTROL
    {.kind = KIND_UNUSED_MODIFIERS, .row = 5, .col = 1, .keycode = KC_LWIN, .led_index = 96, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},    // LWIN
    {.kind = KIND_UNUSED_MODIFIERS, .row = 5, .col = 2, .keycode = KC_LALT, .led_index = 97, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},    // LALT
    {.kind = KIND_UNUSED_MODIFIERS, .row = 5, .col = 11, .keycode = KC_RWIN, .led_index = 100, .accepted_keymods = KEYMOD_ALL, .process_key = NULL, .state = 0},  // RWIN
};

// Pool de bold: ESC, ENTER, BACKSPACE
static bold_t g_bold_pool[BOLD_COUNT] = {
    {.kind = KIND_BOLD, .row = 0, .col = 0, .keycode = KC_ESC, .led_index = 0, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},      // ESC
    {.kind = KIND_BOLD, .row = 1, .col = 13, .keycode = KC_BSPC, .led_index = 32, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},  // BACKSPACE
    {.kind = KIND_BOLD, .row = 3, .col = 12, .keycode = KC_ENT, .led_index = 71, .accepted_keymods = KEYMOD_NONE, .process_key = NULL, .state = 0},    // ENTER
};

// ===== Grid =====

// Grid único: NULL = célula vazia, caso contrário aponta para base_key_t dentro de algum objeto
// Inicializado como NULL por padrão (static/global)
static base_key_t* g_grid[MATRIX_ROWS][MATRIX_COLS] = {NULL};

// ===== Helpers =====

static inline bool in_bounds(uint8_t r, uint8_t c) {
    return (r < MATRIX_ROWS) && (c < MATRIX_COLS);
}

// ===== Funções Públicas =====

// Obtém entrada do grid (O(1))
base_key_t* kind_get_grid_entry(uint8_t row, uint8_t col) {
    if (!in_bounds(row, col)) return NULL;
    return g_grid[row][col];
}

// Obtém a tecla numlock (retorna diretamente do pool, sem precisar de posição)
numlock_t* kind_get_numlock_key(void) {
    return &g_numlock_pool[0];
}

// Obtém a tecla persistence (retorna diretamente do pool, sem precisar de posição)
persistence_key_t* kind_get_persistence_key(void) {
    return &g_persistence_pool[0];
}

// Cast seguro para custom_t
custom_t* kind_get_custom(uint8_t row, uint8_t col) {
    base_key_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_CUSTOM) return NULL;
    return (custom_t*)entry;
}

// Obtém teclas modificadoras diretamente do pool (sem precisar de posição)
// Ordem do pool: [0] RSFT, [1] RALT, [2] FN, [3] RCTL
modifier_t* kind_get_rsft_key(void) {
    return &g_modifier_pool[0];  // RSFT
}

modifier_t* kind_get_ralt_key(void) {
    return &g_modifier_pool[1];  // RALT
}

modifier_t* kind_get_fn_key(void) {
    return &g_modifier_pool[2];  // FN
}

modifier_t* kind_get_rctl_key(void) {
    return &g_modifier_pool[3];  // RCTL
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

// Itera sobre pool de unused
void kind_iterate_unused(bool (*callback)(unused_t* unused, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < UNUSED_COUNT; i++) {
        if (!callback(&g_unused_pool[i], user_data)) return;
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

// Itera sobre pool de unused_modifiers
void kind_iterate_unused_modifiers(bool (*callback)(unused_modifier_t* unused_modifier, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < UNUSED_MODIFIERS_COUNT; i++) {
        if (!callback(&g_unused_modifiers_pool[i], user_data)) return;
    }
}

// Itera sobre todas as entradas do grid
void kind_iterate_grid(bool (*callback)(base_key_t* entry, void* user_data), void* user_data) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            base_key_t* entry = g_grid[row][col];
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
            g_grid[custom->row][custom->col] = (base_key_t*)custom;
        }
    }
    
    // Preenche grid com profiles (sobrescreve customs se houver colisão)
    for (uint8_t i = 0; i < PROFILE_COUNT; i++) {
        profile_key_t* profile = &g_profile_pool[i];
        if (in_bounds(profile->row, profile->col)) {
            // Registra no grid (sobrescreve custom se houver colisão)
            g_grid[profile->row][profile->col] = (base_key_t*)profile;
        }
    }
    
    // Preenche grid com numlock (sobrescreve customs e profiles se houver colisão)
    for (uint8_t i = 0; i < NUMLOCK_COUNT; i++) {
        numlock_t* numlock = &g_numlock_pool[i];
        if (in_bounds(numlock->row, numlock->col)) {
            // Registra no grid (sobrescreve custom e profile se houver colisão)
            g_grid[numlock->row][numlock->col] = (base_key_t*)numlock;
        }
    }
    
    // Preenche grid com persistence (sobrescreve customs, profiles e numlock se houver colisão)
    for (uint8_t i = 0; i < PERSISTENCE_COUNT; i++) {
        persistence_key_t* persistence = &g_persistence_pool[i];
        if (in_bounds(persistence->row, persistence->col)) {
            // Registra no grid (sobrescreve custom, profile e numlock se houver colisão)
            g_grid[persistence->row][persistence->col] = (base_key_t*)persistence;
        }
    }
    
    // Preenche grid com modifiers (sobrescreve positions e customs se houver colisão)
    for (uint8_t i = 0; i < MODIFIER_COUNT; i++) {
        modifier_t* modifier = &g_modifier_pool[i];
        if (in_bounds(modifier->row, modifier->col)) {
            // Registra no grid (sobrescreve position e custom se houver colisão)
            g_grid[modifier->row][modifier->col] = (base_key_t*)modifier;
        }
    }
    
    // Preenche grid com bold (sobrescreve positions, customs e modifiers se houver colisão)
    for (uint8_t i = 0; i < BOLD_COUNT; i++) {
        bold_t* bold = &g_bold_pool[i];
        if (in_bounds(bold->row, bold->col)) {
            // Registra no grid (sobrescreve position, custom e modifier se houver colisão)
            g_grid[bold->row][bold->col] = (base_key_t*)bold;
        }
    }
    
    // Preenche grid com unused_modifiers (sobrescreve positions, customs, modifiers e bold se houver colisão)
    for (uint8_t i = 0; i < UNUSED_MODIFIERS_COUNT; i++) {
        unused_modifier_t* unused_modifier = &g_unused_modifiers_pool[i];
        if (in_bounds(unused_modifier->row, unused_modifier->col)) {
            // Registra no grid (sobrescreve profile, numlock, persistence, custom, modifier e bold se houver colisão)
            g_grid[unused_modifier->row][unused_modifier->col] = (base_key_t*)unused_modifier;
        }
    }
    
    // Preenche grid com unused (sobrescreve profiles, numlock, persistence, customs, modifiers, bold e unused_modifiers se houver colisão)
    for (uint8_t i = 0; i < UNUSED_COUNT; i++) {
        unused_t* unused = &g_unused_pool[i];
        if (in_bounds(unused->row, unused->col)) {
            // Registra no grid (sobrescreve position, custom e modifier se houver colisão)
            g_grid[unused->row][unused->col] = (base_key_t*)unused;
        }
    }
}

// Registra função para uma entrada do grid
bool kind_register_function(uint8_t row, uint8_t col, process_key_t function) {
    if (!in_bounds(row, col)) return false;
    
    base_key_t* entry = g_grid[row][col];
    if (entry == NULL) return false;
    
    if (entry->kind == KIND_PROFILE) {
        profile_key_t* profile = (profile_key_t*)entry;
        profile->process_key = function;
    } else if (entry->kind == KIND_NUMLOCK) {
        numlock_t* numlock = (numlock_t*)entry;
        numlock->process_key = function;
    } else if (entry->kind == KIND_PERSISTENCE) {
        persistence_key_t* persistence = (persistence_key_t*)entry;
        persistence->process_key = function;
    } else if (entry->kind == KIND_CUSTOM) {
        custom_t* custom = (custom_t*)entry;
        custom->process_key = function;
    } else if (entry->kind == KIND_MODIFIER) {
        modifier_t* modifier = (modifier_t*)entry;
        modifier->process_key = function;
    } else if (entry->kind == KIND_UNUSED_MODIFIERS) {
        unused_modifier_t* unused_modifier = (unused_modifier_t*)entry;
        unused_modifier->process_key = function;
    } else if (entry->kind == KIND_UNUSED) {
        unused_t* unused = (unused_t*)entry;
        unused->process_key = function;
    } else if (entry->kind == KIND_BOLD) {
        bold_t* bold = (bold_t*)entry;
        bold->process_key = function;
    } else {
        return false;
    }
    
    return true;
}
