#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdbool.h>
#include <stdint.h>

#define JOYP_ADDR 0xFF00

typedef struct
{
    bool a;
    bool b;
    bool start;
    bool select;
    bool up;
    bool down;
    bool left;
    bool right;
} gb_joypad_state_t;

typedef struct
{
    gb_joypad_state_t joypad_state;
    uint8_t select_bits;
} joypad_t;

void joypad_init(joypad_t *joypad);

void joypad_set_state(joypad_t *joypad, const gb_joypad_state_t state);
uint8_t joypad_read(joypad_t *joypad);
void joypad_write(joypad_t *joypad, uint8_t value);

#endif /* JOYPAD_H */
