#include "core/gb.h"
#include "core/bus.h"
#include "core/cpu.h"
#include "core/joypad.h"
#include "core/memory.h"
#include "core/timer.h"
#include "core/ppu.h"

void gb_init(gb_t *gb)
{
    cpu_init(&gb->cpu);
    memory_init(&gb->memory);
    timer_init(&gb->timer);
    ppu_init(&gb->ppu);
    joypad_init(&gb->joypad);
    bus_init(&gb->bus, &gb->memory, &gb->cartridge, &gb->timer, &gb->joypad, &gb->ppu);
}

void gb_step(gb_t *gb)
{
    uint8_t cycles = cpu_step(&gb->cpu, &gb->bus);

    ppu_step(&gb->ppu, cycles);
}

void gb_reset(gb_t *gb)
{
    (void)gb;
}
