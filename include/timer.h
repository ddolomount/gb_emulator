#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef struct bus bus_t;

typedef struct
{
    uint16_t div_counter;
    uint32_t tima_counter;

    uint8_t tima; // 0xFF05
    uint8_t tma;  // 0xFF06
    uint8_t tac;  // 0xFF07
} gb_timer_t;

void timer_init(gb_timer_t *timer);
void timer_tick(gb_timer_t *timer, bus_t *bus, uint8_t cycles);

uint8_t timer_read8(gb_timer_t *timer, uint16_t addr);
void timer_write8(gb_timer_t *timer, uint16_t addr, uint8_t value);

#endif /* TIMER_H */
