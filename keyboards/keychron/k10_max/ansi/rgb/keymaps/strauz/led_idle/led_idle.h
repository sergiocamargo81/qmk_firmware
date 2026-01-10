#ifndef LED_IDLE_H
#define LED_IDLE_H

#include <stdint.h>
#include <stdbool.h>

// ===== Defines =====

// Tempos de timeout em milissegundos
#define LED_IDLE_TO_DEFAULT_MS  (60 * 1000)   // 1 minuto para cores padrão
#define LED_IDLE_OFF_MS         (5 * 60 * 1000) // 5 minutos para desligar

// Estados do sistema de LEDs
typedef enum {
    LED_STATE_CUSTOM,      // LEDs customizados (estado normal)
    LED_STATE_DEFAULT,     // LEDs com cores padrão do teclado
    LED_STATE_OFF          // LEDs desligados
} led_idle_state_t;

// ===== API Principal =====

// Inicializa o módulo de idle de LEDs
void led_idle_init(void);

// Registra hooks QMK para o módulo
// Deve ser chamado durante keyboard_post_init_user
void led_idle_init_hooks(void);

// Registra hooks que podem ser executados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void led_idle_init_early_hooks(void);

// Callback para notificar atividade do teclado
// Deve ser chamado quando qualquer tecla é pressionada ou liberada
void led_idle_notify_activity(void);

// Retorna o estado atual do sistema de LEDs
led_idle_state_t led_idle_get_state(void);

// Processamento periódico (chamado em matrix_scan_user)
void led_idle_matrix_scan(void);

#endif // LED_IDLE_H