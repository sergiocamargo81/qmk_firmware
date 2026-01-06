#ifndef PULSE_H
#define PULSE_H

#include <stdint.h>

// Função compartilhada para calcular brilho pulsante
// Retorna valor de 0-255 baseado em seno do tempo
// Pulsação: 1 segundo por ciclo, brilho varia de 30% a 100%
uint8_t calculate_pulse_brightness(void);

// Função para calcular efeito de piscar (alterna entre duas cores)
// Retorna true se deve mostrar a cor de piscar, false para cor anterior
// Usa g_rgb_timer para alternar a cada ~500ms
bool calculate_blink_state(void);

// Função para calcular pulsação rápida entre duas cores
// Retorna valor de 0-255: 0 = primeira cor, 255 = segunda cor
// Pulsação rápida: ~0.5 segundos por ciclo completo
// Usa seno para transição suave entre as cores
uint8_t calculate_dual_color_pulse(void);

#endif // PULSE_H
