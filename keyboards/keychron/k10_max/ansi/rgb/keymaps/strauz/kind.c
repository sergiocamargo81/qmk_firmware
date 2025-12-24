#include "kind.h"
#include "keymap.h"  // Para WIN_BASE

// ===== Pools Estáticos =====

// Pool de positions: 10 profiles + 1 numlock + 1 persistence = 12 teclas
// Todos os valores preenchidos estaticamente, exceto function (preenchido pelos módulos)
// Ordenado por row, depois por col
static position_t g_position_pool[POSITION_COUNT] = {
    // Row 1
    {.base = {KIND_POSITION}, .row = 1, .col = 17, .keycode = KC_NUM, .led_index = 36, .supported_keymod = KEYMOD_NONE, .function = NULL},  // KC_NUM
    // Row 2
    {.base = {KIND_POSITION}, .row = 2, .col = 15, .keycode = KC_END, .led_index = 54, .supported_keymod = KEYMOD_FN, .function = NULL},  // KC_END
    {.base = {KIND_POSITION}, .row = 2, .col = 17, .keycode = KC_P7,  .led_index = 56, .supported_keymod = KEYMOD_FN, .function = NULL},  // P7
    {.base = {KIND_POSITION}, .row = 2, .col = 18, .keycode = KC_P8,  .led_index = 57, .supported_keymod = KEYMOD_FN, .function = NULL},  // P8
    {.base = {KIND_POSITION}, .row = 2, .col = 19, .keycode = KC_P9,  .led_index = 58, .supported_keymod = KEYMOD_FN, .function = NULL},  // P9
    // Row 3
    {.base = {KIND_POSITION}, .row = 3, .col = 17, .keycode = KC_P4,  .led_index = 76, .supported_keymod = KEYMOD_FN, .function = NULL},  // P4
    {.base = {KIND_POSITION}, .row = 3, .col = 18, .keycode = KC_P5,  .led_index = 77, .supported_keymod = KEYMOD_FN, .function = NULL},  // P5
    {.base = {KIND_POSITION}, .row = 3, .col = 19, .keycode = KC_P6,  .led_index = 78, .supported_keymod = KEYMOD_FN, .function = NULL},  // P6
    // Row 4
    {.base = {KIND_POSITION}, .row = 4, .col = 17, .keycode = KC_P1,  .led_index = 92, .supported_keymod = KEYMOD_FN, .function = NULL},  // P1
    {.base = {KIND_POSITION}, .row = 4, .col = 18, .keycode = KC_P2,  .led_index = 93, .supported_keymod = KEYMOD_FN, .function = NULL},  // P2
    {.base = {KIND_POSITION}, .row = 4, .col = 19, .keycode = KC_P3,  .led_index = 94, .supported_keymod = KEYMOD_FN, .function = NULL},  // P3
    // Row 5
    {.base = {KIND_POSITION}, .row = 5, .col = 18, .keycode = KC_P0,  .led_index = 106, .supported_keymod = KEYMOD_FN, .function = NULL},  // P0
};

// Pool de customs: 48 teclas (custom_index 0-47)
// Todos os valores preenchidos estaticamente, exceto function (preenchido pelos módulos)
// Ordenado por row, depois por col (mantém custom_index original)
// supported_keymod para custom: máscara de bits aceitando NONE(0x01), FN_RCTL(0x04), FN_RALT(0x08)
#define KEYMOD_CUSTOM_MASK  (KEYMOD_NONE | KEYMOD_FN_RCTL | KEYMOD_FN_RALT)
static custom_t g_custom_pool[CUSTOM_COUNT] = {
    // Row 1
    {.base = {KIND_CUSTOM}, .row = 1, .col = 0,  .keycode = KC_GRV,  .led_index = 19, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 44, .function = NULL},  // 44
    {.base = {KIND_CUSTOM}, .row = 1, .col = 1,  .keycode = KC_1,    .led_index = 20, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 26, .function = NULL},  // 26
    {.base = {KIND_CUSTOM}, .row = 1, .col = 2,  .keycode = KC_2,    .led_index = 21, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 27, .function = NULL},  // 27
    {.base = {KIND_CUSTOM}, .row = 1, .col = 3,  .keycode = KC_3,    .led_index = 22, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 28, .function = NULL},  // 28
    {.base = {KIND_CUSTOM}, .row = 1, .col = 4,  .keycode = KC_4,    .led_index = 23, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 29, .function = NULL},  // 29
    {.base = {KIND_CUSTOM}, .row = 1, .col = 5,  .keycode = KC_5,    .led_index = 24, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 30, .function = NULL},  // 30
    {.base = {KIND_CUSTOM}, .row = 1, .col = 6,  .keycode = KC_6,    .led_index = 25, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 31, .function = NULL},  // 31
    {.base = {KIND_CUSTOM}, .row = 1, .col = 7,  .keycode = KC_7,    .led_index = 26, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 32, .function = NULL},  // 32
    {.base = {KIND_CUSTOM}, .row = 1, .col = 8,  .keycode = KC_8,    .led_index = 27, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 33, .function = NULL},  // 33
    {.base = {KIND_CUSTOM}, .row = 1, .col = 9,  .keycode = KC_9,    .led_index = 28, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 34, .function = NULL},  // 34
    {.base = {KIND_CUSTOM}, .row = 1, .col = 10, .keycode = KC_0,    .led_index = 29, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 35, .function = NULL},  // 35
    {.base = {KIND_CUSTOM}, .row = 1, .col = 11, .keycode = KC_MINS, .led_index = 30, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 37, .function = NULL},  // 37
    {.base = {KIND_CUSTOM}, .row = 1, .col = 12, .keycode = KC_EQL,  .led_index = 31, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 38, .function = NULL},  // 38
    // Row 2
    {.base = {KIND_CUSTOM}, .row = 2, .col = 1,  .keycode = KC_Q,    .led_index = 40, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 16, .function = NULL},  // 16
    {.base = {KIND_CUSTOM}, .row = 2, .col = 2,  .keycode = KC_W,    .led_index = 41, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 22, .function = NULL},  // 22
    {.base = {KIND_CUSTOM}, .row = 2, .col = 3,  .keycode = KC_E,    .led_index = 42, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 4,  .function = NULL},   // 4
    {.base = {KIND_CUSTOM}, .row = 2, .col = 4,  .keycode = KC_R,    .led_index = 43, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 17, .function = NULL},  // 17
    {.base = {KIND_CUSTOM}, .row = 2, .col = 5,  .keycode = KC_T,    .led_index = 44, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 19, .function = NULL},  // 19
    {.base = {KIND_CUSTOM}, .row = 2, .col = 6,  .keycode = KC_Y,    .led_index = 45, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 24, .function = NULL},  // 24
    {.base = {KIND_CUSTOM}, .row = 2, .col = 7,  .keycode = KC_U,    .led_index = 46, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 20, .function = NULL},  // 20
    {.base = {KIND_CUSTOM}, .row = 2, .col = 8,  .keycode = KC_I,    .led_index = 47, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 8,  .function = NULL},   // 8
    {.base = {KIND_CUSTOM}, .row = 2, .col = 9,  .keycode = KC_O,    .led_index = 48, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 14, .function = NULL},  // 14
    {.base = {KIND_CUSTOM}, .row = 2, .col = 10, .keycode = KC_P,    .led_index = 49, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 15, .function = NULL},  // 15
    {.base = {KIND_CUSTOM}, .row = 2, .col = 11, .keycode = KC_LBRC, .led_index = 50, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 39, .function = NULL},  // 39
    {.base = {KIND_CUSTOM}, .row = 2, .col = 12, .keycode = KC_RBRC, .led_index = 51, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 40, .function = NULL},  // 40
    {.base = {KIND_CUSTOM}, .row = 2, .col = 13, .keycode = KC_BSLS, .led_index = 52, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 41, .function = NULL},  // 41
    // Row 3
    {.base = {KIND_CUSTOM}, .row = 3, .col = 1,  .keycode = KC_A,    .led_index = 60, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 0,  .function = NULL},   // 0
    {.base = {KIND_CUSTOM}, .row = 3, .col = 2,  .keycode = KC_S,    .led_index = 61, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 18, .function = NULL},  // 18
    {.base = {KIND_CUSTOM}, .row = 3, .col = 3,  .keycode = KC_D,    .led_index = 62, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 3,  .function = NULL},   // 3
    {.base = {KIND_CUSTOM}, .row = 3, .col = 4,  .keycode = KC_F,    .led_index = 63, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 5,  .function = NULL},   // 5
    {.base = {KIND_CUSTOM}, .row = 3, .col = 5,  .keycode = KC_G,    .led_index = 64, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 6,  .function = NULL},   // 6
    {.base = {KIND_CUSTOM}, .row = 3, .col = 6,  .keycode = KC_H,    .led_index = 65, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 7,  .function = NULL},   // 7
    {.base = {KIND_CUSTOM}, .row = 3, .col = 7,  .keycode = KC_J,    .led_index = 66, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 9,  .function = NULL},   // 9
    {.base = {KIND_CUSTOM}, .row = 3, .col = 8,  .keycode = KC_K,    .led_index = 67, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 10, .function = NULL},  // 10
    {.base = {KIND_CUSTOM}, .row = 3, .col = 9,  .keycode = KC_L,    .led_index = 68, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 11, .function = NULL},  // 11
    {.base = {KIND_CUSTOM}, .row = 3, .col = 10, .keycode = KC_SCLN, .led_index = 69, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 42, .function = NULL},  // 42
    {.base = {KIND_CUSTOM}, .row = 3, .col = 11, .keycode = KC_QUOT, .led_index = 70, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 43, .function = NULL},  // 43
    // Row 4
    {.base = {KIND_CUSTOM}, .row = 4, .col = 2,  .keycode = KC_Z,    .led_index = 80, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 25, .function = NULL},  // 25
    {.base = {KIND_CUSTOM}, .row = 4, .col = 3,  .keycode = KC_X,    .led_index = 81, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 23, .function = NULL},  // 23
    {.base = {KIND_CUSTOM}, .row = 4, .col = 4,  .keycode = KC_C,    .led_index = 82, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 2,  .function = NULL},   // 2
    {.base = {KIND_CUSTOM}, .row = 4, .col = 5,  .keycode = KC_V,    .led_index = 83, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 21, .function = NULL},  // 21
    {.base = {KIND_CUSTOM}, .row = 4, .col = 6,  .keycode = KC_B,    .led_index = 84, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 1,  .function = NULL},   // 1
    {.base = {KIND_CUSTOM}, .row = 4, .col = 7,  .keycode = KC_N,    .led_index = 85, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 13, .function = NULL},  // 13
    {.base = {KIND_CUSTOM}, .row = 4, .col = 8,  .keycode = KC_M,    .led_index = 86, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 12, .function = NULL},  // 12
    {.base = {KIND_CUSTOM}, .row = 4, .col = 9,  .keycode = KC_COMM, .led_index = 87, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 45, .function = NULL},  // 45
    {.base = {KIND_CUSTOM}, .row = 4, .col = 10, .keycode = KC_DOT,  .led_index = 88, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 46, .function = NULL},  // 46
    {.base = {KIND_CUSTOM}, .row = 4, .col = 11, .keycode = KC_SLSH, .led_index = 89, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 47, .function = NULL},  // 47
    // Row 5
    {.base = {KIND_CUSTOM}, .row = 5, .col = 6,  .keycode = KC_SPC,  .led_index = 98, .supported_keymod = KEYMOD_CUSTOM_MASK, .custom_index = 36, .function = NULL},  // 36 (KC_SPC)
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

// Cast seguro para position_t
position_t* kind_get_position(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_POSITION) return NULL;
    return (position_t*)entry;
}

// Cast seguro para custom_t
custom_t* kind_get_custom(uint8_t row, uint8_t col) {
    base_t* entry = kind_get_grid_entry(row, col);
    if (entry == NULL || entry->kind != KIND_CUSTOM) return NULL;
    return (custom_t*)entry;
}

// Obtém custom_t por custom_index (busca linear no pool)
custom_t* kind_get_custom_by_index(uint8_t custom_index) {
    if (custom_index >= CUSTOM_COUNT) return NULL;
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        if (g_custom_pool[i].custom_index == custom_index) {
            return &g_custom_pool[i];
        }
    }
    return NULL;
}


// Itera sobre pool de positions
void kind_iterate_positions(bool (*callback)(position_t* pos, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < POSITION_COUNT; i++) {
        if (!callback(&g_position_pool[i], user_data)) return;
    }
}

// Itera sobre pool de customs
void kind_iterate_customs(bool (*callback)(custom_t* custom, void* user_data), void* user_data) {
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        if (!callback(&g_custom_pool[i], user_data)) return;
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
    
    // Preenche grid com positions (sobrescreve customs se houver colisão)
    for (uint8_t i = 0; i < POSITION_COUNT; i++) {
        position_t* position = &g_position_pool[i];
        if (in_bounds(position->row, position->col)) {
            // Registra no grid (sobrescreve custom se houver colisão)
            g_grid[position->row][position->col] = (base_t*)position;
        }
    }
}

// Registra função para uma entrada do grid
bool kind_register_function(uint8_t row, uint8_t col, key_function_t function) {
    if (!in_bounds(row, col)) return false;
    
    base_t* entry = g_grid[row][col];
    if (entry == NULL) return false;
    
    if (entry->kind == KIND_POSITION) {
        position_t* pos = (position_t*)entry;
        pos->function = function;
    } else if (entry->kind == KIND_CUSTOM) {
        custom_t* custom = (custom_t*)entry;
        custom->function = function;
    } else {
        return false;
    }
    
    return true;
}
