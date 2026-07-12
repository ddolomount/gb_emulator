#ifndef GB_H
#define GB_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "core/memory.h"
#include "core/cpu.h"
#include "core/cartridge.h"
#include "core/bus.h"

// GameBoy struct
typedef struct {
    cpu_t cpu;
    bus_t bus;
    Memory_t mem;
    Cartridge_t cartridge;
} gb_t;

uint8_t gb_mem_read(gb_t *gb, const uint16_t addr);
uint8_t gb_mem_write(gb_t *gb, const uint16_t addr, const uint8_t val);

void gb_init();
void gb_reset();

void gb_load_rom();

void gb_step();
void gb_run_frame();



#endif /* GB_H */
