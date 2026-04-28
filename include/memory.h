#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define VRAM_START_ADDR 0x8000
#define VRAM_END_ADDR 0x9FFF

#define WRAM_START_ADDR 0xC000
#define WRAM_END_ADDR 0xDFFF

#define OAM_START_ADDR 0xFE00
#define OAM_END_ADDR 0xFE9F

#define IO_START_ADDR 0xFF00
#define IO_END_ADDR 0xFF7F

#define HRAM_START_ADDR 0xFF80
#define HRAM_END_ADDR 0xFFFE

#define IE_ADDR 0xFFFF

// Memory arrays
typedef struct {
    uint8_t VRAM[0x2000];
    uint8_t WRAM[0x2000];
    uint8_t OAM[0xA0];
    uint8_t IO[0x80];
    uint8_t HRAM[0x7F];
    uint8_t ie; 
} Memory_t;

void memory_init(Memory_t *memory);
void memory_reset(Memory_t *memory);

#endif /* MEMORY_H */
