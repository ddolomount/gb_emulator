#include "../include/timer.h"
#include "../include/bus.h"

#define REG_DIV  0xFF04
#define REG_TIMA 0xFF05
#define REG_TMA  0xFF06
#define REG_TAC  0xFF07
#define REG_IF   0xFF0F

#define TIMER_INTERRUPT_BIT 0x04

static uint32_t timer_period_cycles(uint8_t tac)
{
    /* Return the number of cpu cycles before
     * incrementing the tima_counter
     */
    switch (tac & 0x03)
    {
        case 0x00: return 1024; // 4096 Hz
        case 0x01: return 16;   // 262144 Hz
        case 0x02: return 64;   // 65536 Hz
        case 0x03: return 256;  // 16384 Hz
        default: return 1024;
    }
}

void timer_init(gb_timer_t *timer)
{
    timer->div_counter = 0;
    timer->tima_counter = 0;

    /* Set registers to 0 */
    timer->tima = 0;
    timer->tma  = 0;
    timer->tac  = 0;
}

void timer_tick(gb_timer_t *timer, bus_t *bus, uint8_t cycles)
{
    /* Increment internal counters */
    timer->div_counter += cycles;

    /* Only increment if TAC bit 2 is set */
    if ((timer->tac &0x04) == 0)
    {
        return;
    }
    timer->tima_counter += cycles;

    uint32_t period = timer_period_cycles(timer->tac);

    while (timer->tima_counter >= period)
    {
        timer->tima_counter -= period;

        if (timer->tima == 0xFF)
        {
            timer->tima = timer->tma;

            uint8_t iflag = bus_read8(bus, REG_IF);
            bus_write8(bus, REG_IF, iflag | TIMER_INTERRUPT_BIT);
        }
        else 
        {
            timer->tima++;
        }
    }
}

uint8_t timer_read8(gb_timer_t *timer, uint16_t addr)
{
    switch (addr)
    {
        case REG_DIV:
        {
            return (uint8_t)(timer->div_counter >> 8);
        }
        case REG_TIMA:
        {
            return timer->tima;
        }
        case REG_TMA:
        {
            return timer->tma;
        }
        case REG_TAC:
        {
            return timer->tac | 0xF8;
        }
        default:
        {
            return 0xFF;
        }
    }
}

void timer_write8(gb_timer_t *timer, uint16_t addr, uint8_t value)
{
    switch (addr)
    {
        case REG_DIV:
        {
            timer->div_counter = 0;
            break;
        }
        case REG_TIMA:
        {
            timer->tima = value;
            break;
        }
        case REG_TMA:
        {
            timer->tma = value;
            break;
        }
        case REG_TAC:
        {
            timer->tac = (value & 0x07);
            break;
        }
        default:
        {
            return;
        }
    }
}
