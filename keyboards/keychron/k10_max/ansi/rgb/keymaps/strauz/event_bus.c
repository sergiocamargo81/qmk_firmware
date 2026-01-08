#include "event_bus.h"

#include <string.h>

// ===== Estruturas Internas =====

// Array de handlers por tipo de evento
// g_handlers[event_type][index] = handler
static event_handler_t g_handlers[EVENT_COUNT][EVENT_BUS_MAX_HANDLERS] = {NULL};

// Contador de handlers por tipo de evento
static uint8_t g_handler_counts[EVENT_COUNT] = {0};

// Flag de inicialização
static bool g_initialized = false;

// ===== API Principal =====

void event_bus_init(void) {
    if (g_initialized) return;
    
    // Limpa todos os arrays
    memset(g_handlers, 0, sizeof(g_handlers));
    memset(g_handler_counts, 0, sizeof(g_handler_counts));
    
    g_initialized = true;
}

bool event_bus_subscribe(event_type_t type, event_handler_t handler) {
    if (!g_initialized) {
        event_bus_init();
    }
    
    if (type >= EVENT_COUNT || handler == NULL) {
        return false;
    }
    
    // Verifica se já está registrado
    for (uint8_t i = 0; i < g_handler_counts[type]; i++) {
        if (g_handlers[type][i] == handler) {
            return false; // Já registrado
        }
    }
    
    // Verifica limite
    if (g_handler_counts[type] >= EVENT_BUS_MAX_HANDLERS) {
        return false; // Limite atingido
    }
    
    // Adiciona ao array
    g_handlers[type][g_handler_counts[type]] = handler;
    g_handler_counts[type]++;
    
    return true;
}

bool event_bus_unsubscribe(event_type_t type, event_handler_t handler) {
    if (type >= EVENT_COUNT || handler == NULL) {
        return false;
    }
    
    // Procura o handler no array
    for (uint8_t i = 0; i < g_handler_counts[type]; i++) {
        if (g_handlers[type][i] == handler) {
            // Remove movendo os elementos seguintes uma posição para trás
            for (uint8_t j = i; j < g_handler_counts[type] - 1; j++) {
                g_handlers[type][j] = g_handlers[type][j + 1];
            }
            g_handlers[type][g_handler_counts[type] - 1] = NULL;
            g_handler_counts[type]--;
            return true;
        }
    }
    
    return false; // Não encontrado
}

bool event_bus_publish(event_type_t type, const event_data_t* data) {
    if (type >= EVENT_COUNT) {
        return false;
    }
    
    // Cria evento
    event_t event = {
        .type = type,
        .data = data ? *data : (event_data_t){{0}}
    };
    
    // Chama todos os handlers registrados
    if (g_handler_counts[type] == 0) {
        return false; // Nenhum handler registrado
    }
    
    bool at_least_one_called = false;
    for (uint8_t i = 0; i < g_handler_counts[type]; i++) {
        if (g_handlers[type][i] != NULL) {
            g_handlers[type][i](&event);
            at_least_one_called = true;
        }
    }
    
    return at_least_one_called;
}

// ===== API de Conveniência =====

bool event_bus_subscribe_rgb_indicators(event_handler_t handler) {
    if (handler == NULL) return false;
    return event_bus_subscribe(EVENT_RGB_INDICATORS, handler);
}

bool event_bus_subscribe_profile_changed(event_handler_t handler) {
    if (handler == NULL) return false;
    return event_bus_subscribe(EVENT_PROFILE_CHANGED, handler);
}

bool event_bus_subscribe_fn_state_changed(event_handler_t handler) {
    if (handler == NULL) return false;
    return event_bus_subscribe(EVENT_FN_STATE_CHANGED, handler);
}

// ===== Debug/Utilidades =====

uint8_t event_bus_get_handler_count(event_type_t type) {
    if (type >= EVENT_COUNT) {
        return 0;
    }
    return g_handler_counts[type];
}

const char* event_bus_get_event_name(event_type_t type) {
    switch (type) {
        case EVENT_KEYBOARD_POST_INIT: return "KEYBOARD_POST_INIT";
        case EVENT_MATRIX_SCAN: return "MATRIX_SCAN";
        case EVENT_RGB_INDICATORS: return "RGB_INDICATORS";
        case EVENT_EECONFIG_INIT: return "EECONFIG_INIT";
        case EVENT_SETTINGS_LOADED: return "SETTINGS_LOADED";
        case EVENT_PROFILE_CHANGED: return "PROFILE_CHANGED";
        case EVENT_FN_STATE_CHANGED: return "FN_STATE_CHANGED";
        case EVENT_CUSTOM_BEHAVIOR_ADDED: return "CUSTOM_BEHAVIOR_ADDED";
        case EVENT_CUSTOM_BEHAVIOR_REMOVED: return "CUSTOM_BEHAVIOR_REMOVED";
        case EVENT_CUSTOM_BEHAVIOR_CHANGED: return "CUSTOM_BEHAVIOR_CHANGED";
        case EVENT_COUNT: return "COUNT";
        default: return "UNKNOWN";
    }
}
