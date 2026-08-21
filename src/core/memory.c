#include "core/memory.h"
#include <stdio.h>
#include <string.h>

#define VRAM_MEM_START 0x8000
#define VRAM_MEM_END   0x9FFF

void memory_init(Memory_t *memory)
{
    memset(memory, 0, sizeof(*memory));
}

void memory_reset(Memory_t *memory)
{
    memory_init(memory);
}
