#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include QMK_KEYBOARD_H
#include <stdbool.h>
#include <stdint.h>

// ===== Defines =====

// Número máximo de handlers por tipo de evento
#define EVENT_BUS_MAX_HANDLERS 16

// ===== Tipos de Eventos =====

typedef enum {
    // ===== Eventos de Sistema =====
    EVENT_KEYBOARD_POST_INIT,      // Executado após inicialização do teclado
    EVENT_MATRIX_SCAN,             // Executado periodicamente no loop principal
    EVENT_RGB_INDICATORS,          // Executado durante renderização RGB
    EVENT_EECONFIG_INIT,           // Executado quando EEPROM é inicializada
    
    // ===== Eventos de Settings/Profile =====
    EVENT_SETTINGS_LOADED,         // Settings carregados da EEPROM
    EVENT_PROFILE_CHANGED,         // Profile ativo mudou (old_profile, new_profile)

    // ===== Eventos de Modifiers =====
    EVENT_FN_STATE_CHANGED,        // Estado de FN mudou (keymod_t)
    
    // ===== Eventos de Custom Behaviors =====
    EVENT_CUSTOM_BEHAVIOR_ADDED,   // Behavior foi adicionado a uma tecla (row, col, behavior)
    EVENT_CUSTOM_BEHAVIOR_REMOVED, // Behavior foi removido de uma tecla (row, col, behavior)
    EVENT_CUSTOM_BEHAVIOR_CHANGED, // Behavior de tecla mudou (row, col, old_behavior, new_behavior)
    
    // ===== Contador (deve ser o último) =====
    EVENT_COUNT
} event_type_t;

// ===== Estruturas de Dados dos Eventos =====

// Dados para EVENT_PROFILE_CHANGED
typedef struct {
    void* old_profile;  // profile_t* (void* para evitar dependência circular)
    void* new_profile;  // profile_t* (void* para evitar dependência circular)
} event_profile_changed_data_t;

// Dados para EVENT_FN_STATE_CHANGED
typedef struct {
    uint16_t keymod;  // keymod_t (uint16_t para evitar dependência circular)
} event_fn_state_changed_data_t;

// Dados para EVENT_CUSTOM_BEHAVIOR_ADDED/REMOVED
typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t behavior;  // custom_behaviors_t (uint8_t para evitar dependência circular)
} event_custom_behavior_data_t;

// Dados para EVENT_CUSTOM_BEHAVIOR_CHANGED
typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t old_behavior;  // custom_behaviors_t (uint8_t para evitar dependência circular)
    uint8_t new_behavior;  // custom_behaviors_t (uint8_t para evitar dependência circular)
} event_custom_behavior_changed_data_t;

// Union de dados de eventos
typedef union {
    event_profile_changed_data_t profile_changed;
    event_fn_state_changed_data_t fn_state_changed;
    event_custom_behavior_data_t custom_behavior;
    event_custom_behavior_changed_data_t custom_behavior_changed;
    // Para eventos sem dados, usar campo vazio
} event_data_t;

// Estrutura de evento
typedef struct {
    event_type_t type;
    event_data_t data;
} event_t;

// ===== Tipos de Handlers =====

// Handler genérico para eventos
typedef void (*event_handler_t)(const event_t* event);

// ===== API Principal =====

// Registra um handler para um tipo de evento
// Retorna true se sucesso, false se limite atingido ou handler inválido
bool event_bus_subscribe(event_type_t type, event_handler_t handler);

// Remove um handler registrado
// Retorna true se encontrado e removido, false caso contrário
bool event_bus_unsubscribe(event_type_t type, event_handler_t handler);

// Publica um evento (dispara todos os handlers registrados)
// Retorna true se pelo menos um handler foi chamado
bool event_bus_publish(event_type_t type, const event_data_t* data);

// Publica um evento sem dados
static inline bool event_bus_publish_void(event_type_t type) {
    event_data_t empty = {0};
    return event_bus_publish(type, &empty);
}

// ===== API de Conveniência =====

// Registra handler para rgb_indicators
bool event_bus_subscribe_rgb_indicators(event_handler_t handler);

// Registra handler para profile_changed
bool event_bus_subscribe_profile_changed(event_handler_t handler);

// Registra handler para fn_state_changed
bool event_bus_subscribe_fn_state_changed(event_handler_t handler);

// ===== Inicialização =====

// Inicializa o event bus (deve ser chamado antes de qualquer uso)
void event_bus_init(void);

// ===== Debug/Utilidades =====

// Retorna o número de handlers registrados para um tipo de evento
uint8_t event_bus_get_handler_count(event_type_t type);

// Retorna o nome do tipo de evento (para debug)
const char* event_bus_get_event_name(event_type_t type);

#endif // EVENT_BUS_H
