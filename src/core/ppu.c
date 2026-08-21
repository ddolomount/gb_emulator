#include <string.h>

#include "core/ppu.h"
#include "core/bus.h"
#include "core/memory.h"

#define TILE_BYTE_SIZE 16

typedef enum
{
    PPU_MODE_0, /* HBlank */
    PPU_MODE_1, /* VBlank */
    PPU_MODE_2, /* OAM Scan */
    PPU_MODE_3, /* Drawing Pixels */
} ppu_state_t;

static ppu_state_t ppu_state;

void ppu_init(ppu_t *ppu)
{
    (void)ppu;

    /*
     * PPU starts in OAM Scan mode
     */
    ppu_state = PPU_MODE_2;

    memset(ppu->framebuffer, 0, sizeof(ppu->framebuffer));
}

void ppu_fsm(ppu_t *ppu)
{
    (void)ppu;

    ppu_state_t next_state = PPU_MODE_2;

    switch (ppu_state)
    {
        case PPU_MODE_0:
        {
            break;
        }
        case PPU_MODE_1:
        {
            break;
        }
        case PPU_MODE_2:
        {
            break;
        }
        case PPU_MODE_3:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    ppu_state = next_state;
}

void read_tile_data(ppu_t *ppu, bus_t *bus)
{
    (void)ppu;

    /*
     * Each tile takes up 16 bytes
     */
    for (int i = 0; i < TILE_BYTE_SIZE; i++)
    {
        uint8_t tile_data = bus_read8(bus, VRAM_START_ADDR + i);

    }
}
