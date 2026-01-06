#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

// ===== Enum de Cores =====

typedef enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_WHITE,
    COLOR_PURPLE,
    COLOR_ORANGE_BURNT,  // Laranja queimado (mais vermelho)
    COLOR_OFF,
    COLOR_COUNT
} color_t;

// ===== Estrutura RGB =====

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_rgb_t;

// ===== Array de Cores =====

static const color_rgb_t color_rgb[COLOR_COUNT] = {
    [COLOR_RED] = {.r = 255, .g = 0,   .b = 0},
    [COLOR_GREEN] = {.r = 0,   .g = 255, .b = 0},
    [COLOR_BLUE] = {.r = 0,   .g = 0,   .b = 255},
    [COLOR_YELLOW] = {.r = 255, .g = 255, .b = 0},
    [COLOR_WHITE] = {.r = 255, .g = 255, .b = 255},
    [COLOR_PURPLE] = {.r = 128, .g = 0,   .b = 128},
    [COLOR_ORANGE_BURNT] = {.r = 255, .g = 69, .b = 0},   // Laranja queimado (mais vermelho)
    [COLOR_OFF] = {.r = 0,   .g = 0,   .b = 0},
};

// ===== Funções Auxiliares =====

// Obtém RGB de uma cor
static inline color_rgb_t color_get_rgb(color_t color) {
    if (color >= COLOR_COUNT) {
        return color_rgb[COLOR_OFF];
    }
    return color_rgb[color];
}

// Obtém componentes RGB individuais
static inline uint8_t color_get_r(color_t color) {
    return color_get_rgb(color).r;
}

static inline uint8_t color_get_g(color_t color) {
    return color_get_rgb(color).g;
}

static inline uint8_t color_get_b(color_t color) {
    return color_get_rgb(color).b;
}

// Aplica brilho a uma cor (para pulsação)
static inline color_rgb_t color_apply_brightness(color_t color, uint8_t brightness) {
    color_rgb_t rgb = color_get_rgb(color);
    rgb.r = (rgb.r * brightness) / 255;
    rgb.g = (rgb.g * brightness) / 255;
    rgb.b = (rgb.b * brightness) / 255;
    return rgb;
}

#endif // COLORS_H
