#ifndef PPU_H
#define PPU_H

#include <stdint.h>

#define GB_WIDTH 160
#define GB_HEIGHT 144

typedef struct
{
    uint32_t framebuffer[GB_HEIGHT][GB_WIDTH];
} ppu_t;

void ppu_init(ppu_t *ppu);

#endif // !PPU_H
