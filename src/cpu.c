#include <stdint.h>
#include <string.h>
#include "../include/cpu.h"

#define BC_R16_ID 0
#define DE_R16_ID 1
#define HL_R16_ID 2
#define SP_R16_ID 3 

#define GET_REG_ID(opcode) ((opcode >> 4) & 0x03)

#define SET_FLAG(cpu, flag, cond) \
    ((cpu)->f = (cond) ? ((cpu)->f | (flag)) : ((cpu)->f & ~(flag)))

/* Static Functions */

static void cpu_set_flag(cpu_t *cpu, uint8_t flag, bool set)
{
    if (set)
        cpu->f |= flag;
    else
        cpu->f &= ~flag;
}

static inline bool half_carry_16_add(uint16_t a, uint16_t b)
{
    return ((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF;
}

static inline bool carry_16_add(uint16_t a, uint16_t b)
{
    return ((uint32_t)a + (uint32_t)b) > 0xFFFF;
}

static void cpu_set_r16(cpu_t *cpu, uint8_t reg_id, uint16_t value)
{
    switch (reg_id)
    {
        case 0:
        {
            // BC
            cpu->b = (value >> 8) & 0xFF;
            cpu->c = value & 0xFF;
            break;
        }
        case 1:
        {
            // DE
            cpu->d = (value >> 8) & 0xFF;
            cpu->e = value & 0xFF;
            break;
        }
        case 2:
        {
            // HL
            cpu->h = (value >> 8) & 0xFF;
            cpu->l = value & 0xFF;
            break;
        }
        case 3:
        {
            // SP
            cpu->sp = value;
            break;
        }
    }
}

static uint16_t cpu_get_r16(cpu_t *cpu, uint8_t reg_id)
{
    switch (reg_id)
    {
        case 0:
        {
            return ((uint16_t)cpu->b << 8) | cpu->c; // BC
        }
        case 1:
        {
            return ((uint16_t)cpu->d << 8) | cpu->e; // DE
        }
        case 2:
        {
            return ((uint16_t)cpu->h << 8) | cpu->l; // HL
        }
        case 3:
        {
            return cpu->sp; // SP
        }
        default:
        {
            return 0;
        }
    }
}

static uint16_t cpu_get_r16mem(cpu_t *cpu, uint8_t reg_id)
{
    switch (reg_id)
    {
        case 0:
        {
            return ((uint16_t)cpu->b << 8) | cpu->c; // BC
        }
        case 1:
        {
            return ((uint16_t)cpu->d << 8) | cpu->e; // DE
        }
        case 2:
        {
            // hl+
            uint16_t hl = ((uint16_t)cpu->h << 8) | cpu->l;
            cpu_set_r16(cpu, HL_R16_ID, hl + 1);
            return hl;
        }
        case 3:
        {
            // hl-
            uint16_t hl = ((uint16_t)cpu->h << 8) | cpu->l;
            cpu_set_r16(cpu, HL_R16_ID, hl - 1);
            return hl;
        }
        default:
        {
            return 0;
        }
    }
}

/* Global Functions */

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
        // nop
        case 0x00:
        {
            cycle_count = 4;
            break;
        }
        // ld r16, imm16
        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31:
        {
            // Read imm16 from cartridge and write to register
            uint16_t value = bus_read16(bus, cpu->pc);
            cpu->pc += 2;

            cpu_set_r16(cpu, GET_REG_ID(opcode), value);

            cycle_count = 12;
            break;
        }
        // ld [r16mem], a
        case 0x02:
        case 0x12:
        case 0x22:
        case 0x32:
        {
            // Read 16-bit mem address and write A to it
            uint16_t r16mem = cpu_get_r16mem(cpu, GET_REG_ID(opcode));
            bus_write8(bus, r16mem, cpu->a);

            cycle_count = 8;
            break;
        }
        // ld a, [r16mem]
        case 0x0A:
        case 0x1A:
        case 0x2A:
        case 0x3A:
        {
            // Read 16-bit value from r16mem and write it to A
            uint16_t r16mem = cpu_get_r16mem(cpu, GET_REG_ID(opcode));
            uint8_t value = bus_read8(bus, r16mem);
            cpu->a = value;
            cycle_count = 8;
            break;
        }
        // ld [imm16], sp
        case 0x08:
        {
            uint16_t imm16_addr = bus_read8(bus, cpu->pc++); // Low byte
            imm16_addr |= ((uint16_t)bus_read8(bus, cpu->pc++) << 8); // High byte
            bus_write16(bus, imm16_addr, cpu->sp);
            cycle_count = 20;
            break;
        }
        // inc r16
        case 0x03:
        case 0x13:
        case 0x23:
        case 0x33:
        {
            uint16_t r16 = cpu_get_r16(cpu, GET_REG_ID(opcode));
            r16++;
            cpu_set_r16(cpu, GET_REG_ID(opcode), r16);
            cycle_count = 8;
            break;
        }
        // dec r16
        case 0x0B:
        case 0x1B:
        case 0x2B:
        case 0x3B:
        {
            uint16_t r16 = cpu_get_r16(cpu, GET_REG_ID(opcode));
            r16--;
            cpu_set_r16(cpu, GET_REG_ID(opcode), r16);
            cycle_count = 8;
            break;
        }
        // add hl, r16
        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39:
        {
            uint16_t r16 = cpu_get_r16(cpu, GET_REG_ID(opcode));
            uint16_t hl = cpu_get_r16(cpu, HL_R16_ID);

            uint16_t result = hl + r16;
          
            // N flag cleared
            cpu->f &= ~FLAG_N;

            // Check hald carry and carry flags
            bool half_carry = half_carry_16_add(r16, hl);
            bool carry = carry_16_add(r16, hl);

            cpu_set_flag(cpu, FLAG_H, half_carry);
            cpu_set_flag(cpu, FLAG_C, carry);

            cpu_set_r16(cpu, HL_R16_ID, result);
            cycle_count = 8;
            break;
        }
    }

    // Return number of cycles required for instruction
    return cycle_count;
}




