#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cpu.h"

#define BC_R16_ID 0
#define DE_R16_ID 1
#define HL_R16_ID 2
#define SP_R16_ID 3
#define AF_R16_ID 3

#define B_R8_ID  0
#define C_R8_ID  1
#define D_R8_ID  2
#define E_R8_ID  3
#define H_R8_ID  4
#define L_R8_ID  5
#define HL_R8_ID 6
#define A_R8_ID  7

#define COND_NZ_ID  0
#define COND_Z_ID   1
#define COND_NC_ID  2
#define COND_C_ID   3

#define GET_R16_ID(opcode)  ((opcode >> 4) & 0x03)
#define GET_R8_ID(opcode)   ((opcode >> 3) & 0x07)
#define GET_COND_ID(opcode) ((opcode >> 3) & 0x03) 

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

static bool cpu_get_flag(cpu_t *cpu, uint8_t flag)
{
    return (cpu->f & flag) != 0;
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

static void cpu_set_r16stk(cpu_t *cpu, uint8_t reg_id, uint16_t value)
{
    switch (reg_id) 
    {
        case BC_R16_ID:
        {
            cpu->b = (value >> 8) & 0xFF;
            cpu->c = value & 0xFF;
            break;
        }
        case DE_R16_ID:
        {
            cpu->d = (value >> 8) & 0xFF;
            cpu->e = value & 0xFF;
            break;
        }
        case HL_R16_ID:
        {
            cpu->h = (value >> 8) & 0xFF;
            cpu->l = value & 0xFF;
            break;
        }
        case AF_R16_ID:
        {
            cpu->a = (value >> 8) & 0xFF;
            cpu->f = value & 0xF0; // Only uses upper nibble
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

static void cpu_ld_addr_a(cpu_t *cpu, bus_t *bus, uint16_t addr)
{
    bus_write8(bus, addr, cpu->a);
}

static void cpu_ld_a_addr(cpu_t *cpu, bus_t *bus, uint16_t addr)
{
    cpu->a = bus_read8(bus, addr);
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

static uint16_t cpu_get_r16stk(cpu_t *cpu, uint8_t reg_id)
{
    switch (reg_id)
    {
        case BC_R16_ID:
        {
            return ((uint16_t)cpu->b << 8) | cpu->c; // BC
        }
        case DE_R16_ID:
        {
            return ((uint16_t)cpu->d << 8) | cpu->e; // DE
        }
        case HL_R16_ID:
        {
            return ((uint16_t)cpu->h << 8) | cpu->l; // HL
        }
        case AF_R16_ID:
        {
            return ((uint16_t)cpu->a << 8) | cpu->f; // AF
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

static uint8_t cpu_fetch8(cpu_t *cpu, bus_t *bus)
{
    return bus_read8(bus, cpu->pc++);
}

static uint16_t cpu_fetch16(cpu_t *cpu, bus_t *bus)
{
    uint16_t value = bus_read16(bus, cpu->pc);
    cpu->pc += 2;
    return value;
}

static bool condition_check(cpu_t *cpu, uint8_t cond_id)
{
    switch (cond_id)
    {
        case COND_NZ_ID:
        {
            return !cpu_get_flag(cpu, FLAG_Z);
        }
        case COND_Z_ID:
        {
            return cpu_get_flag(cpu, FLAG_Z);
        }
        case COND_NC_ID:
        {
            return !cpu_get_flag(cpu, FLAG_C);
        }
        case COND_C_ID:
        {
            return cpu_get_flag(cpu, FLAG_C);
        }
    }
    return false;
}

// TODO: May be able to make this general add that handles flags
static void cpu_add_a(cpu_t *cpu, uint8_t value, uint8_t carry_in)
{
    uint8_t a = cpu->a;
    uint16_t full_result = (uint16_t)a + (uint16_t)value + (uint16_t)carry_in;
    uint8_t result = (uint8_t)full_result;

    bool half_carry = ((a & 0x0F) + (value & 0x0F) + carry_in) > 0x0F;
    bool carry = full_result > 0xFF;

    cpu->a = result;

    cpu_set_flag(cpu, FLAG_Z, (result == 0));
    cpu_set_flag(cpu, FLAG_N, false);
    cpu_set_flag(cpu, FLAG_H, half_carry);
    cpu_set_flag(cpu, FLAG_C, carry);
}

static void cpu_sub_a(cpu_t *cpu, uint8_t value, uint8_t carry_in)
{
    uint8_t a = cpu->a;

    uint16_t subtrahend = (uint16_t)value + (uint16_t)carry_in;
    uint16_t full_result = (uint16_t)a - subtrahend;
    uint8_t result = (uint8_t)full_result;

    bool half_borrow = (a & 0x0F) < ((value & 0x0F) + carry_in);
    bool borrow = (uint16_t)a < subtrahend;

    cpu->a = result;

    cpu_set_flag(cpu, FLAG_Z, (result == 0));
    cpu_set_flag(cpu, FLAG_N, true);
    cpu_set_flag(cpu, FLAG_H, half_borrow);
    cpu_set_flag(cpu, FLAG_C, borrow);
}

static void cpu_alu_a(cpu_t *cpu, uint8_t op_id, uint8_t value)
{
    switch (op_id)
    {
        case 0: // ADD
            cpu_add_a(cpu, value, 0);
            break;

        case 1: // ADC
            cpu_add_a(cpu, value, cpu_get_flag(cpu, FLAG_C) ? 1 : 0);
            break;

        case 2: // SUB
            cpu_sub_a(cpu, value, 0);
            break;

        case 3: // SBC
            cpu_sub_a(cpu, value, cpu_get_flag(cpu, FLAG_C) ? 1 : 0);
            break;

        case 4: // AND
            cpu->a &= value;
            cpu_set_flag(cpu, FLAG_Z, cpu->a == 0);
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, true);
            cpu_set_flag(cpu, FLAG_C, false);
            break;

        case 5: // XOR
            cpu->a ^= value;
            cpu_set_flag(cpu, FLAG_Z, cpu->a == 0);
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, false);
            break;

        case 6: // OR
            cpu->a |= value;
            cpu_set_flag(cpu, FLAG_Z, cpu->a == 0);
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, false);
            break;

        case 7: // CP
        {
            uint8_t old_a = cpu->a;
            cpu_sub_a(cpu, value, 0);
            cpu->a = old_a;
            break;
        }
    }
}

static uint16_t cpu_add_sp_offset(cpu_t *cpu, int8_t offset)
{
    uint16_t sp = cpu->sp;
    uint8_t uoffset = (uint8_t)offset;
    uint16_t result = (uint16_t)(sp + offset);

    bool half_carry = ((sp & 0x0F) + (uoffset & 0x0F)) > 0x0F;
    bool carry = ((sp & 0xFF) + uoffset) > 0xFF;

    cpu_set_flag(cpu, FLAG_Z, false);
    cpu_set_flag(cpu, FLAG_N, false);
    cpu_set_flag(cpu, FLAG_H, half_carry);
    cpu_set_flag(cpu, FLAG_C, carry);

    return result;
}

static uint8_t cpu_execute_cb(cpu_t *cpu, bus_t *bus)
{
    uint8_t opcode = cpu_fetch8(cpu, bus);
    uint8_t reg_id = opcode & 0x07;
    uint8_t bit_index = (opcode >> 3) & 0x07;
    uint8_t value = cpu_get_r8(cpu, bus, reg_id);
    uint8_t result = value;
    uint8_t carry = 0;

    if (opcode < 0x40)
    {
        switch (bit_index)
        {
            case 0: // rlc
                carry = (value >> 7) & 0x01;
                result = (uint8_t)((value << 1) | carry);
                break;
            case 1: // rrc
                carry = value & 0x01;
                result = (uint8_t)((value >> 1) | (carry << 7));
                break;
            case 2: // rl
            {
                uint8_t old_carry = cpu_get_flag(cpu, FLAG_C) ? 1 : 0;
                carry = (value >> 7) & 0x01;
                result = (uint8_t)((value << 1) | old_carry);
                break;
            }
            case 3: // rr
            {
                uint8_t old_carry = cpu_get_flag(cpu, FLAG_C) ? 1 : 0;
                carry = value & 0x01;
                result = (uint8_t)((value >> 1) | (old_carry << 7));
                break;
            }
            case 4: // sla
                carry = (value >> 7) & 0x01;
                result = (uint8_t)(value << 1);
                break;
            case 5: // sra
                carry = value & 0x01;
                result = (uint8_t)((value >> 1) | (value & 0x80));
                break;
            case 6: // swap
                result = (uint8_t)((value << 4) | (value >> 4));
                carry = 0;
                break;
            case 7: // srl
                carry = value & 0x01;
                result = (uint8_t)(value >> 1);
                break;
        }

        cpu_set_r8(cpu, bus, reg_id, result);
        cpu_set_flag(cpu, FLAG_Z, result == 0);
        cpu_set_flag(cpu, FLAG_N, false);
        cpu_set_flag(cpu, FLAG_H, false);
        cpu_set_flag(cpu, FLAG_C, carry != 0);

        return (reg_id == HL_R8_ID) ? 16 : 8;
    }

    if (opcode < 0x80)
    {
        cpu_set_flag(cpu, FLAG_Z, (value & (1u << bit_index)) == 0);
        cpu_set_flag(cpu, FLAG_N, false);
        cpu_set_flag(cpu, FLAG_H, true);

        return (reg_id == HL_R8_ID) ? 12 : 8;
    }

    if (opcode < 0xC0)
    {
        result = (uint8_t)(value & ~(1u << bit_index));
        cpu_set_r8(cpu, bus, reg_id, result);
        return (reg_id == HL_R8_ID) ? 16 : 8;
    }

    result = (uint8_t)(value | (1u << bit_index));
    cpu_set_r8(cpu, bus, reg_id, result);
    return (reg_id == HL_R8_ID) ? 16 : 8;
}

static void cpu_push16(cpu_t *cpu, bus_t *bus, uint16_t value)
{
    cpu->sp -= 2;
    bus_write16(bus, cpu->sp, value);
}

static uint16_t cpu_pop16(cpu_t *cpu, bus_t *bus)
{
    uint16_t value = bus_read16(bus, cpu->sp);
    cpu->sp += 2;
    return value;
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
        // rlca 
        case 0x07:
        {
            uint8_t a = cpu_get_r8(cpu, bus, A_R8_ID);
            uint8_t old_bit7 = (a >> 7) & 0x01;

            a = (a << 1) | old_bit7;
            cpu_set_r8(cpu, bus, A_R8_ID, a);

            cpu_set_flag(cpu, FLAG_Z, false);
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, old_bit7);
            
            cycle_count = 4;
            break;
        }
        // rrca
        case 0x0F:
        {
            uint8_t a = cpu_get_r8(cpu, bus, A_R8_ID);
            uint8_t old_bit0 = (a & 0x01);

            a = (a >> 1) | (old_bit0 << 7);
            cpu_set_r8(cpu, bus, A_R8_ID, a);

            cpu_set_flag(cpu, FLAG_Z, false);
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, old_bit0);

            cycle_count = 4;
            break;
        }
        // rla
        case 0x17:
        {
            uint8_t a = cpu_get_r8(cpu, bus, A_R8_ID);
            uint8_t c = cpu_get_flag(cpu, FLAG_C);

            uint8_t new_c = (a >> 7) & 0x01;

            a = (a << 1) | (c & 0x01);

            cpu_set_r8(cpu, bus, A_R8_ID, a);

            cpu_set_flag(cpu, FLAG_Z, false);
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, new_c);

            cycle_count = 4;
            break;
        }
        // rra 
        case 0x1F:
        {
            uint8_t a = cpu_get_r8(cpu, bus, A_R8_ID);
            uint8_t c = cpu_get_flag(cpu, FLAG_C);
            uint8_t new_c = (a & 0x01);

            a = (a >> 1) | (c << 7);

            cpu_set_r8(cpu, bus, A_R8_ID, a);
            
            cpu_set_flag(cpu, FLAG_Z, false);
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, new_c);
            
            cycle_count = 4;
            break;
        }
        // daa
        case 0x27:
        {
            uint8_t a = cpu_get_r8(cpu, bus, A_R8_ID);
            uint8_t adjustment = 0;
            bool carry = cpu_get_flag(cpu, FLAG_C);

            if (cpu_get_flag(cpu, FLAG_N))
            {
                // previous op was subtraction
                if (cpu_get_flag(cpu, FLAG_H))
                {
                    adjustment |= 0x06;
                }

                if (carry)
                {
                    adjustment |= 0x60;
                }

                a = (uint8_t)(a - adjustment);
            }
            else 
            {
                // previous op was addition
                if (cpu_get_flag(cpu, FLAG_H) || ((a & 0x0F) > 0x09))
                {
                    adjustment |= 0x06;
                }

                if (carry || a > 0x99)
                {
                    adjustment |= 0x60;
                    carry = true;
                }

                a = (uint8_t)(a + adjustment);
            }

            cpu_set_r8(cpu, bus, A_R8_ID, a);


            cpu_set_flag(cpu, FLAG_Z, (a == 0));
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, carry);
            
            cycle_count = 4;
            break;
        }
        // cpl
        case 0x2F:
        {
            uint8_t a = cpu_get_r8(cpu, bus, A_R8_ID);
            a = ~a;
            cpu_set_r8(cpu, bus, A_R8_ID, a);

            cpu_set_flag(cpu, FLAG_N, true);
            cpu_set_flag(cpu, FLAG_H, true);

            cycle_count = 4;
            break;
        }
        // scf
        case 0x37:
        {
            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, true);

            cycle_count = 4;
            break;
        }
        // ccf
        case 0x3F:
        {
            uint8_t carry = cpu_get_flag(cpu, FLAG_C);
            carry = (~carry & 0x01);

            cpu_set_flag(cpu, FLAG_N, false);
            cpu_set_flag(cpu, FLAG_H, false);
            cpu_set_flag(cpu, FLAG_C, carry);

            cycle_count = 4;
            break;
        }
        // jr imm8
        case 0x18:
        {
            int8_t offset = (int8_t)bus_read8(bus, cpu->pc++);
            cpu->pc += offset;

            cycle_count = 12;
            break;
        }
        // jr cond, imm8
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38:
        {
            uint8_t cond_id = GET_COND_ID(opcode);
            int8_t offset = (int8_t)bus_read8(bus, cpu->pc++);
           
            if (condition_check(cpu, cond_id))
            {
                cpu->pc += offset;
                cycle_count = 12;
            }
            else 
            {
                cycle_count = 8;
            }
            break;
        }
        // cb prefix
        case 0xCB:
        {
            cycle_count = cpu_execute_cb(cpu, bus);
            break;
        }
        // ret cond
        case 0xC0:
        case 0xC8:
        case 0xD0:
        case 0xD8:
        {
            uint8_t cond_id = GET_COND_ID(opcode);

            if (condition_check(cpu, cond_id))
            {
                cpu->pc = cpu_pop16(cpu, bus);
                cycle_count = 20;
            }
            else
            {
                cycle_count = 8;
            }
            break;
        }
        // ret
        case 0xC9:
        {
            cpu->pc = cpu_pop16(cpu, bus);
            cycle_count = 16;
            break;
        }
        // reti
        case 0xD9:
        {
            cpu->pc = cpu_pop16(cpu, bus);
            cpu->ime = true;
            cycle_count = 16;
            break;
        }
        // jp cond, imm16
        case 0xC2:
        case 0xCA:
        case 0xD2:
        case 0xDA:
        {
            uint8_t cond_id = GET_COND_ID(opcode);
            uint16_t target = bus_read16(bus, cpu->pc);
            cpu->pc += 2;

            if (condition_check(cpu, cond_id))
            {
                cpu->pc = target;
                cycle_count = 16;
            }
            else
            {
                cycle_count = 12;
            }
            break;
        }
        // jp imm16
        case 0xC3:
        {
            uint16_t target = bus_read16(bus, cpu->pc);
            cpu->pc = target;
            cycle_count = 16;
            break;
        }
        // jp hl
        case 0xE9:
        {
            cpu->pc = cpu_get_r16(cpu, HL_R16_ID);
            cycle_count = 4;
            break;
        }
        // call cond, imm16
        case 0xC4:
        case 0xCC:
        case 0xD4:
        case 0xDC:
        {
            uint8_t cond_id = GET_COND_ID(opcode);
            uint16_t target = bus_read16(bus, cpu->pc);
            cpu->pc += 2;

            if (condition_check(cpu, cond_id))
            {
                cpu_push16(cpu, bus, cpu->pc);
                cpu->pc = target;
                cycle_count = 24;
            }
            else
            {
                cycle_count = 12;
            }
            break;
        }
        // call imm16
        case 0xCD:
        {
            uint16_t target = bus_read16(bus, cpu->pc);
            cpu->pc += 2;
            cpu_push16(cpu, bus, cpu->pc);
            cpu->pc = target;
            cycle_count = 24;
            break;
        }
        // rst tgt3
        case 0xC7:
        case 0xCF:
        case 0xD7:
        case 0xDF:
        case 0xE7:
        case 0xEF:
        case 0xF7:
        case 0xFF:
        {
            uint16_t target = opcode & 0x38;
            cpu_push16(cpu, bus, cpu->pc);
            cpu->pc = target;
            cycle_count = 16;
            break;
        }
        // stop
        case 0x10:
        {
            uint8_t stop_arg = bus_read8(bus, cpu->pc++);
            (void)stop_arg;

            // TODO: Handle stop behaviour

            cycle_count = 4;
            break;
        }
        // ld r8, r8
        case 0x40 ... 0x7F:
        {
            uint8_t src = (opcode & 0x07);
            uint8_t dest = GET_R8_ID(opcode);

            uint8_t value = cpu_get_r8(cpu, bus, src);
            cpu_set_r8(cpu, bus, dest, value);

            cycle_count = (dest == HL_R8_ID || src == HL_R8_ID) ? 8 : 4;
            break;
        }
        // 8-bit arithmetic operations
        case 0x80 ... 0xBF:
        {
            uint8_t reg_id = (opcode & 0x07);
            uint8_t op_id = (opcode >> 3) & 0x07;
            uint8_t value = cpu_get_r8(cpu, bus, reg_id);

            cpu_alu_a(cpu, op_id, value);

            cycle_count = (reg_id == HL_R8_ID) ? 8 : 4;
            break;
        }
        // 8-bit arithmetic with immediate
        case 0xC6:
        case 0xCE:
        case 0xDE:
        case 0xD6:
        case 0xE6:
        case 0xEE:
        case 0xF6:
        case 0xFE:
        {
            uint8_t op_id = (opcode >> 3) & 0x07;
            uint8_t value = bus_read8(bus, cpu->pc++);

            cpu_alu_a(cpu, op_id, value);

            cycle_count = 8;
            break;
        }
        // pop r16stk
        case 0xC1:
        case 0xD1:
        case 0xE1:
        case 0xF1:
        {
            uint8_t reg_id = (opcode >> 4) & 0x03;

            uint16_t value = cpu_pop16(cpu, bus);

            cpu_set_r16stk(cpu, reg_id, value);

            cycle_count = 12;
            break;
        }
        // push r16stk
        case 0xC5:
        case 0xD5:
        case 0xE5:
        case 0xF5:
        {
            uint16_t reg_id = (opcode >> 4) & 0x03;
            uint16_t value = cpu_get_r16stk(cpu, reg_id);

            cpu_push16(cpu, bus, value);

            cycle_count = 16;
            break;
        }
        // ldh [c], a
        case 0xE2:
        {
            uint8_t c = cpu_get_r8(cpu, bus, C_R8_ID);
            uint16_t addr = 0xFF00 + c;
            
            cpu_ld_addr_a(cpu, bus, addr);
            
            cycle_count = 8;
            break;
        }
        // ldh [imm8], a
        case 0xE0:
        {
            uint8_t imm8 = cpu_fetch8(cpu, bus);
            uint16_t addr = 0xFF00 + imm8;

            cpu_ld_addr_a(cpu, bus, addr);

            cycle_count = 12;
            break;
        }
        // ld [imm16], a
        case 0xEA:
        {
            uint16_t imm16 = cpu_fetch16(cpu, bus);

            cpu_ld_addr_a(cpu, bus, imm16);

            cycle_count = 16;
            break;
        }
        // ldh a, [c]
        case 0xF2:
        {
            uint8_t c = cpu_get_r8(cpu, bus, C_R8_ID);
            uint16_t addr = 0xFF00 + c;

            cpu_ld_a_addr(cpu, bus, addr);

            cycle_count = 8;
            break;
        }
        // ldh a, [imm8]
        case 0xF0:
        {
            uint8_t imm8 = cpu_fetch8(cpu, bus);
            uint16_t addr = 0xFF00 + imm8;

            cpu_ld_a_addr(cpu, bus, addr);

            cycle_count = 12;
            break;
        }
        // ld a, [imm16]
        case 0xFA:
        {
            uint16_t imm16 = cpu_fetch16(cpu, bus);

            cpu_ld_a_addr(cpu, bus, imm16);

            cycle_count = 16;
            break;
        }
        // add sp, imm8
        case 0xE8:
        {
            int8_t imm8 = cpu_fetch8(cpu, bus);
            cpu->sp = cpu_add_sp_offset(cpu, imm8);

            cycle_count = 16;
            break;
        }
        // ld hl, sp + imm8
        case 0xF8:
        {
            int8_t imm8 = cpu_fetch8(cpu, bus);
            uint16_t result = cpu_add_sp_offset(cpu, imm8);

            cpu_set_r16(cpu, HL_R16_ID, result);

            cycle_count = 12;
            break;
        }
        // ld sp, hl
        case 0xF9:
        {
            uint16_t hl = cpu_get_r16(cpu, HL_R16_ID);
            cpu_set_r16(cpu, SP_R16_ID, hl);

            cycle_count = 8;
            break;
        }
        // di
        case 0xF3:
        {
            cpu->ime = false;
            cycle_count = 4;
            break;
        }
        // ei
        case 0xFB:
        {
            cpu->ime = true;
            cycle_count = 4;
            break;
        }

    }

    // Return number of cycles required for instruction
    return cycle_count;
}

