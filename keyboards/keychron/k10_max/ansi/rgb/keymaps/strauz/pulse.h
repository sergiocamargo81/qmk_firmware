#ifndef PULSE_H
#define PULSE_H

#include <stdint.h>

// Função compartilhada para calcular brilho pulsante
// Retorna valor de 0-255 baseado em seno do tempo
// Pulsação otimizada: 2x mais rápida que a versão anterior (~1.5 segundos por ciclo)
uint8_t calculate_pulse_brightness(void);

#endif // PULSE_H
