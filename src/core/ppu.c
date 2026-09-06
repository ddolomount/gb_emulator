#include <stdio.h>
#include <string.h>

#include "core/memory.h"
#include "core/ppu.h"

#define TILE_BYTE_SIZE 16
#define NUM_OF_OBJECTS 40

static bool sprite_overlaps_scanline(ppu_t *ppu, uint8_t y)
{
    /*
     * LCDC.2 == 0 8x8 mode
     * LCDC.2 == 1 8x16 mode
     */
    int sprite_y = (int)y - 16;

    int height = 8;

    if ((ppu->registers.lcdc & (1 << 2)) != 0)
    {
        height = 16;
    }

    return (ppu->registers.ly >= sprite_y && ppu->registers.ly < (sprite_y + height));
}

static void oam_scan(ppu_t *ppu, uint8_t cycles)
{
    uint8_t scan_iterations = (uint8_t)(cycles / 2);

    uint8_t index = ppu->scan_index;

    for (int i = index; (i < scan_iterations) && (i < NUM_OF_OBJECTS); i++)
    {
        uint8_t offset = index * 4;

        uint8_t y          = ppu->OAM[offset + 0];
        uint8_t x          = ppu->OAM[offset + 1];
        uint8_t tile_index = ppu->OAM[offset + 2];
        uint8_t attributes = ppu->OAM[offset + 3];

        if (sprite_overlaps_scanline(ppu, y))
        {
            if (ppu->scanline_sprite_count < 10)
            {
                sprite_t *sprite   = &ppu->scanline_sprites[ppu->scanline_sprite_count];
                sprite->y          = y;
                sprite->x          = x;
                sprite->tile       = tile_index;
                sprite->attributes = attributes;
                sprite->oam_index  = index;

                ppu->scanline_sprite_count++;
            }
        }

        ppu->scan_index++;
    }
}

void ppu_init(ppu_t *ppu)
{
    memset(ppu->framebuffer, 0, sizeof(*ppu->framebuffer));

    ppu->mode        = PPU_MODE_2;
    ppu->frame_ready = false;
}

bool ppu_frame_ready(ppu_t *ppu)
{
    return ppu->frame_ready;
}

uint32_t *ppu_get_framebuffer(ppu_t *ppu)
{
    return &ppu->framebuffer[0][0];
}

void ppu_step(ppu_t *ppu, uint8_t cycles)
{
    ppu->mode_cycles += cycles;
    uint8_t extra_cycles = 0;
    ppu_mode_t next_mode;

    switch (ppu->mode)
    {
        case PPU_MODE_2:
        {
            if (ppu->mode_cycles >= 80)
            {
                oam_scan(ppu, 80 - (ppu->mode_cycles - cycles));
                extra_cycles = ppu->mode_cycles - 80;
                next_mode    = PPU_MODE_3;
            }
            else
            {
                oam_scan(ppu, cycles);
                next_mode = PPU_MODE_2;
            }
            break;
        }
        case PPU_MODE_3:
        {
            if (ppu->mode_cycles >= 172)
            {
                extra_cycles = ppu->mode_cycles - 172;
                next_mode    = PPU_MODE_0;
            }
            else
            {
                next_mode = PPU_MODE_3;
            }
            break;
        }
        case PPU_MODE_0:
        {
            if (ppu->mode_cycles >= 376)
            {
                extra_cycles = ppu->mode_cycles - 376;

                if (ppu->registers.ly < 144)
                {
                    next_mode = PPU_MODE_2;
                }
                else if (ppu->registers.ly >= 144)
                {
                    next_mode = PPU_MODE_1;
                }
            }
            else
            {
                next_mode = PPU_MODE_0;
            }
            break;
        }
        case PPU_MODE_1:
        {
            if (ppu->mode_cycles >= 4560)
            {
                extra_cycles = ppu->mode_cycles - 4560;

                next_mode = PPU_MODE_2;
            }
            else
            {
                next_mode = PPU_MODE_1;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    ppu->mode_cycles = extra_cycles;
    ppu->mode        = next_mode;
}

uint8_t ppu_read(const ppu_t *ppu, uint16_t addr)
{
    // TODO: Enforce CPU-visible PPU access rules
    if (addr >= VRAM_START_ADDR && addr <= VRAM_END_ADDR)
    {
        return ppu->VRAM[addr - VRAM_START_ADDR];
    }

    if (addr >= OAM_START_ADDR && addr <= OAM_END_ADDR)
    {
        return ppu->OAM[addr - OAM_START_ADDR];
    }

    switch (addr)
    {
        case LCDC_ADDR:
        {
            return ppu->registers.lcdc;
        }
        case STAT_ADDR:
        {
            return ppu->registers.stat;
        }
        case SCY_ADDR:
        {
            return ppu->registers.scy;
        }
        case SCX_ADDR:
        {
            return ppu->registers.scx;
        }
        case LY_ADDR:
        {
            return ppu->registers.ly;
        }
        case LYC_ADDR:
        {
            return ppu->registers.lyc;
        }
        case BGP_ADDR:
        {
            return ppu->registers.bgp;
        }
        case OBP0_ADDR:
        {
            return ppu->registers.obp0;
        }
        case OBP1_ADDR:
        {
            return ppu->registers.obp1;
        }
        case WY_ADDR:
        {
            return ppu->registers.wy;
        }
        case WX_ADDR:
        {
            return ppu->registers.wx;
        }
        default:
        {
            return 0xFF;
        }
    }
}

void ppu_write(ppu_t *ppu, uint16_t addr, uint8_t value)
{
    if (addr >= VRAM_START_ADDR && addr <= VRAM_END_ADDR)
    {
        ppu->VRAM[addr - VRAM_START_ADDR] = value;
    }

    if (addr >= OAM_START_ADDR && addr <= OAM_END_ADDR)
    {
        ppu->OAM[addr - OAM_START_ADDR] = value;
    }

    switch (addr)
    {
        case LCDC_ADDR:
        {
            ppu->registers.lcdc = value;
            break;
        }
        case SCY_ADDR:
        {
            ppu->registers.scy = value;
            break;
        }
        case SCX_ADDR:
        {
            ppu->registers.scx = value;
            break;
        }
        case LY_ADDR:
        {
            ppu->registers.ly = value;
            break;
        }
        case LYC_ADDR:
        {
            ppu->registers.lyc = value;
            break;
        }
        case BGP_ADDR:
        {
            ppu->registers.bgp = value;
            break;
        }
        case OBP0_ADDR:
        {
            ppu->registers.obp0 = value;
            break;
        }
        case OBP1_ADDR:
        {
            ppu->registers.obp1 = value;
            break;
        }
        case WY_ADDR:
        {
            ppu->registers.wy = value;
            break;
        }
        case WX_ADDR:
        {
            ppu->registers.wx = value;
            break;
        }
        default:
        {
            break;
        }
    }
}
