#include "pulse.h"
#include "rgb_matrix.h"           // Para g_rgb_timer
#include <lib/lib8tion/lib8tion.h> // Para sin8, scale8

// Função compartilhada para calcular brilho pulsante
// Retorna valor de 0-255 baseado em seno do tempo
// Pulsação: 1 segundo por ciclo, brilho varia de 30% a 100%
uint8_t calculate_pulse_brightness(void) {
    // Usa g_rgb_timer para criar efeito de pulsação
    // scale16by8 multiplica por 15 para pulsação de 1 segundo por ciclo
    // 1 segundo = ~1000ms, g_rgb_timer incrementa a ~16ms, então ~62 ciclos por segundo
    // Para 1 ciclo completo em 1 segundo: scale16by8(time, 15) ≈ 1 segundo
    uint16_t time = scale16by8((uint16_t)(g_rgb_timer & 0xFFFF), 15);
    // sin8 retorna 0-255, onde 128 é o meio
    // Ajusta para que o mínimo seja 30% e máximo seja 100%
    uint8_t sine = sin8((uint8_t)time);
    // Escala de 77 (30% de 255) a 255 (100%)
    // 255 - 77 = 178 (range de 178 valores)
    return scale8(sine, 178) + 77;
}

// Função para calcular efeito de piscar (alterna entre duas cores)
// Retorna true se deve mostrar a cor de piscar, false para cor anterior
// Usa g_rgb_timer para alternar a cada ~500ms
bool calculate_blink_state(void) {
    // Usa bit 6 do timer para alternar (~500ms por ciclo)
    // g_rgb_timer incrementa a ~16ms, então bit 6 alterna a cada 64 ticks ≈ 1 segundo
    // Usamos bit 7 para alternar mais rápido (~500ms)
    return (g_rgb_timer & (1 << 7)) != 0;
}

// Função para calcular pulsação rápida entre duas cores
// Retorna valor de 0-255: 0 = primeira cor, 255 = segunda cor
// Pulsação rápida: ~0.5 segundos por ciclo completo
// Usa seno para transição suave entre as cores
uint8_t calculate_dual_color_pulse(void) {
    // Usa g_rgb_timer para criar efeito de pulsação rápida
    // scale16by8 multiplica por 30 para pulsação de 0.5 segundos por ciclo
    // 0.5 segundo = ~500ms, g_rgb_timer incrementa a ~16ms, então ~31 ciclos por segundo
    // Para 1 ciclo completo em 0.5 segundo: scale16by8(time, 30) ≈ 0.5 segundo
    uint16_t time = scale16by8((uint16_t)(g_rgb_timer & 0xFFFF), 30);
    // sin8 retorna 0-255, onde 128 é o meio
    // Normaliza para 0-255 onde 0 = primeira cor e 255 = segunda cor
    uint8_t sine = sin8((uint8_t)time);
    // Ajusta para que o mínimo seja 0 e máximo seja 255
    // sin8 já retorna 0-255, então podemos usar diretamente
    return sine;
}
