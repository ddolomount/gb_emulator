#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// Memory arrays
typedef struct {
    uint8_t VRAM[0x2000];
    uint8_t WRAM[0x2000];
    uint8_t OAM[0xA0];
    uint8_t HRAM[0x7F];
    uint8_t ie; 
} Memory_t;

uint8_t memory_read8(Memory_t *mem, uint16_t addr);
void memory_write8(Memory_t *mem, uint16_t addr, uint8_t value);

#endif /* MEMORY_H */
