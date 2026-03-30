#ifndef MLCLICK_H
#define MLCLICK_H

#include QMK_KEYBOARD_H
#include "../kind.h"

#define MLCLICK_PRESS_DELAY 500
#define MLCLICK_INTERVAL 50
#define MLCLICK_RESTING_TIME 5000

typedef uint8_t mlclick_state_t;
enum {
    MLCLICK_WAITING,
    MLCLICK_PRESSING,
    MLCLICK_FIRING,
    MLCLICK_RESTING,
    MLCLICK_TOGGLE_FIRING
};

extern mlclick_t* mlclick_position;

void mlclick_register_position(void);
void mlclick_init_early_hooks(void);
void mlclick_init_hooks(void);

#endif // MLCLICK_H
