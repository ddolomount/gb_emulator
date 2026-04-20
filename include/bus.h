#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "cartridge.h"

typedef struct {
    Memory_t *memory;
    Cartridge_t *cartridge;
} bus_t;

void bus_init(bus_t *bus, Memory_t *memory, Cartridge_t *cartridge);

uint8_t bus_read8(bus_t *bus, uint16_t addr);
void bus_write8(bus_t *bus, uint16_t addr, uint8_t value);

uint16_t bus_read16(bus_t *bus, uint16_t addr);
void bus_write16(bus_t *bus, uint16_t addr, uint16_t value);

#endif /* BUS_H */
