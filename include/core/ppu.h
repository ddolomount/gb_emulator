#ifndef PPU_H
#define PPU_H

#include <stdbool.h>
#include <stdint.h>

#define GB_WIDTH  160
#define GB_HEIGHT 144

#define LCDC_ADDR 0xFF40
#define STAT_ADDR 0xFF41
#define SCY_ADDR  0xFF42
#define SCX_ADDR  0xFF43
#define LY_ADDR   0xFF44
#define LYC_ADDR  0xFF45
#define BGP_ADDR  0xFF47
#define OBP0_ADDR 0xFF48
#define OBP1_ADDR 0xFF49
#define WY_ADDR   0xFF4A
#define WX_ADDR   0xFF4B

typedef enum
{
    PPU_MODE_0, /* HBlank */
    PPU_MODE_1, /* VBlank */
    PPU_MODE_2, /* OAM Scan */
    PPU_MODE_3, /* Drawing Pixels */
} ppu_mode_t;

typedef struct
{
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t attributes;
    uint8_t oam_index;
} sprite_t;

typedef struct
{
    uint8_t lcdc;
    uint8_t stat;
    uint8_t scy;
    uint8_t scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint8_t wy;
    uint8_t wx;
} ppu_registers_t;

typedef struct ppu
{
    uint8_t VRAM[0x2000];
    uint8_t OAM[0xA0];

    uint32_t framebuffer[GB_HEIGHT][GB_WIDTH];
    ppu_registers_t registers;
    
    ppu_mode_t mode;
    uint16_t mode_cycles;

    sprite_t scanline_sprites[10];
    uint8_t scanline_sprite_count;
    uint8_t scan_index;

    bool frame_ready;
} ppu_t;

void ppu_init(ppu_t *ppu);
bool ppu_frame_ready(ppu_t *ppu);
uint32_t *ppu_get_framebuffer(ppu_t *ppu);
void ppu_step(ppu_t *ppu, uint8_t cycles);
uint8_t ppu_read(const ppu_t *ppu, uint16_t addr);
void ppu_write(ppu_t *ppu, uint16_t addr, uint8_t value);

#endif // !PPU_H
