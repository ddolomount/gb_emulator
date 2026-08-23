#include <string.h>

#include "core/ppu.h"

#define TILE_BYTE_SIZE 16

void ppu_init(ppu_t *ppu)
{
    (void)ppu;

    /*
     * PPU starts in OAM Scan mode
     */
    memset(ppu->framebuffer, 0, sizeof(*ppu));

    ppu->mode        = PPU_MODE_2;
    ppu->frame_ready = false;
}

bool ppu_frame_ready(ppu_t *ppu)
{
    return ppu->frame_ready;
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
                next_mode = PPU_MODE_3;
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
                next_mode = PPU_MODE_0;
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
    ppu->mode = next_mode;
}
