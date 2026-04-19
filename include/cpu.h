#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "bus.h"

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

// CPU related registers
typedef struct {
    uint8_t a, f; // Accumulator & flags
    uint8_t b, c; 
    uint8_t d, e; 
    uint8_t h, l; 
    uint16_t sp;  // Stack Pointer
    uint16_t pc;  // Program Counter/Pointer
} cpu_t;

/*
 * Initialize CPU and set all registers to default values
 */
void cpu_init(cpu_t *cpu);

/*
 * Fetch, decode, execute instruction then return cycle count 
 */
uint8_t cpu_step(cpu_t *cpu, bus_t *bus);

#endif /* CPU_H */
