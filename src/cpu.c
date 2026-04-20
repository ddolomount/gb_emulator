#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../include/cpu.h"

#define BC_R16_ID 0
#define DE_R16_ID 1
#define HL_R16_ID 2
#define SP_R16_ID 3 

#define B_R8_ID  0
#define C_R8_ID  1
#define D_R8_ID  2
#define E_R8_ID  3
#define H_R8_ID  4
#define L_R8_ID  5
#define HL_R8_ID 6
#define A_R8_ID  7

#define GET_R16_ID(opcode) ((opcode >> 4) & 0x03)
#define GET_R8_ID(opcode)  ((opcode >> 3) & 0x07)

#define SET_FLAG(cpu, flag, cond) \
    ((cpu)->f = (cond) ? ((cpu)->f | (flag)) : ((cpu)->f & ~(flag)))

/* Static Functions */

static uint16_t cpu_get_r16(cpu_t *cpu, uint8_t reg_id);

static void cpu_set_flag(cpu_t *cpu, uint8_t flag, bool set)
{
    if (set)
        cpu->f |= flag;
    else
        cpu->f &= ~flag;
}

static inline bool half_carry_8_add(uint8_t a, uint8_t b)
{
    return ((a & 0x0F) + (b & 0x0F)) > 0x0F;
}

static inline bool carry_8_add(uint8_t a, uint8_t b)
{
    return ((uint16_t)a + (uint16_t)b) > 0xFF;
}

static inline bool half_carry_8_sub(uint8_t a, uint8_t b)
{
    return (a & 0x0F) < (b & 0x0F);
}

static inline bool carry_8_sub(uint8_t a, uint8_t b)
{
    return a < b;
}

static inline bool half_carry_16_add(uint16_t a, uint16_t b)
{
    return ((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF;
}

static inline bool carry_16_add(uint16_t a, uint16_t b)
{
    return ((uint32_t)a + (uint32_t)b) > 0xFFFF;
}

static inline bool half_carry_16_sub(uint16_t a, uint16_t b)
{
    return (a & 0x0FFF) < (b & 0x0FFF);
}

static inline bool carry_16_sub(uint16_t a, uint16_t b)
{
    return a < b;
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

static void cpu_set_r8(cpu_t *cpu, bus_t *bus, uint8_t reg_id, uint8_t value)
{
    switch (reg_id)
    {
        case B_R8_ID:
        {
            cpu->b = value;
            break;
        }
        case C_R8_ID:
        {
            cpu->c = value;
            break;
        }
        case D_R8_ID:
        {
            cpu->d = value;
            break;
        }

        case E_R8_ID:
        {
            cpu->e = value;
            break;
        }
        case H_R8_ID:
        {
            cpu->h = value;
            break;
        }
        case L_R8_ID:
        {
            cpu->l = value;
            break;
        }
        case HL_R8_ID:
        {
            uint16_t hl_mem = cpu_get_r16(cpu, HL_R16_ID);
            bus_write8(bus, hl_mem, value);
            break;
        }
        case A_R8_ID:
        {
            cpu->a = value;
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

static uint8_t cpu_get_r8(cpu_t *cpu, bus_t *bus, uint8_t reg_id)
{
    switch (reg_id)
    {
        case B_R8_ID:
        {
            return cpu->b;
        }
        case C_R8_ID:
        {
            return cpu->c;
        }
        case D_R8_ID:
        {
            return cpu->d;
        }

        case E_R8_ID:
        {
            return cpu->e;
        }
        case H_R8_ID:
        {
            return cpu->h;
        }
        case L_R8_ID:
        {
            return cpu->l;
        }
        case HL_R8_ID:
        {
            uint16_t hl_mem = cpu_get_r16(cpu, HL_R16_ID);
            return bus_read8(bus, hl_mem);
        }
        case A_R8_ID:
        {
            return cpu->a;
        }
        default:
        {
            return 0xFF;
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

            cpu_set_r16(cpu, GET_R16_ID(opcode), value);

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
            uint16_t r16mem = cpu_get_r16mem(cpu, GET_R16_ID(opcode));
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
            uint16_t r16mem = cpu_get_r16mem(cpu, GET_R16_ID(opcode));
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
            uint16_t r16 = cpu_get_r16(cpu, GET_R16_ID(opcode));
            r16++;
            cpu_set_r16(cpu, GET_R16_ID(opcode), r16);
            cycle_count = 8;
            break;
        }
        // dec r16
        case 0x0B:
        case 0x1B:
        case 0x2B:
        case 0x3B:
        {
            uint16_t r16 = cpu_get_r16(cpu, GET_R16_ID(opcode));
            r16--;
            cpu_set_r16(cpu, GET_R16_ID(opcode), r16);
            cycle_count = 8;
            break;
        }
        // add hl, r16
        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39:
        {
            uint16_t r16 = cpu_get_r16(cpu, GET_R16_ID(opcode));
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
        // inc r8
        case 0x04:
        case 0x0C:
        case 0x14:
        case 0x1C:
        case 0x24:
        case 0x2C:
        case 0x34:
        case 0x3C:
        {
            uint8_t reg_id = GET_R8_ID(opcode);
            uint8_t r8 = cpu_get_r8(cpu, bus, reg_id);
            bool half_carry = half_carry_8_add(r8, 1);
            r8++;
            cpu_set_r8(cpu, bus, reg_id, r8);

            cpu_set_flag(cpu, FLAG_Z, (r8 == 0));
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, half_carry);

            cycle_count = (reg_id == HL_R8_ID) ? 12 : 4;
            break;
        }
        // dec r8
        case 0x05:
        case 0x0D:
        case 0x15:
        case 0x1D:
        case 0x25:
        case 0x2D:
        case 0x35:
        case 0x3D:
        {
            uint8_t reg_id = GET_R8_ID(opcode);
            uint8_t r8 = cpu_get_r8(cpu, bus, reg_id);
            bool half_carry = half_carry_8_sub(r8, 1);
            r8--;
            cpu_set_r8(cpu, bus, reg_id, r8);

            cpu_set_flag(cpu, FLAG_Z, (r8 == 0));
            cpu_set_flag(cpu, FLAG_N, true);
            cpu_set_flag(cpu, FLAG_H, half_carry);

            cycle_count = (reg_id == HL_R8_ID) ? 12 : 4;
            break;
        }
        // ld r8, imm8
        case 0x06:
        case 0x0E:
        case 0x16:
        case 0x1E:
        case 0x26:
        case 0x2E:
        case 0x36:
        case 0x3E:
        {
            uint8_t reg_id = GET_R8_ID(opcode);
            uint8_t imm8 = bus_read8(bus, cpu->pc++);
            cpu_set_r8(cpu, bus, reg_id, imm8);

            cycle_count = (reg_id == HL_R8_ID) ? 12 : 8;
            break;
        }
    }

    // Return number of cycles required for instruction
    return cycle_count;
}




