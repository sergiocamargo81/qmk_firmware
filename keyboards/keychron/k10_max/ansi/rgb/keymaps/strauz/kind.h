#ifndef KIND_H
#define KIND_H

#include QMK_KEYBOARD_H
#include "keymod.h"      // Para keymod_t
#include "keyboard.h"     // Para keypos_t
#include "action.h"       // Para keyrecord_t
#include "custom_behaviors.h"  // Para custom_behaviors_t

// ===== Forward Declarations =====

// Forward declaration de base_key_t para process_key_t
typedef struct base_key_t base_key_t;

// Tipo de função para processar eventos (baseado em posição, não keycode)
// Recebe: ponteiro para a struct da key, se está pressionada, e o keymod atual
typedef bool (*process_key_t)(base_key_t* key, bool pressed, keymod_t keymod);

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
    KIND_DISABLED_MODIFIERS
} kind_t;

// Header comum (deve ser o primeiro membro para dispatch)
typedef struct base_key_t {
    kind_t kind;                // READONLY Tipo da tecla
    uint8_t row;                // READONLY
    uint8_t col;                // READONLY
    uint16_t keycode;           // READONLY Keycode da layer BASE
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos
    process_key_t process_key;  // Handler da posição (NULL se não tiver)
    uint8_t state;              // Estado do módulo (uint8_t)
} base_key_t;

// ===== Tipos Concretos =====

// Para teclas de profile: 10 teclas (P0-P9)
// Campos relacionados ao base_key_t (podem ser acessados via cast para base_key_t*)
typedef struct {
    kind_t kind;                // READONLY Tipo da tecla (relacionado ao base)
    uint8_t row;                // READONLY (relacionado ao base)
    uint8_t col;                // READONLY (relacionado ao base)
    uint16_t keycode;           // READONLY Keycode da layer BASE (relacionado ao base)
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever) (relacionado ao base)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos (relacionado ao base)
    process_key_t process_key;  // Handler da posição (NULL se não tiver) (relacionado ao base)
    uint8_t state;              // Estado do módulo (uint8_t) (relacionado ao base)
    uint8_t profile_index;      // READONLY Índice do profile (0-9)
} profile_key_t;

// Para tecla numlock: 1 tecla (KC_NUM)
// Campos relacionados ao base_key_t (podem ser acessados via cast para base_key_t*)
typedef struct {
    kind_t kind;                // READONLY Tipo da tecla (relacionado ao base)
    uint8_t row;                // READONLY (relacionado ao base)
    uint8_t col;                // READONLY (relacionado ao base)
    uint16_t keycode;           // READONLY Keycode da layer BASE (relacionado ao base)
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever) (relacionado ao base)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos (relacionado ao base)
    process_key_t process_key;  // Handler da posição (NULL se não tiver) (relacionado ao base)
    uint8_t state;              // Estado do módulo (uint8_t) (relacionado ao base)
} numlock_t;

// Para tecla persistence: 1 tecla (KC_END)
// Campos relacionados ao base_key_t (podem ser acessados via cast para base_key_t*)
typedef struct {
    kind_t kind;                // READONLY Tipo da tecla (relacionado ao base)
    uint8_t row;                // READONLY (relacionado ao base)
    uint8_t col;                // READONLY (relacionado ao base)
    uint16_t keycode;           // READONLY Keycode da layer BASE (relacionado ao base)
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever) (relacionado ao base)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos (relacionado ao base)
    process_key_t process_key;  // Handler da posição (NULL se não tiver) (relacionado ao base)
    uint8_t state;              // Estado do módulo (uint8_t) (relacionado ao base)
} persistence_key_t;

// Para teclas custom: 48 teclas (persist_index 0-47)
// Campos relacionados ao base_key_t (podem ser acessados via cast para base_key_t*)
typedef struct {
    kind_t kind;                // READONLY Tipo da tecla (relacionado ao base)
    uint8_t row;                // READONLY (relacionado ao base)
    uint8_t col;                // READONLY (relacionado ao base)
    uint16_t keycode;           // READONLY Keycode da layer BASE (relacionado ao base)
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever) (relacionado ao base)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos (relacionado ao base)
    process_key_t process_key;  // Handler da posição (NULL se não tiver) (relacionado ao base)
    uint8_t state;              // Estado do módulo (uint8_t) (relacionado ao base)
    uint8_t persist_index;       // READONLY Índice persistente (0-47)
    uint32_t timer;             // Timer para hold/toggle (uint32_t)
    custom_behaviors_t custom_behavior; // Cache do behavior atual (atualizado quando profile muda)
} custom_t;

// Para teclas desabilitadas: teclas que não são position nem custom
// Campos relacionados ao base_key_t (podem ser acessados via cast para base_key_t*)
typedef struct {
    kind_t kind;                // READONLY Tipo da tecla (relacionado ao base)
    uint8_t row;                // READONLY (relacionado ao base)
    uint8_t col;                // READONLY (relacionado ao base)
    uint16_t keycode;           // READONLY Keycode da layer BASE (relacionado ao base)
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever) (relacionado ao base)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos (relacionado ao base)
    process_key_t process_key; // Handler da posição (NULL se não tiver) (relacionado ao base)
    uint8_t state;              // Estado do módulo (uint8_t) (relacionado ao base)
} disabled_t;

// Para teclas modificadoras: RSHIFT, RALT, FN, RCTRL
// Campos relacionados ao base_key_t (podem ser acessados via cast para base_key_t*)
typedef struct {
    kind_t kind;                // READONLY Tipo da tecla (relacionado ao base)
    uint8_t row;                // READONLY (relacionado ao base)
    uint8_t col;                // READONLY (relacionado ao base)
    uint16_t keycode;           // READONLY Keycode da layer BASE (relacionado ao base)
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever) (relacionado ao base)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos (relacionado ao base)
    process_key_t process_key;  // Handler da posição (NULL se não tiver) (relacionado ao base)
    uint8_t state;              // Estado do módulo (uint8_t) (relacionado ao base)
} modifier_t;

// Para teclas bold: ESC, ENTER, BACKSPACE
// Campos relacionados ao base_key_t (podem ser acessados via cast para base_key_t*)
typedef struct {
    kind_t kind;                // READONLY Tipo da tecla (relacionado ao base)
    uint8_t row;                // READONLY (relacionado ao base)
    uint8_t col;                // READONLY (relacionado ao base)
    uint16_t keycode;           // READONLY Keycode da layer BASE (relacionado ao base)
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever) (relacionado ao base)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos (relacionado ao base)
    process_key_t process_key;  // Handler da posição (NULL se não tiver) (relacionado ao base)
    uint8_t state;              // Estado do módulo (uint8_t) (relacionado ao base)
} bold_t;

// Para teclas modificadoras desabilitadas: TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN
// Campos relacionados ao base_key_t (podem ser acessados via cast para base_key_t*)
typedef struct {
    kind_t kind;                // READONLY Tipo da tecla (relacionado ao base)
    uint8_t row;                // READONLY (relacionado ao base)
    uint8_t col;                // READONLY (relacionado ao base)
    uint16_t keycode;           // READONLY Keycode da layer BASE (relacionado ao base)
    uint8_t led_index;          // READONLY LED index (0-127, nunca sobrescrever) (relacionado ao base)
    keymod_t accepted_keymods;  // READONLY Flags de keymod aceitos (relacionado ao base)
    process_key_t process_key;  // Handler da posição (NULL se não tiver) (relacionado ao base)
    uint8_t state;              // Estado do módulo (uint8_t) (relacionado ao base)
} others_t;

// ===== Defines =====

#define PROFILE_COUNT     10  // 10 profiles (P0-P9)
#define NUMLOCK_COUNT     1   // 1 numlock (KC_NUM)
#define PERSISTENCE_COUNT 1   // 1 persistence (KC_END)
#define CUSTOM_COUNT      48  // 48 teclas custom (persist_index 0-47)
#define DISABLED_COUNT    34  // Teclas que não são profile, numlock, persistence, custom, modifier, bold nem disabled_modifiers
#define MODIFIER_COUNT    4   // RSHIFT, RALT, FN, RCTRL
#define BOLD_COUNT        3   // ESC, ENTER, BACKSPACE
#define DISABLED_MODIFIERS_COUNT       7   // Disabled modifiers: TAB, LSFT(KC_TAB), LSHIFT, LCONTROL, LWIN, LALT, RWIN

// ===== API de Acesso =====

// Obtém entrada do grid (O(1))
base_key_t* kind_get_grid_entry(uint8_t row, uint8_t col);

// Obtém a tecla numlock (retorna diretamente do pool, sem precisar de posição)
numlock_t* kind_get_numlock_key(void);

// Obtém a tecla persistence (retorna diretamente do pool, sem precisar de posição)
persistence_key_t* kind_get_persistence_key(void);

// Cast seguro para custom_t
custom_t* kind_get_custom(uint8_t row, uint8_t col);

// Obtém custom_t por persist_index
custom_t* kind_get_custom_by_index(uint8_t persist_index);

// Obtém teclas modificadoras diretamente do pool (sem precisar de posição)
modifier_t* kind_get_rsft_key(void);  // Right Shift
modifier_t* kind_get_ralt_key(void);  // Right Alt
modifier_t* kind_get_fn_key(void);    // Function key
modifier_t* kind_get_rctl_key(void);  // Right Control

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

// Itera sobre pool de disabled_modifiers
void kind_iterate_disabled_modifiers(bool (*callback)(others_t* disabled_modifier, void* user_data), void* user_data);

// Itera sobre todas as entradas do grid
void kind_iterate_grid(bool (*callback)(base_key_t* entry, void* user_data), void* user_data);

// ===== Inicialização =====

// Preenche grid com ponteiros para os pools e led_index
void kind_init_grid(void);

// Registra função para uma entrada do grid
bool kind_register_function(uint8_t row, uint8_t col, process_key_t function);

#endif // KIND_H
