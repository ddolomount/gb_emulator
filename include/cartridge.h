#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define ROM_START_ADDR 0x0000
#define ROM_END_ADDR   0x7FFF

typedef struct {
    uint8_t *rom;
    size_t rom_size;
} Cartridge_t;

bool cartridge_load(Cartridge_t *cart, const char *path);

#endif /* CARTRIDGE_H */
