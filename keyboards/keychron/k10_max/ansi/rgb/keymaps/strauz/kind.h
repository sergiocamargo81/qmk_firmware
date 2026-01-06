#ifndef KIND_H
#define KIND_H

#include QMK_KEYBOARD_H
#include "keymod.h"      // Para keymod_t
#include "keyboard.h"     // Para keypos_t
#include "action.h"       // Para keyrecord_t

// ===== Forward Declarations =====

// Tipo de função para processar eventos (baseado em posição, não keycode)
typedef bool (*key_function_t)(keyrecord_t *record, keymod_t keymod);

// ===== Tipos Base =====

typedef enum {
    KIND_NONE = 0,
    KIND_PROFILE,
    KIND_NUMLOCK,
    KIND_PERSISTENCE,
    KIND_CUSTOM,
    KIND_DISABLED,
    KIND_MODIFIER,
    KIND_BOLD,
    KIND_OTHER
} kind_t;

// Header comum (deve ser o primeiro membro para dispatch)
typedef struct {
    kind_t kind;
} base_t;

// ===== Tipos Concretos =====

// Para teclas de profile: 10 teclas (P0-P9)
typedef struct {
    base_t   base;               // READONLY Primeiro membro para dispatch
    uint8_t  row;                // READONLY
    uint8_t  col;                // READONLY
    uint16_t keycode;            // READONLY Keycode da layer BASE
    uint8_t  led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    keymod_t accepted_keymods;   // READONLY Flags de keymod aceitos
    key_function_t function;     // Handler da posição (NULL se não tiver)
    uint8_t  state;              // Estado do módulo (uint8_t)
} profile_key_t;

// Para tecla numlock: 1 tecla (KC_NUM)
typedef struct {
    base_t   base;               // READONLY Primeiro membro para dispatch
    uint8_t  row;                // READONLY
    uint8_t  col;                // READONLY
    uint16_t keycode;            // READONLY Keycode da layer BASE
    uint8_t  led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    keymod_t accepted_keymods;   // READONLY Flags de keymod aceitos
    key_function_t function;     // Handler da posição (NULL se não tiver)
    uint8_t  state;              // Estado do módulo (uint8_t)
} numlock_t;

// Para tecla persistence: 1 tecla (KC_END)
typedef struct {
    base_t   base;               // READONLY Primeiro membro para dispatch
    uint8_t  row;                // READONLY
    uint8_t  col;                // READONLY
    uint16_t keycode;            // READONLY Keycode da layer BASE
    uint8_t  led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    keymod_t accepted_keymods;   // READONLY Flags de keymod aceitos
    key_function_t function;     // Handler da posição (NULL se não tiver)
    uint8_t  state;              // Estado do módulo (uint8_t)
} persistence_key_t;

// Para teclas custom: 48 teclas (persist_index 0-47)
typedef struct {
    base_t   base;               // READONLY Primeiro membro para dispatch
    uint8_t  row;                // READONLY
    uint8_t  col;                // READONLY
    uint16_t keycode;            // READONLY Keycode da layer BASE
    uint8_t  led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    keymod_t accepted_keymods;   // READONLY Flags de keymod aceitos
    uint8_t  persist_index;      // READONLY Índice persistente (0-47)
    key_function_t function;     // Handler da posição (NULL se não tiver)
    uint8_t  state;              // Estado do módulo (uint8_t)
    uint32_t timer;              // Timer para hold/toggle (uint32_t)
} custom_t;

// Para teclas desabilitadas: teclas que não são position nem custom
typedef struct {
    base_t   base;               // READONLY Primeiro membro para dispatch
    uint8_t  row;                // READONLY
    uint8_t  col;                // READONLY
    uint16_t keycode;            // READONLY Keycode da layer BASE
    uint8_t  led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    key_function_t function;     // Handler da posição (NULL se não tiver)
    uint8_t  state;              // Estado do módulo (uint8_t)
} disabled_t;

// Para teclas modificadoras: RSHIFT, RALT, FN, RCTRL
typedef struct {
    base_t   base;               // READONLY Primeiro membro para dispatch
    uint8_t  row;                // READONLY
    uint8_t  col;                // READONLY
    uint16_t keycode;            // READONLY Keycode da layer BASE
    uint8_t  led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    key_function_t function;     // Handler da posição (NULL se não tiver)
    uint8_t  state;              // Estado do módulo (uint8_t)
} modifier_t;

// Para teclas bold: ESC, ENTER, BACKSPACE
typedef struct {
    base_t   base;               // READONLY Primeiro membro para dispatch
    uint8_t  row;                // READONLY
    uint8_t  col;                // READONLY
    uint16_t keycode;            // READONLY Keycode da layer BASE
    uint8_t  led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    key_function_t function;     // Handler da posição (NULL se não tiver)
    uint8_t  state;              // Estado do módulo (uint8_t)
} bold_t;

// Para outras teclas modificadoras: TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN
typedef struct {
    base_t   base;               // READONLY Primeiro membro para dispatch
    uint8_t  row;                // READONLY
    uint8_t  col;                // READONLY
    uint16_t keycode;            // READONLY Keycode da layer BASE
    uint8_t  led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    key_function_t function;     // Handler da posição (NULL se não tiver)
    uint8_t  state;              // Estado do módulo (uint8_t)
} others_t;

// ===== Defines =====

#define PROFILE_COUNT     10  // 10 profiles (P0-P9)
#define NUMLOCK_COUNT     1   // 1 numlock (KC_NUM)
#define PERSISTENCE_COUNT 1   // 1 persistence (KC_END)
#define CUSTOM_COUNT      48  // 48 teclas custom (persist_index 0-47)
#define DISABLED_COUNT    34  // Teclas que não são profile, numlock, persistence, custom, modifier, bold nem other
#define MODIFIER_COUNT    4   // RSHIFT, RALT, FN, RCTRL
#define BOLD_COUNT        3   // ESC, ENTER, BACKSPACE
#define OTHER_COUNT       7   // TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN

// ===== API de Acesso =====

// Obtém entrada do grid (O(1))
base_t* kind_get_grid_entry(uint8_t row, uint8_t col);

// Cast seguro para profile_key_t
profile_key_t* kind_get_profile(uint8_t row, uint8_t col);

// Cast seguro para numlock_t
numlock_t* kind_get_numlock(uint8_t row, uint8_t col);

// Cast seguro para persistence_key_t
persistence_key_t* kind_get_persistence(uint8_t row, uint8_t col);

// Cast seguro para custom_t
custom_t* kind_get_custom(uint8_t row, uint8_t col);

// Obtém custom_t por persist_index
custom_t* kind_get_custom_by_index(uint8_t persist_index);

// Cast seguro para disabled_t
disabled_t* kind_get_disabled(uint8_t row, uint8_t col);

// Cast seguro para modifier_t
modifier_t* kind_get_modifier(uint8_t row, uint8_t col);

// Cast seguro para bold_t
bold_t* kind_get_bold(uint8_t row, uint8_t col);

// Cast seguro para others_t
others_t* kind_get_other(uint8_t row, uint8_t col);

// Itera sobre pool de profiles
void kind_iterate_profiles(bool (*callback)(profile_key_t* profile, void* user_data), void* user_data);

// Itera sobre pool de numlock
void kind_iterate_numlock(bool (*callback)(numlock_t* numlock, void* user_data), void* user_data);

// Itera sobre pool de persistence
void kind_iterate_persistence(bool (*callback)(persistence_key_t* persistence, void* user_data), void* user_data);

// Itera sobre pool de customs
void kind_iterate_customs(bool (*callback)(custom_t* custom, void* user_data), void* user_data);

// Itera sobre pool de disabled
void kind_iterate_disabled(bool (*callback)(disabled_t* disabled, void* user_data), void* user_data);

// Itera sobre pool de modifiers
void kind_iterate_modifiers(bool (*callback)(modifier_t* modifier, void* user_data), void* user_data);

// Itera sobre pool de bold
void kind_iterate_bold(bool (*callback)(bold_t* bold, void* user_data), void* user_data);

// Itera sobre pool de other
void kind_iterate_other(bool (*callback)(others_t* other, void* user_data), void* user_data);

// Itera sobre todas as entradas do grid
void kind_iterate_grid(bool (*callback)(base_t* entry, void* user_data), void* user_data);

// ===== Inicialização =====

// Preenche grid com ponteiros para os pools e led_index
void kind_init_grid(void);

// Registra função para uma entrada do grid
bool kind_register_function(uint8_t row, uint8_t col, key_function_t function);

#endif // KIND_H
