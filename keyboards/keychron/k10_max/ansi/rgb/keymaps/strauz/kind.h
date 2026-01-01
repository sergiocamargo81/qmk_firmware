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
    KIND_POSITION,
    KIND_CUSTOM,
    KIND_DISABLED,
    KIND_MODIFIER
} kind_t;

// Header comum (deve ser o primeiro membro para dispatch)
typedef struct {
    kind_t kind;
} base_t;

// ===== Tipos Concretos =====

// Para teclas especiais: profiles (10), numlock (1), persistence (1) = 12 teclas
typedef struct {
    base_t   base;               // Primeiro membro para dispatch
    uint8_t  row;
    uint8_t  col;
    uint16_t keycode;            // Keycode da layer BASE
    uint8_t  led_index;          // LED index (NO_LED se não tiver)
    keymod_t supported_keymod;   // Enum único de keymod suportado
    key_function_t function;     // Handler da posição (NULL se não tiver)
} position_t;

// Para teclas custom: 48 teclas (custom_index 0-47)
typedef struct {
    base_t   base;               // Primeiro membro para dispatch
    uint8_t  row;
    uint8_t  col;
    uint16_t keycode;            // Keycode da layer BASE
    uint8_t  led_index;          // LED index (NO_LED se não tiver)
    keymod_t supported_keymod;   // Enum único de keymod suportado
    uint8_t  custom_index;       // Índice custom (0-47)
    key_function_t function;     // Handler da posição (NULL se não tiver)
} custom_t;

// Para teclas desabilitadas: teclas que não são position nem custom
typedef struct {
    base_t   base;               // Primeiro membro para dispatch
    uint8_t  row;
    uint8_t  col;
    uint16_t keycode;            // Keycode da layer BASE
    uint8_t  led_index;          // LED index (NO_LED se não tiver)
} disabled_t;

// Para teclas modificadoras: RSHIFT, RALT, FN, RCTRL
typedef struct {
    base_t   base;               // Primeiro membro para dispatch
    uint8_t  row;
    uint8_t  col;
    uint16_t keycode;            // Keycode da layer BASE
    uint8_t  led_index;          // LED index (NO_LED se não tiver)
} modifier_t;

// ===== Defines =====

#define POSITION_COUNT 12  // 10 profiles + 1 numlock + 1 persistence
#define CUSTOM_COUNT   48  // 48 teclas custom (custom_index 0-47)
#define DISABLED_COUNT 44  // Teclas que não são position, custom nem modifier
#define MODIFIER_COUNT 4   // RSHIFT, RALT, FN, RCTRL

// ===== API de Acesso =====

// Obtém entrada do grid (O(1))
base_t* kind_get_grid_entry(uint8_t row, uint8_t col);

// Cast seguro para position_t
position_t* kind_get_position(uint8_t row, uint8_t col);

// Cast seguro para custom_t
custom_t* kind_get_custom(uint8_t row, uint8_t col);

// Obtém custom_t por custom_index
custom_t* kind_get_custom_by_index(uint8_t custom_index);

// Cast seguro para disabled_t
disabled_t* kind_get_disabled(uint8_t row, uint8_t col);

// Cast seguro para modifier_t
modifier_t* kind_get_modifier(uint8_t row, uint8_t col);


// Itera sobre pool de positions
void kind_iterate_positions(bool (*callback)(position_t* pos, void* user_data), void* user_data);

// Itera sobre pool de customs
void kind_iterate_customs(bool (*callback)(custom_t* custom, void* user_data), void* user_data);

// Itera sobre pool de disabled
void kind_iterate_disabled(bool (*callback)(disabled_t* disabled, void* user_data), void* user_data);

// Itera sobre pool de modifiers
void kind_iterate_modifiers(bool (*callback)(modifier_t* modifier, void* user_data), void* user_data);

// Itera sobre todas as entradas do grid
void kind_iterate_grid(bool (*callback)(base_t* entry, void* user_data), void* user_data);

// ===== Inicialização =====

// Preenche grid com ponteiros para os pools e led_index
void kind_init_grid(void);

// Registra função para uma entrada do grid
bool kind_register_function(uint8_t row, uint8_t col, key_function_t function);

#endif // KIND_H
