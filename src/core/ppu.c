#include <stdio.h>
#include <string.h>

#include "core/memory.h"
#include "core/ppu.h"

#define TILE_BYTE_SIZE 16

static void oam_scan(ppu_t *ppu, uint8_t cycles)
{
    (void)ppu;
    (void)cycles;
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
                extra_cycles = ppu->mode_cycles - 80;
                next_mode    = PPU_MODE_3;
            }
            else
            {
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

                if (ppu->ly < 144)
                {
                    next_mode = PPU_MODE_2;
                }
                else if (ppu->ly >= 144)
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

uint8_t ppu_read(ppu_t *ppu, uint16_t addr)
{
    if (addr >= VRAM_START_ADDR && addr <= VRAM_END_ADDR)
    {
        return ppu->VRAM[addr - VRAM_START_ADDR];
    }
    else if (addr >= OAM_START_ADDR && addr <= OAM_END_ADDR)
    {
        return ppu->OAM[addr - OAM_START_ADDR];
    }
    else
    {
        switch (addr)
        {
            case SCY_ADDR:
            {
                return ppu->scy;
                break;
            }
            case SCX_ADDR:
            {
                return ppu->scx;
                break;
            }
            case LY_ADDR:
            {
                return ppu->ly;
                break;
            }
            case LYC_ADDR:
            {
                return ppu->lyc;
                break;
            }
            case WY_ADDR:
            {
                return ppu->wy;
                break;
            }
            case WX_ADDR:
            {
                return ppu->wx;
                break;
            }
            default:
            {
                printf("Invalid PPU address: %x, in read", addr);
                return 0xFF;
            }
        }
    }
}

void ppu_write(ppu_t *ppu, uint16_t addr, uint8_t value)
{
    if (addr >= VRAM_START_ADDR && addr <= VRAM_END_ADDR)
    {
        ppu->VRAM[addr - VRAM_START_ADDR] = value;
    }
    else if (addr >= OAM_START_ADDR && addr <= OAM_END_ADDR)
    {
        ppu->OAM[addr - OAM_START_ADDR] = value;
    }
    else
    {
        switch (addr)
        {
            case SCY_ADDR:
            {
                ppu->scy = value;
                break;
            }
            case SCX_ADDR:
            {
                ppu->scx = value;
                break;
            }
            case LY_ADDR:
            {
                ppu->ly = value;
                break;
            }
            case LYC_ADDR:
            {
                ppu->lyc = value;
                break;
            }
            case WY_ADDR:
            {
                ppu->wy = value;
                break;
            }
            case WX_ADDR:
            {
                ppu->wx = value;
                break;
            }
            default:
            {
                printf("Invalid PPU address: %x, in write", addr);
                break;
            }
        }
    }
}
