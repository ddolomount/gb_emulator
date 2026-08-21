#include "core/gb.h"
#include "core/bus.h"
#include "core/cpu.h"
#include "core/memory.h"
#include "core/timer.h"
#include "core/ppu.h"

void gb_init(gb_t *gb)
{
    cpu_init(&gb->cpu);
    bus_init(&gb->bus, &gb->memory, &gb->cartridge, &gb->timer);
    memory_init(&gb->memory);
    timer_init(&gb->timer);
    ppu_init(&gb->ppu);
}

void gb_step(gb_t *gb)
{
    (void)gb;
}

void gb_reset(gb_t *gb)
{
    (void)gb;
}
