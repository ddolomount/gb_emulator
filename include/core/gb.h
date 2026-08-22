#ifndef GB_H
#define GB_H

#include <stdint.h>
#include <stdbool.h>

#include "core/memory.h"
#include "core/cpu.h"
#include "core/cartridge.h"
#include "core/bus.h"
#include "core/timer.h"
#include "core/ppu.h"
#include "core/joypad.h"

// GameBoy struct
typedef struct {
    cpu_t cpu;
    bus_t bus;
    Memory_t memory;
    Cartridge_t cartridge;
    gb_timer_t timer;
    ppu_t ppu;
    joypad_t joypad;
} gb_t;

uint8_t gb_mem_read(gb_t *gb, const uint16_t addr);
uint8_t gb_mem_write(gb_t *gb, const uint16_t addr, const uint8_t val);

void gb_init(gb_t *gb);
void gb_reset(gb_t *gb);

void gb_step(gb_t *gb);

bool gb_frame_ready(gb_t *gb);

void gb_load_rom();

#endif /* GB_H */
