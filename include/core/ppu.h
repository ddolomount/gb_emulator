#ifndef PPU_H
#define PPU_H

#include <stdbool.h>
#include <stdint.h>

#define GB_WIDTH  160
#define GB_HEIGHT 144

#define SCY_ADDR 0xFF42
#define SCX_ADDR 0xFF43
#define LY_ADDR  0xFF44
#define LYC_ADDR 0xFF45
#define WY_ADDR  0xFF4A
#define WX_ADDR  0xFF4B

typedef enum
{
    PPU_MODE_0, /* HBlank */
    PPU_MODE_1, /* VBlank */
    PPU_MODE_2, /* OAM Scan */
    PPU_MODE_3, /* Drawing Pixels */
} ppu_mode_t;

typedef struct
{
    uint32_t framebuffer[GB_HEIGHT][GB_WIDTH];

    ppu_mode_t mode;

    uint16_t mode_cycles;
    uint8_t ly;

    bool frame_ready;
} ppu_t;

void ppu_init(ppu_t *ppu);
bool ppu_frame_ready(ppu_t *ppu);
void ppu_step(ppu_t *ppu, uint8_t cycles);

#endif // !PPU_H
