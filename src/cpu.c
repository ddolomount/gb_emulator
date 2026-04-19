#include "../include/cpu.h"


void cpu_init(cpu_t *cpu)
{
    cpu->a = 0;
    cpu->f = 0;
    cpu->b = 0;
    cpu->c = 0;
    cpu->d = 0;
    cpu->e = 0;
    cpu->h = 0;
    cpu->l = 0;
}

uint8_t cpu_step(cpu_t *cpu, bus_t *bus)
{
    uint8_t cycle_count = 0;

    // TODO: Fetch, Decode, and Execute instruction

    return cycle_count;
}
