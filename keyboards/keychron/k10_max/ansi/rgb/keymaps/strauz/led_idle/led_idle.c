#include "led_idle.h"
#include "../event_bus.h"
#include <stdint.h>

// ===== Callbacks do Event Bus =====

// Callback para evento de atividade detectada
static void led_idle_activity_user(const event_t* event) {
    if (event->type == EVENT_ACTIVITY_DETECTED) {
        led_idle_notify_activity();
    }
}

// Callback para evento matrix_scan
static void led_idle_matrix_scan_user(const event_t* event) {
    if (event->type == EVENT_MATRIX_SCAN) {
        led_idle_matrix_scan();
    }
}

// ===== Variáveis Globais =====

// Estado atual do sistema de LEDs
static led_idle_state_t g_led_state = LED_STATE_CUSTOM;

// Timestamp da última atividade (em ms desde boot)
static uint32_t g_last_activity_time = 0;

// Flag para controlar se o módulo foi inicializado
static bool g_led_idle_initialized = false;

// ===== Funções Internas =====

// Atualiza o estado dos LEDs baseado no tempo de inatividade
static void update_led_state(void) {
    if (!g_led_idle_initialized) {
        return;
    }

    uint32_t current_time = timer_read32();
    uint32_t idle_time = current_time - g_last_activity_time;

    led_idle_state_t new_state = LED_STATE_CUSTOM;

    // Determina o novo estado baseado no tempo de inatividade
    if (idle_time >= LED_IDLE_OFF_MS) {
        new_state = LED_STATE_OFF;
    } else if (idle_time >= LED_IDLE_TO_DEFAULT_MS) {
        new_state = LED_STATE_DEFAULT;
    }

    // Só atualiza se o estado mudou
    if (new_state != g_led_state) {
        g_led_state = new_state;
        // Nota: A mudança visual será aplicada na próxima chamada de rgb_matrix_indicators_user
    }
}

// ===== API Principal =====

// Inicializa o módulo de idle de LEDs
void led_idle_init(void) {
    g_led_state = LED_STATE_CUSTOM;
    g_last_activity_time = timer_read32();
    g_led_idle_initialized = true;
}

// Registra hooks QMK para o módulo
void led_idle_init_hooks(void) {
    // Registra callback para detectar atividade do teclado
    event_bus_subscribe_activity_detected(led_idle_activity_user);
}

// Registra hooks que podem ser executados antes da inicialização completa
void led_idle_init_early_hooks(void) {
    // Registra callback para processamento periódico
    event_bus_subscribe(EVENT_MATRIX_SCAN, led_idle_matrix_scan_user);
}

// Callback para notificar atividade do teclado
void led_idle_notify_activity(void) {
    if (!g_led_idle_initialized) {
        return;
    }

    g_last_activity_time = timer_read32();

    // Se estava em estado não-custom, volta para custom
    if (g_led_state != LED_STATE_CUSTOM) {
        g_led_state = LED_STATE_CUSTOM;
    }
}

// Retorna o estado atual do sistema de LEDs
led_idle_state_t led_idle_get_state(void) {
    return g_led_state;
}

// Processamento periódico (chamado em matrix_scan_user)
void led_idle_matrix_scan(void) {
    update_led_state();
}