#ifndef LEDS_H
#define LEDS_H

// ===== API Principal =====

// Inicializa o módulo de controle de LEDs
void leds_init(void);

// Registra hooks QMK para o módulo
// Deve ser chamado durante keyboard_post_init_user
void leds_init_hooks(void);

// Registra hooks que podem ser executados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void leds_init_early_hooks(void);

// Callback para notificar atividade do teclado
// Deve ser chamado quando qualquer tecla é pressionada ou liberada
void leds_notify_activity(void);


// Retorna se deve exibir cores dos módulos ou apenas cores padrão
// true = exibir cores dos módulos, false = apenas cores padrão
bool leds_should_show_modules(void);

// Processamento periódico (chamado em matrix_scan_user)
void leds_matrix_scan(void);

#endif // LEDS_H