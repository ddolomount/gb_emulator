#include "core/gb.h"
#include "core/bus.h"
#include "core/cpu.h"
#include "core/memory.h"
#include "core/timer.h"

void gb_init(gb_t *gb)
{
    cpu_init(&gb->cpu);
    bus_init(&gb->bus, &gb->memory, &gb->cartridge, &gb->timer);
    memory_init(&gb->memory);
    timer_init(&gb->timer);
}
