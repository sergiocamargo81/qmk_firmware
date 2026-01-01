#include "pulse.h"
#include "rgb_matrix.h"           // Para g_rgb_timer
#include <lib/lib8tion/lib8tion.h> // Para sin8, scale8

// Função compartilhada para calcular brilho pulsante
// Retorna valor de 0-255 baseado em seno do tempo
// Pulsação otimizada: 2x mais rápida que a versão anterior (~1.5 segundos por ciclo)
uint8_t calculate_pulse_brightness(void) {
    // Usa g_rgb_timer para criar efeito de pulsação
    // scale16by8 multiplica por 10 (ao invés de 5) para pulsação 2x mais rápida (~1.5 segundos por ciclo)
    // Versão anterior: scale16by8(time, 5) = ~3 segundos por ciclo
    // Versão atual: scale16by8(time, 10) = ~1.5 segundos por ciclo (2x mais rápido)
    uint16_t time = scale16by8((uint16_t)(g_rgb_timer & 0xFFFF), 10);
    // sin8 retorna 0-255, onde 128 é o meio
    // Ajusta para que o mínimo seja 10% e máximo seja 100%
    uint8_t sine = sin8((uint8_t)time);
    // Escala de 26 (10% de 255) a 255 (100%)
    return scale8(sine, 229) + 26;
}
