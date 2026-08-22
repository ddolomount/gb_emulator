#include <string.h>

#include "core/joypad.h"

#define SELECT_BUTTONS (1 << 5)
#define SELECT_D_PAD (1 << 4)

void joypad_init(joypad_t *joypad)
{
    memset(joypad, 0, sizeof(*joypad));
}

void joypad_set_state(joypad_t *joypad, const gb_joypad_state_t state)
{
    joypad->joypad_state = state;
}

uint8_t joypad_read(joypad_t *joypad)
{
    // Bits 7-6 not used (reads as 1)
    uint8_t joyp = 0xCF;

    // Isolate lower nibble
    joyp &= ~0x30;
    joyp |= joypad->select_bits;

    // Assumes that a true in the joypad_state_t is being pressed (active low in register)
    if ((joypad->select_bits & SELECT_D_PAD) == 0)
    {
        joyp &= ~(joypad->joypad_state.right << 0);
        joyp &= ~(joypad->joypad_state.left << 1);
        joyp &= ~(joypad->joypad_state.up << 2);
        joyp &= ~(joypad->joypad_state.down << 3);
    }

    if ((joypad->select_bits & SELECT_BUTTONS) == 0)
    {
        joyp &= ~(joypad->joypad_state.a << 0);
        joyp &= ~(joypad->joypad_state.b << 1);
        joyp &= ~(joypad->joypad_state.select << 2);
        joyp &= ~(joypad->joypad_state.start << 3);
    }

    return joyp;
}

void joypad_write(joypad_t *joypad, uint8_t value)
{
    joypad->select_bits = value & 0x30;
}
