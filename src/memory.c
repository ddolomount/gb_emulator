#include "memory.h"
#include <stdio.h>
#include <string.h>

#define VRAM_MEM_START 0x8000
#define VRAM_MEM_END   0x9FFF

uint8_t memory_read8(Memory_t *mem, uint16_t addr)
{
    if (addr >= VRAM_MEM_START && addr <= VRAM_MEM_END)
    {
        return mem->VRAM[addr - 0x8000];
    }
    else 
    {
        printf("Invalid read at address %02X\n", addr);
        return 0xFF;
    }
}

void memory_write8(Memory_t *mem, uint16_t addr, uint8_t value)
{
    if (addr >= VRAM_MEM_START && addr <= VRAM_MEM_END)
    {
        mem->VRAM[addr - 0x8000] = value;
    }
    else 
    {
        printf("Invalid write to address %02X\n", addr);
    }
}

void memory_init(Memory_t *memory)
{
    memset(memory, 0, sizeof(*memory));
}

void memory_reset(Memory_t *memory)
{
    memory_init(memory);
}
