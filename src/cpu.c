#include <string.h>
#include "../include/cpu.h"


void cpu_init(cpu_t *cpu)
{
    // Set to 0 for now
    memset(cpu, 0, sizeof(cpu_t));   
}

uint8_t cpu_step(cpu_t *cpu, bus_t *bus)
{
    uint8_t cycle_count = 0;

    // TODO: Fetch, Decode, and Execute instruction
  
    // Fetch opcode for instruction
    uint8_t opcode = bus_read8(bus, cpu->pc++);

    // Decode instruction and execute
    switch (opcode) 
    {
        case 0x00:
        {
            cycle_count = 4;
            break;
        }
    }

    // Return number of cycles required for instruction
    return cycle_count;
}
