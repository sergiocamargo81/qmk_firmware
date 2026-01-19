#include QMK_KEYBOARD_H

#include "leds.h"
#include "../event_bus.h"
#include "../custom_behaviors.h"
#include "../settings.h"
#include "../profile.h"
#include <stdint.h>

// ===== Defines =====

// Tempos de timeout em milissegundos
#define LEDS_TO_STOPPING_MS  (60 * 1000)   // 1 minuto para stopping
#define LEDS_TO_SLEEPING_MS  (5 * 60 * 1000) // 5 minutos para sleeping

// ===== Estados Internos =====

typedef enum {
    LEDS_ACTIVE,     // LEDs normais (cores módulos ou padrão baseado no profile)
    LEDS_STOPPING,   // 1min+: apenas cores padrão do teclado
    LEDS_SLEEPING    // 5min+: LEDs desligados
} leds_state_t;

// ===== Callbacks do Event Bus =====

// Callback para evento de atividade detectada
static void leds_activity_callback(const event_t* event) {
    if (event->type == EVENT_ACTIVITY_DETECTED) {
        leds_notify_activity();
    }
}

// Callback para evento matrix_scan
static void leds_matrix_scan_callback(const event_t* event) {
    if (event->type == EVENT_MATRIX_SCAN) {
        leds_matrix_scan();
    }
}

// ===== Variáveis Globais =====

// Estado atual do sistema de LEDs
static leds_state_t g_leds_state = LEDS_ACTIVE;

// Timestamp da última atividade (em ms desde boot)
static uint32_t g_last_activity_time = 0;

// Flag para controlar se o módulo foi inicializado
static bool g_leds_initialized = false;

// ===== Funções Internas =====

// Verifica se o profile ativo está vazio
static bool profile_is_empty(void) {
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile) return true;

    for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
        if (active_profile->behaviors[i] != CUSTOM_BEHAVIOR_UNASSOCIATED) {
            return false;
        }
    }
    return true;
}

// Atualiza o estado dos LEDs baseado no tempo de inatividade
static void update_leds_state(void) {
    if (!g_leds_initialized) {
        return;
    }

    // Usa timer_elapsed32 para evitar problemas de overflow
    uint32_t idle_time = timer_elapsed32(g_last_activity_time);

    leds_state_t new_state = LEDS_ACTIVE;

    // Determina o novo estado baseado no tempo de inatividade
    if (idle_time >= LEDS_TO_SLEEPING_MS) {
        new_state = LEDS_SLEEPING;
    } else if (idle_time >= LEDS_TO_STOPPING_MS) {
        new_state = LEDS_STOPPING;
    }

    // Só atualiza se o estado mudou
    if (new_state != g_leds_state) {
        g_leds_state = new_state;
    }
}

// ===== API Principal =====

// Inicializa o módulo de controle de LEDs
void leds_init(void) {
    g_leds_state = LEDS_ACTIVE;
    g_last_activity_time = timer_read32();
    g_leds_initialized = true;
}

// Registra hooks QMK para o módulo
void leds_init_hooks(void) {
    // Registra callback para detectar atividade do teclado
    event_bus_subscribe_activity_detected(leds_activity_callback);
}

// Registra hooks que podem ser executados antes da inicialização completa
void leds_init_early_hooks(void) {
    // Registra callback para processamento periódico
    event_bus_subscribe(EVENT_MATRIX_SCAN, leds_matrix_scan_callback);
}

// Callback para notificar atividade do teclado
void leds_notify_activity(void) {
    if (!g_leds_initialized) {
        return;
    }

    g_last_activity_time = timer_read32();

    // Se estava em estado não-active, volta para active
    if (g_leds_state != LEDS_ACTIVE) {
        g_leds_state = LEDS_ACTIVE;
    }
}


// Retorna se deve exibir cores dos módulos ou apenas cores padrão
// true = exibir cores dos módulos, false = apenas cores padrão
bool leds_should_show_modules(void) {
    if (!g_leds_initialized) {
        return true; // Comportamento padrão
    }

    // Apenas no estado ACTIVE e com profile não vazio deve mostrar módulos
    return (g_leds_state == LEDS_ACTIVE) && !profile_is_empty();
}

// Processamento periódico (chamado em matrix_scan_user)
void leds_matrix_scan(void) {
    update_leds_state();

    // Gerenciar estado do RGB matrix baseado no estado atual
    // Isso garante que funciona mesmo quando RGB matrix está desabilitado
    if (g_leds_state == LEDS_SLEEPING) {
        if (rgb_matrix_is_enabled()) {
            rgb_matrix_disable();
        }
    } else {
        if (!rgb_matrix_is_enabled()) {
            rgb_matrix_enable();
        }
    }
}