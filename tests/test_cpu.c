#include "unity/unity.h"
#include "../include/cpu.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_cpu_init_sets_registers_to_zero(void)
{
    cpu_t cpu = {0};

    TEST_ASSERT_EQUAL_UINT8(0, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0, cpu.sp);
    TEST_ASSERT_EQUAL_UINT16(0, cpu.pc);
}

void test_cpu_nop_instruction(void)
{
    uint8_t rom[0x200] = {0};
    rom[0x0100] = 0x00;

    Cartridge_t cartridge = {
        .rom = rom,
        .rom_size = sizeof(rom),
    };

    Memory_t memory = {0};
    bus_t bus;

    bus_init(&bus, &memory, &cartridge);

    cpu_t cpu = {0};
    cpu.pc = 0x0100;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(4, cycles);

    TEST_ASSERT_EQUAL_UINT8(0, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0, cpu.sp);
}

static void setup_instruction_test(uint8_t *rom,
                                   size_t rom_size,
                                   Memory_t *memory,
                                   Cartridge_t *cartridge,
                                   bus_t *bus,
                                   uint8_t opcode,
                                   uint8_t imm_lo,
                                   uint8_t imm_hi)
{
    rom[0x0100] = opcode;
    rom[0x0101] = imm_lo;
    rom[0x0102] = imm_hi;

    *cartridge = (Cartridge_t){
        .rom = rom,
        .rom_size = rom_size,
    };

    *memory = (Memory_t){0};

    bus_init(bus, memory, cartridge);
}

static void setup_opcode_test(uint8_t *rom,
                              size_t rom_size,
                              Memory_t *memory,
                              Cartridge_t *cartridge,
                              bus_t *bus,
                              uint8_t opcode)
{
    setup_instruction_test(rom, rom_size, memory, cartridge, bus, opcode, 0x00, 0x00);
}

static cpu_t test_cpu_with_sentinel_registers(void)
{
    return (cpu_t){
        .a = 0xA1,
        .f = 0xF0,
        .b = 0xB1,
        .c = 0xC1,
        .d = 0xD1,
        .e = 0xE1,
        .h = 0xA5,
        .l = 0x5A,
        .sp = 0xDFFE,
        .pc = 0x0100,
    };
}

static void assert_ld_r16_common_result(cpu_t *cpu, uint8_t cycles)
{
    TEST_ASSERT_EQUAL_UINT8(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0103, cpu->pc);

    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu->a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu->f);
}

void test_ld_bc_imm16(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x01, 0x34, 0x12);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_r16_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x12, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0x34, cpu.c);

    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_ld_de_imm16(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x11, 0xCD, 0xAB);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_r16_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);

    TEST_ASSERT_EQUAL_UINT8(0xAB, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xCD, cpu.e);

    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_ld_hl_imm16(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x21, 0xEF, 0xBE);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_r16_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);

    TEST_ASSERT_EQUAL_UINT8(0xBE, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0xEF, cpu.l);

    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_ld_sp_imm16(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x31, 0xFE, 0xFF);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_r16_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);

    TEST_ASSERT_EQUAL_UINT16(0xFFFE, cpu.sp);
}

static void assert_ld_r16mem_a_common_result(cpu_t *cpu, uint8_t cycles)
{
    TEST_ASSERT_EQUAL_UINT8(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu->pc);

    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu->a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu->f);
}

void test_ld_bc_mem_a(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x02);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.b = 0xC1;
    cpu.c = 0x23;

    memory.WRAM[0x0122] = 0x44;
    memory.WRAM[0x0123] = 0x00;
    memory.WRAM[0x0124] = 0x55;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_r16mem_a_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xA1, memory.WRAM[0x0123]);
    TEST_ASSERT_EQUAL_UINT8(0x44, memory.WRAM[0x0122]);
    TEST_ASSERT_EQUAL_UINT8(0x55, memory.WRAM[0x0124]);

    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0x23, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_ld_de_mem_a(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x12);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.d = 0xC4;
    cpu.e = 0x56;

    memory.WRAM[0x0455] = 0x66;
    memory.WRAM[0x0456] = 0x00;
    memory.WRAM[0x0457] = 0x77;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_r16mem_a_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xA1, memory.WRAM[0x0456]);
    TEST_ASSERT_EQUAL_UINT8(0x66, memory.WRAM[0x0455]);
    TEST_ASSERT_EQUAL_UINT8(0x77, memory.WRAM[0x0457]);

    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xC4, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0x56, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_ld_hli_mem_a(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x22);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xC7;
    cpu.l = 0x89;

    memory.WRAM[0x0788] = 0x88;
    memory.WRAM[0x0789] = 0x00;
    memory.WRAM[0x078A] = 0x99;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_r16mem_a_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xA1, memory.WRAM[0x0789]);
    TEST_ASSERT_EQUAL_UINT8(0x88, memory.WRAM[0x0788]);
    TEST_ASSERT_EQUAL_UINT8(0x99, memory.WRAM[0x078A]);

    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xC7, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x8A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_ld_hld_mem_a(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x32);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xC7;
    cpu.l = 0x89;

    memory.WRAM[0x0788] = 0xAA;
    memory.WRAM[0x0789] = 0x00;
    memory.WRAM[0x078A] = 0xBB;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_r16mem_a_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xA1, memory.WRAM[0x0789]);
    TEST_ASSERT_EQUAL_UINT8(0xAA, memory.WRAM[0x0788]);
    TEST_ASSERT_EQUAL_UINT8(0xBB, memory.WRAM[0x078A]);

    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xC7, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x88, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

static void assert_ld_a_r16mem_common_result(cpu_t *cpu, uint8_t cycles)
{
    TEST_ASSERT_EQUAL_UINT8(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu->pc);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu->f);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu->sp);
}

void test_ld_a_bc_mem(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x0A);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.b = 0xC1;
    cpu.c = 0x23;

    memory.WRAM[0x0123] = 0x4E;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_a_r16mem_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x4E, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0x23, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
}

void test_ld_a_de_mem(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x1A);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.d = 0xC4;
    cpu.e = 0x56;

    memory.WRAM[0x0456] = 0x9C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_a_r16mem_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x9C, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xC4, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0x56, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
}

void test_ld_a_hli_mem(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x2A);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xC7;
    cpu.l = 0x89;

    memory.WRAM[0x0788] = 0x11;
    memory.WRAM[0x0789] = 0x62;
    memory.WRAM[0x078A] = 0x22;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_a_r16mem_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x62, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x0788]);
    TEST_ASSERT_EQUAL_UINT8(0x62, memory.WRAM[0x0789]);
    TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x078A]);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xC7, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x8A, cpu.l);
}

void test_ld_a_hld_mem(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x3A);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xC7;
    cpu.l = 0x89;

    memory.WRAM[0x0788] = 0x33;
    memory.WRAM[0x0789] = 0xE4;
    memory.WRAM[0x078A] = 0x44;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_ld_a_r16mem_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xE4, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0x33, memory.WRAM[0x0788]);
    TEST_ASSERT_EQUAL_UINT8(0xE4, memory.WRAM[0x0789]);
    TEST_ASSERT_EQUAL_UINT8(0x44, memory.WRAM[0x078A]);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xC7, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x88, cpu.l);
}

void test_ld_imm16_mem_sp(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x08, 0x34, 0xC2);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.sp = 0xDFFE;

    memory.WRAM[0x0233] = 0x11;
    memory.WRAM[0x0234] = 0x00;
    memory.WRAM[0x0235] = 0x00;
    memory.WRAM[0x0236] = 0x22;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(20, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0103, cpu.pc);

    TEST_ASSERT_EQUAL_UINT8(0xFE, memory.WRAM[0x0234]);
    TEST_ASSERT_EQUAL_UINT8(0xDF, memory.WRAM[0x0235]);
    TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x0233]);
    TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x0236]);

    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

static void assert_r16_one_byte_common_result(cpu_t *cpu, uint8_t cycles)
{
    TEST_ASSERT_EQUAL_UINT8(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu->pc);

    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu->a);
}

void test_inc_bc(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x03);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.b = 0x12;
    cpu.c = 0xFF;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_r16_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0x13, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_inc_de(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x13);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.d = 0xFF;
    cpu.e = 0xFF;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_r16_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_inc_hl(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x23);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0x7F;
    cpu.l = 0xFF;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_r16_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_inc_sp(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x33);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.sp = 0xFFFF;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_r16_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0x0000, cpu.sp);
}

void test_dec_bc(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x0B);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.b = 0x13;
    cpu.c = 0x00;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_r16_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0x12, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_dec_de(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x1B);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.d = 0x00;
    cpu.e = 0x00;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_r16_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_dec_hl(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x2B);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0x80;
    cpu.l = 0x00;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_r16_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
}

void test_dec_sp(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x3B);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.sp = 0x0000;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_r16_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, cpu.sp);
}

static void assert_add_hl_r16_common_result(cpu_t *cpu, uint8_t cycles)
{
    TEST_ASSERT_EQUAL_UINT8(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu->pc);

    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu->a);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu->sp);
}

void test_add_hl_bc(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x09);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.b = 0x01;
    cpu.c = 0x02;
    cpu.h = 0x12;
    cpu.l = 0x34;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_add_hl_r16_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(FLAG_Z, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0x02, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0x13, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x36, cpu.l);
}

void test_add_hl_de_sets_half_carry(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x19);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.d = 0x00;
    cpu.e = 0x01;
    cpu.h = 0x0F;
    cpu.l = 0xFF;
    cpu.f = FLAG_N | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_add_hl_r16_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(FLAG_H, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.l);
}

void test_add_hl_hl_sets_carry_without_setting_zero(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x29);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0x80;
    cpu.l = 0x00;
    cpu.f = FLAG_N | FLAG_H;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_add_hl_r16_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(FLAG_C, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.l);
}

void test_add_hl_sp_sets_half_carry_and_carry(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x39);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xFF;
    cpu.l = 0xFF;
    cpu.sp = 0x0001;
    cpu.f = FLAG_Z | FLAG_N;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);

    TEST_ASSERT_EQUAL_UINT8(FLAG_Z | FLAG_H | FLAG_C, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.l);
    TEST_ASSERT_EQUAL_UINT16(0x0001, cpu.sp);
}

static void test_set_r8_register(cpu_t *cpu, uint8_t reg_id, uint8_t value)
{
    switch (reg_id) {
        case 0:
            cpu->b = value;
            break;
        case 1:
            cpu->c = value;
            break;
        case 2:
            cpu->d = value;
            break;
        case 3:
            cpu->e = value;
            break;
        case 4:
            cpu->h = value;
            break;
        case 5:
            cpu->l = value;
            break;
        case 7:
            cpu->a = value;
            break;
    }
}

static uint8_t test_get_r8_register(cpu_t *cpu, uint8_t reg_id)
{
    switch (reg_id) {
        case 0:
            return cpu->b;
        case 1:
            return cpu->c;
        case 2:
            return cpu->d;
        case 3:
            return cpu->e;
        case 4:
            return cpu->h;
        case 5:
            return cpu->l;
        case 7:
            return cpu->a;
        default:
            return 0xFF;
    }
}

void test_inc_r8_registers(void)
{
    const struct {
        uint8_t opcode;
        uint8_t reg_id;
    } cases[] = {
        {0x04, 0},
        {0x0C, 1},
        {0x14, 2},
        {0x1C, 3},
        {0x24, 4},
        {0x2C, 5},
        {0x3C, 7},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = FLAG_Z | FLAG_N | FLAG_C;
        test_set_r8_register(&cpu, cases[i].reg_id, 0x0F);

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(4, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(0x10, test_get_r8_register(&cpu, cases[i].reg_id));
        TEST_ASSERT_EQUAL_UINT8(FLAG_H | FLAG_C, cpu.f);
    }
}

void test_inc_hl_mem(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x34);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xC1;
    cpu.l = 0x23;
    cpu.f = FLAG_N | FLAG_C;

    memory.WRAM[0x0122] = 0x44;
    memory.WRAM[0x0123] = 0xFF;
    memory.WRAM[0x0124] = 0x55;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x00, memory.WRAM[0x0123]);
    TEST_ASSERT_EQUAL_UINT8(0x44, memory.WRAM[0x0122]);
    TEST_ASSERT_EQUAL_UINT8(0x55, memory.WRAM[0x0124]);
    TEST_ASSERT_EQUAL_UINT8(FLAG_Z | FLAG_H | FLAG_C, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x23, cpu.l);
}

void test_dec_r8_registers(void)
{
    const struct {
        uint8_t opcode;
        uint8_t reg_id;
    } cases[] = {
        {0x05, 0},
        {0x0D, 1},
        {0x15, 2},
        {0x1D, 3},
        {0x25, 4},
        {0x2D, 5},
        {0x3D, 7},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = FLAG_Z | FLAG_C;
        test_set_r8_register(&cpu, cases[i].reg_id, 0x10);

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(4, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(0x0F, test_get_r8_register(&cpu, cases[i].reg_id));
        TEST_ASSERT_EQUAL_UINT8(FLAG_N | FLAG_H | FLAG_C, cpu.f);
    }
}

void test_dec_hl_mem(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x35);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xC4;
    cpu.l = 0x56;
    cpu.f = FLAG_C;

    memory.WRAM[0x0455] = 0x66;
    memory.WRAM[0x0456] = 0x01;
    memory.WRAM[0x0457] = 0x77;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x00, memory.WRAM[0x0456]);
    TEST_ASSERT_EQUAL_UINT8(0x66, memory.WRAM[0x0455]);
    TEST_ASSERT_EQUAL_UINT8(0x77, memory.WRAM[0x0457]);
    TEST_ASSERT_EQUAL_UINT8(FLAG_Z | FLAG_N | FLAG_C, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xC4, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x56, cpu.l);
}

void test_ld_r8_imm8_registers(void)
{
    const struct {
        uint8_t opcode;
        uint8_t reg_id;
        uint8_t value;
    } cases[] = {
        {0x06, 0, 0x10},
        {0x0E, 1, 0x21},
        {0x16, 2, 0x32},
        {0x1E, 3, 0x43},
        {0x26, 4, 0x54},
        {0x2E, 5, 0x65},
        {0x3E, 7, 0x87},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode, cases[i].value, 0x00);

        cpu_t cpu = test_cpu_with_sentinel_registers();

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(8, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0102, cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(cases[i].value, test_get_r8_register(&cpu, cases[i].reg_id));
        TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
        TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
    }
}

void test_ld_hl_mem_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x36, 0x7B, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xC7;
    cpu.l = 0x89;

    memory.WRAM[0x0788] = 0x11;
    memory.WRAM[0x0789] = 0x00;
    memory.WRAM[0x078A] = 0x22;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0102, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0x7B, memory.WRAM[0x0789]);
    TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x0788]);
    TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x078A]);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xC7, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x89, cpu.l);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cpu_init_sets_registers_to_zero);
    RUN_TEST(test_cpu_nop_instruction);
    RUN_TEST(test_ld_bc_imm16);
    RUN_TEST(test_ld_de_imm16);
    RUN_TEST(test_ld_hl_imm16);
    RUN_TEST(test_ld_sp_imm16);
    RUN_TEST(test_ld_bc_mem_a);
    RUN_TEST(test_ld_de_mem_a);
    RUN_TEST(test_ld_hli_mem_a);
    RUN_TEST(test_ld_hld_mem_a);
    RUN_TEST(test_ld_a_bc_mem);
    RUN_TEST(test_ld_a_de_mem);
    RUN_TEST(test_ld_a_hli_mem);
    RUN_TEST(test_ld_a_hld_mem);
    RUN_TEST(test_ld_imm16_mem_sp);
    RUN_TEST(test_inc_bc);
    RUN_TEST(test_inc_de);
    RUN_TEST(test_inc_hl);
    RUN_TEST(test_inc_sp);
    RUN_TEST(test_dec_bc);
    RUN_TEST(test_dec_de);
    RUN_TEST(test_dec_hl);
    RUN_TEST(test_dec_sp);
    RUN_TEST(test_add_hl_bc);
    RUN_TEST(test_add_hl_de_sets_half_carry);
    RUN_TEST(test_add_hl_hl_sets_carry_without_setting_zero);
    RUN_TEST(test_add_hl_sp_sets_half_carry_and_carry);
    RUN_TEST(test_inc_r8_registers);
    RUN_TEST(test_inc_hl_mem);
    RUN_TEST(test_dec_r8_registers);
    RUN_TEST(test_dec_hl_mem);
    RUN_TEST(test_ld_r8_imm8_registers);
    RUN_TEST(test_ld_hl_mem_imm8);
    return UNITY_END();
}
