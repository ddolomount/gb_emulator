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
    TEST_ASSERT_FALSE(cpu.ime);
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

static void assert_accumulator_one_byte_common_result(cpu_t *cpu, uint8_t cycles)
{
    TEST_ASSERT_EQUAL_UINT8(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu->pc);

    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu->b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu->c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu->d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu->e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu->h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu->l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu->sp);
}

void test_rlca(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x07);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x85;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x0B, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(FLAG_C, cpu.f);
}

void test_rrca(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x0F);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x81;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xC0, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(FLAG_C, cpu.f);
}

void test_rla(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x17);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x80;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(FLAG_C, cpu.f);
}

void test_rra(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x1F);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x01;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(FLAG_C, cpu.f);
}

void test_daa_after_addition(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x27);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x3C;
    cpu.f = 0;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.f);
}

void test_daa_after_subtraction(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x27);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x0F;
    cpu.f = FLAG_N | FLAG_H;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x09, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(FLAG_N, cpu.f);
}

void test_cpl(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x2F);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x35;
    cpu.f = FLAG_Z | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0xCA, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(FLAG_Z | FLAG_N | FLAG_H | FLAG_C, cpu.f);
}

void test_scf(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x37);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x5A;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(FLAG_Z | FLAG_C, cpu.f);
}

void test_ccf(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x3F);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x5A;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_accumulator_one_byte_common_result(&cpu, cycles);

    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(FLAG_Z, cpu.f);
}

static void assert_branch_registers_unchanged(cpu_t *cpu)
{
    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu->a);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu->b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu->c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu->d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu->e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu->h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu->l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu->sp);
}

static void assert_control_flow_registers_unchanged(cpu_t *cpu, uint8_t expected_f)
{
    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu->a);
    TEST_ASSERT_EQUAL_UINT8(expected_f, cpu->f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu->b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu->c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu->d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu->e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu->h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu->l);
}

void test_jr_imm8_forward(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x18, 0x05, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0107, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    assert_branch_registers_unchanged(&cpu);
}

void test_jr_imm8_backward(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x18, 0xFC, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(12, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x00FE, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    assert_branch_registers_unchanged(&cpu);
}

void test_jr_cond_imm8_taken(void)
{
    const struct {
        uint8_t opcode;
        uint8_t flags;
    } cases[] = {
        {0x20, FLAG_N | FLAG_C},
        {0x28, FLAG_Z | FLAG_H},
        {0x30, FLAG_Z | FLAG_N},
        {0x38, FLAG_C | FLAG_H},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode, 0x05, 0x00);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = cases[i].flags;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(12, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0107, cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(cases[i].flags, cpu.f);
        assert_branch_registers_unchanged(&cpu);
    }
}

void test_jr_cond_imm8_not_taken(void)
{
    const struct {
        uint8_t opcode;
        uint8_t flags;
    } cases[] = {
        {0x20, FLAG_Z | FLAG_C},
        {0x28, FLAG_C},
        {0x30, FLAG_Z | FLAG_C},
        {0x38, FLAG_Z},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode, 0x05, 0x00);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = cases[i].flags;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(8, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0102, cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(cases[i].flags, cpu.f);
        assert_branch_registers_unchanged(&cpu);
    }
}

void test_stop(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x10, 0x00, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0102, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    assert_branch_registers_unchanged(&cpu);
}

void test_ret_cond_taken(void)
{
    const struct {
        uint8_t opcode;
        uint8_t flags;
    } cases[] = {
        {0xC0, FLAG_N | FLAG_C},
        {0xC8, FLAG_Z | FLAG_H},
        {0xD0, FLAG_Z | FLAG_N},
        {0xD8, FLAG_C | FLAG_H},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = cases[i].flags;
        cpu.sp = 0xDFFC;

        memory.WRAM[0x1FFB] = 0x11;
        memory.WRAM[0x1FFC] = 0x78;
        memory.WRAM[0x1FFD] = 0x56;
        memory.WRAM[0x1FFE] = 0x22;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(20, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
        TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
        assert_control_flow_registers_unchanged(&cpu, cases[i].flags);
        TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x1FFB]);
        TEST_ASSERT_EQUAL_UINT8(0x78, memory.WRAM[0x1FFC]);
        TEST_ASSERT_EQUAL_UINT8(0x56, memory.WRAM[0x1FFD]);
        TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x1FFE]);
    }
}

void test_ret_cond_not_taken(void)
{
    const struct {
        uint8_t opcode;
        uint8_t flags;
    } cases[] = {
        {0xC0, FLAG_Z | FLAG_C},
        {0xC8, FLAG_C},
        {0xD0, FLAG_Z | FLAG_C},
        {0xD8, FLAG_Z},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = cases[i].flags;
        cpu.sp = 0xDFFC;

        memory.WRAM[0x1FFB] = 0x11;
        memory.WRAM[0x1FFC] = 0x78;
        memory.WRAM[0x1FFD] = 0x56;
        memory.WRAM[0x1FFE] = 0x22;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(8, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
        TEST_ASSERT_EQUAL_UINT16(0xDFFC, cpu.sp);
        assert_control_flow_registers_unchanged(&cpu, cases[i].flags);
        TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x1FFB]);
        TEST_ASSERT_EQUAL_UINT8(0x78, memory.WRAM[0x1FFC]);
        TEST_ASSERT_EQUAL_UINT8(0x56, memory.WRAM[0x1FFD]);
        TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x1FFE]);
    }
}

void test_ret(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xC9);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.sp = 0xDFFC;

    memory.WRAM[0x1FFB] = 0x11;
    memory.WRAM[0x1FFC] = 0x78;
    memory.WRAM[0x1FFD] = 0x56;
    memory.WRAM[0x1FFE] = 0x22;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
    assert_control_flow_registers_unchanged(&cpu, 0xF0);
    TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x1FFB]);
    TEST_ASSERT_EQUAL_UINT8(0x78, memory.WRAM[0x1FFC]);
    TEST_ASSERT_EQUAL_UINT8(0x56, memory.WRAM[0x1FFD]);
    TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x1FFE]);
}

void test_reti(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xD9);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.sp = 0xDFFC;

    memory.WRAM[0x1FFB] = 0x11;
    memory.WRAM[0x1FFC] = 0x78;
    memory.WRAM[0x1FFD] = 0x56;
    memory.WRAM[0x1FFE] = 0x22;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
    assert_control_flow_registers_unchanged(&cpu, 0xF0);
    TEST_ASSERT_TRUE(cpu.ime);
    TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x1FFB]);
    TEST_ASSERT_EQUAL_UINT8(0x78, memory.WRAM[0x1FFC]);
    TEST_ASSERT_EQUAL_UINT8(0x56, memory.WRAM[0x1FFD]);
    TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x1FFE]);
}

void test_jp_cond_imm16_taken(void)
{
    const struct {
        uint8_t opcode;
        uint8_t flags;
    } cases[] = {
        {0xC2, FLAG_N | FLAG_C},
        {0xCA, FLAG_Z | FLAG_H},
        {0xD2, FLAG_Z | FLAG_N},
        {0xDA, FLAG_C | FLAG_H},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode, 0x78, 0x56);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = cases[i].flags;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(16, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
        TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
        assert_control_flow_registers_unchanged(&cpu, cases[i].flags);
    }
}

void test_jp_cond_imm16_not_taken(void)
{
    const struct {
        uint8_t opcode;
        uint8_t flags;
    } cases[] = {
        {0xC2, FLAG_Z | FLAG_C},
        {0xCA, FLAG_C},
        {0xD2, FLAG_Z | FLAG_C},
        {0xDA, FLAG_Z},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode, 0x78, 0x56);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = cases[i].flags;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(12, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0103, cpu.pc);
        TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
        assert_control_flow_registers_unchanged(&cpu, cases[i].flags);
    }
}

void test_jp_imm16(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xC3, 0x78, 0x56);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(16, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    assert_branch_registers_unchanged(&cpu);
}

void test_jp_hl(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xE9);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.h = 0xC1;
    cpu.l = 0x23;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0xC123, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
    TEST_ASSERT_EQUAL_UINT8(0xA1, cpu.a);
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu.b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu.d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu.e);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.h);
    TEST_ASSERT_EQUAL_UINT8(0x23, cpu.l);
}

void test_call_cond_imm16_taken(void)
{
    const struct {
        uint8_t opcode;
        uint8_t flags;
    } cases[] = {
        {0xC4, FLAG_N | FLAG_C},
        {0xCC, FLAG_Z | FLAG_H},
        {0xD4, FLAG_Z | FLAG_N},
        {0xDC, FLAG_C | FLAG_H},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode, 0x78, 0x56);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = cases[i].flags;

        memory.WRAM[0x1FFB] = 0x11;
        memory.WRAM[0x1FFC] = 0x00;
        memory.WRAM[0x1FFD] = 0x00;
        memory.WRAM[0x1FFE] = 0x22;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(24, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
        TEST_ASSERT_EQUAL_UINT16(0xDFFC, cpu.sp);
        assert_control_flow_registers_unchanged(&cpu, cases[i].flags);
        TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x1FFB]);
        TEST_ASSERT_EQUAL_UINT8(0x03, memory.WRAM[0x1FFC]);
        TEST_ASSERT_EQUAL_UINT8(0x01, memory.WRAM[0x1FFD]);
        TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x1FFE]);
    }
}

void test_call_cond_imm16_not_taken(void)
{
    const struct {
        uint8_t opcode;
        uint8_t flags;
    } cases[] = {
        {0xC4, FLAG_Z | FLAG_C},
        {0xCC, FLAG_C},
        {0xD4, FLAG_Z | FLAG_C},
        {0xDC, FLAG_Z},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode, 0x78, 0x56);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.f = cases[i].flags;

        memory.WRAM[0x1FFB] = 0x11;
        memory.WRAM[0x1FFC] = 0x00;
        memory.WRAM[0x1FFD] = 0x00;
        memory.WRAM[0x1FFE] = 0x22;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(12, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0103, cpu.pc);
        TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
        assert_control_flow_registers_unchanged(&cpu, cases[i].flags);
        TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x1FFB]);
        TEST_ASSERT_EQUAL_UINT8(0x00, memory.WRAM[0x1FFC]);
        TEST_ASSERT_EQUAL_UINT8(0x00, memory.WRAM[0x1FFD]);
        TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x1FFE]);
    }
}

void test_call_imm16(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xCD, 0x78, 0x56);

    cpu_t cpu = test_cpu_with_sentinel_registers();

    memory.WRAM[0x1FFB] = 0x11;
    memory.WRAM[0x1FFC] = 0x00;
    memory.WRAM[0x1FFD] = 0x00;
    memory.WRAM[0x1FFE] = 0x22;

    uint8_t cycles = cpu_step(&cpu, &bus);

    TEST_ASSERT_EQUAL_UINT8(24, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x5678, cpu.pc);
    TEST_ASSERT_EQUAL_UINT16(0xDFFC, cpu.sp);
    assert_control_flow_registers_unchanged(&cpu, 0xF0);
    TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x1FFB]);
    TEST_ASSERT_EQUAL_UINT8(0x03, memory.WRAM[0x1FFC]);
    TEST_ASSERT_EQUAL_UINT8(0x01, memory.WRAM[0x1FFD]);
    TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x1FFE]);
}

void test_rst_tgt3(void)
{
    const struct {
        uint8_t opcode;
        uint16_t target;
    } cases[] = {
        {0xC7, 0x0000},
        {0xCF, 0x0008},
        {0xD7, 0x0010},
        {0xDF, 0x0018},
        {0xE7, 0x0020},
        {0xEF, 0x0028},
        {0xF7, 0x0030},
        {0xFF, 0x0038},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode);

        cpu_t cpu = test_cpu_with_sentinel_registers();

        memory.WRAM[0x1FFB] = 0x11;
        memory.WRAM[0x1FFC] = 0x00;
        memory.WRAM[0x1FFD] = 0x00;
        memory.WRAM[0x1FFE] = 0x22;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(16, cycles);
        TEST_ASSERT_EQUAL_UINT16(cases[i].target, cpu.pc);
        TEST_ASSERT_EQUAL_UINT16(0xDFFC, cpu.sp);
        assert_control_flow_registers_unchanged(&cpu, 0xF0);
        TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x1FFB]);
        TEST_ASSERT_EQUAL_UINT8(0x01, memory.WRAM[0x1FFC]);
        TEST_ASSERT_EQUAL_UINT8(0x01, memory.WRAM[0x1FFD]);
        TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x1FFE]);
    }
}

void test_ld_r8_r8_registers(void)
{
    const uint8_t reg_ids[] = {0, 1, 2, 3, 4, 5, 7};

    for (size_t dst_i = 0; dst_i < sizeof(reg_ids) / sizeof(reg_ids[0]); dst_i++) {
        for (size_t src_i = 0; src_i < sizeof(reg_ids) / sizeof(reg_ids[0]); src_i++) {
            uint8_t dst_id = reg_ids[dst_i];
            uint8_t src_id = reg_ids[src_i];
            uint8_t opcode = 0x40 | (dst_id << 3) | src_id;
            uint8_t value = 0x40 + (uint8_t)(dst_i * 8) + (uint8_t)src_i;

            uint8_t rom[0x200] = {0};
            Memory_t memory;
            Cartridge_t cartridge;
            bus_t bus;

            setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, opcode);

            cpu_t cpu = test_cpu_with_sentinel_registers();
            test_set_r8_register(&cpu, src_id, value);
            if (dst_id != src_id) {
                test_set_r8_register(&cpu, dst_id, 0x00);
            }

            uint8_t cycles = cpu_step(&cpu, &bus);

            TEST_ASSERT_EQUAL_UINT8(4, cycles);
            TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
            TEST_ASSERT_EQUAL_UINT8(value, test_get_r8_register(&cpu, dst_id));
            TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
            TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
        }
    }
}

void test_ld_r8_hl_mem(void)
{
    const uint8_t dst_ids[] = {0, 1, 2, 3, 4, 5, 7};

    for (size_t i = 0; i < sizeof(dst_ids) / sizeof(dst_ids[0]); i++) {
        uint8_t dst_id = dst_ids[i];
        uint8_t opcode = 0x40 | (dst_id << 3) | 0x06;
        uint8_t value = 0x80 + (uint8_t)i;

        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, opcode);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.h = 0xC1;
        cpu.l = 0x23;

        memory.WRAM[0x0122] = 0x11;
        memory.WRAM[0x0123] = value;
        memory.WRAM[0x0124] = 0x22;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(8, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(value, test_get_r8_register(&cpu, dst_id));
        TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x0122]);
        TEST_ASSERT_EQUAL_UINT8(value, memory.WRAM[0x0123]);
        TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x0124]);
        TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
        TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
    }
}

void test_ld_hl_mem_r8(void)
{
    const uint8_t src_ids[] = {0, 1, 2, 3, 4, 5, 7};

    for (size_t i = 0; i < sizeof(src_ids) / sizeof(src_ids[0]); i++) {
        uint8_t src_id = src_ids[i];
        uint8_t opcode = 0x40 | (0x06 << 3) | src_id;

        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, opcode);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.h = 0xC4;
        cpu.l = 0x56;

        if (src_id == 4) {
            cpu.h = 0xC4;
        } else if (src_id == 5) {
            cpu.l = 0x56;
        } else {
            test_set_r8_register(&cpu, src_id, 0x90 + (uint8_t)i);
        }

        uint8_t value = test_get_r8_register(&cpu, src_id);

        memory.WRAM[0x0455] = 0x33;
        memory.WRAM[0x0456] = 0x00;
        memory.WRAM[0x0457] = 0x44;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(8, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(value, memory.WRAM[0x0456]);
        TEST_ASSERT_EQUAL_UINT8(0x33, memory.WRAM[0x0455]);
        TEST_ASSERT_EQUAL_UINT8(0x44, memory.WRAM[0x0457]);
        TEST_ASSERT_EQUAL_UINT8(value, test_get_r8_register(&cpu, src_id));
        TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.f);
        TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
    }
}

static void assert_alu_a_common_result(cpu_t *cpu,
                                       uint8_t cycles,
                                       uint8_t expected_a,
                                       uint8_t expected_f)
{
    TEST_ASSERT_EQUAL_UINT8(4, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0101, cpu->pc);
    TEST_ASSERT_EQUAL_UINT8(expected_a, cpu->a);
    TEST_ASSERT_EQUAL_UINT8(expected_f, cpu->f);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu->c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu->d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu->e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu->h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu->l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu->sp);
}

static void assert_alu_a_imm8_common_result(cpu_t *cpu,
                                            uint8_t cycles,
                                            uint8_t expected_a,
                                            uint8_t expected_f)
{
    TEST_ASSERT_EQUAL_UINT8(8, cycles);
    TEST_ASSERT_EQUAL_UINT16(0x0102, cpu->pc);
    TEST_ASSERT_EQUAL_UINT8(expected_a, cpu->a);
    TEST_ASSERT_EQUAL_UINT8(expected_f, cpu->f);
    TEST_ASSERT_EQUAL_UINT8(0xB1, cpu->b);
    TEST_ASSERT_EQUAL_UINT8(0xC1, cpu->c);
    TEST_ASSERT_EQUAL_UINT8(0xD1, cpu->d);
    TEST_ASSERT_EQUAL_UINT8(0xE1, cpu->e);
    TEST_ASSERT_EQUAL_UINT8(0xA5, cpu->h);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu->l);
    TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu->sp);
}

void test_add_a_r8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x80);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x8F;
    cpu.b = 0x91;
    cpu.f = FLAG_Z | FLAG_N;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_common_result(&cpu, cycles, 0x20, FLAG_H | FLAG_C);
    TEST_ASSERT_EQUAL_UINT8(0x91, cpu.b);
}

void test_adc_a_r8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x88);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x8E;
    cpu.b = 0x71;
    cpu.f = FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_common_result(&cpu, cycles, 0x00, FLAG_Z | FLAG_H | FLAG_C);
    TEST_ASSERT_EQUAL_UINT8(0x71, cpu.b);
}

void test_sub_a_r8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x90);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x10;
    cpu.b = 0x01;
    cpu.f = FLAG_Z | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_common_result(&cpu, cycles, 0x0F, FLAG_N | FLAG_H);
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.b);
}

void test_sbc_a_r8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0x98);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x10;
    cpu.b = 0x0F;
    cpu.f = FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_common_result(&cpu, cycles, 0x00, FLAG_Z | FLAG_N | FLAG_H);
    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.b);
}

void test_and_a_r8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xA0);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0xF0;
    cpu.b = 0x0F;
    cpu.f = FLAG_N | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_common_result(&cpu, cycles, 0x00, FLAG_Z | FLAG_H);
    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.b);
}

void test_xor_a_r8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xA8);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x5A;
    cpu.b = 0x5A;
    cpu.f = FLAG_N | FLAG_H | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_common_result(&cpu, cycles, 0x00, FLAG_Z);
    TEST_ASSERT_EQUAL_UINT8(0x5A, cpu.b);
}

void test_or_a_r8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xB0);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x50;
    cpu.b = 0x0A;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_common_result(&cpu, cycles, 0x5A, 0);
    TEST_ASSERT_EQUAL_UINT8(0x0A, cpu.b);
}

void test_cp_a_r8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xB8);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x10;
    cpu.b = 0x01;
    cpu.f = FLAG_Z | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_common_result(&cpu, cycles, 0x10, FLAG_N | FLAG_H);
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.b);
}

void test_add_a_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xC6, 0x91, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x8F;
    cpu.f = FLAG_Z | FLAG_N;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_imm8_common_result(&cpu, cycles, 0x20, FLAG_H | FLAG_C);
}

void test_adc_a_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xCE, 0x71, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x8E;
    cpu.f = FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_imm8_common_result(&cpu, cycles, 0x00, FLAG_Z | FLAG_H | FLAG_C);
}

void test_sub_a_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xD6, 0x01, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x10;
    cpu.f = FLAG_Z | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_imm8_common_result(&cpu, cycles, 0x0F, FLAG_N | FLAG_H);
}

void test_sbc_a_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xDE, 0x0F, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x10;
    cpu.f = FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_imm8_common_result(&cpu, cycles, 0x00, FLAG_Z | FLAG_N | FLAG_H);
}

void test_and_a_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xE6, 0x0F, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0xF0;
    cpu.f = FLAG_N | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_imm8_common_result(&cpu, cycles, 0x00, FLAG_Z | FLAG_H);
}

void test_xor_a_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xEE, 0x5A, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x5A;
    cpu.f = FLAG_N | FLAG_H | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_imm8_common_result(&cpu, cycles, 0x00, FLAG_Z);
}

void test_or_a_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xF6, 0x0A, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x50;
    cpu.f = FLAG_Z | FLAG_N | FLAG_H | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_imm8_common_result(&cpu, cycles, 0x5A, 0);
}

void test_cp_a_imm8(void)
{
    uint8_t rom[0x200] = {0};
    Memory_t memory;
    Cartridge_t cartridge;
    bus_t bus;

    setup_instruction_test(rom, sizeof(rom), &memory, &cartridge, &bus, 0xFE, 0x01, 0x00);

    cpu_t cpu = test_cpu_with_sentinel_registers();
    cpu.a = 0x10;
    cpu.f = FLAG_Z | FLAG_C;

    uint8_t cycles = cpu_step(&cpu, &bus);

    assert_alu_a_imm8_common_result(&cpu, cycles, 0x10, FLAG_N | FLAG_H);
}

void test_alu_a_hl_mem_operands(void)
{
    const struct {
        uint8_t opcode;
        uint8_t initial_a;
        uint8_t initial_f;
        uint8_t operand;
        uint8_t expected_a;
        uint8_t expected_f;
    } cases[] = {
        {0x86, 0x8F, FLAG_Z | FLAG_N, 0x91, 0x20, FLAG_H | FLAG_C},
        {0x8E, 0x8E, FLAG_C, 0x71, 0x00, FLAG_Z | FLAG_H | FLAG_C},
        {0x96, 0x10, FLAG_Z | FLAG_C, 0x01, 0x0F, FLAG_N | FLAG_H},
        {0x9E, 0x10, FLAG_C, 0x0F, 0x00, FLAG_Z | FLAG_N | FLAG_H},
        {0xA6, 0xF0, FLAG_N | FLAG_C, 0x0F, 0x00, FLAG_Z | FLAG_H},
        {0xAE, 0x5A, FLAG_N | FLAG_H | FLAG_C, 0x5A, 0x00, FLAG_Z},
        {0xB6, 0x50, FLAG_Z | FLAG_N | FLAG_H | FLAG_C, 0x0A, 0x5A, 0},
        {0xBE, 0x10, FLAG_Z | FLAG_C, 0x01, 0x10, FLAG_N | FLAG_H},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        uint8_t rom[0x200] = {0};
        Memory_t memory;
        Cartridge_t cartridge;
        bus_t bus;

        setup_opcode_test(rom, sizeof(rom), &memory, &cartridge, &bus, cases[i].opcode);

        cpu_t cpu = test_cpu_with_sentinel_registers();
        cpu.a = cases[i].initial_a;
        cpu.f = cases[i].initial_f;
        cpu.h = 0xC1;
        cpu.l = 0x23;

        memory.WRAM[0x0122] = 0x11;
        memory.WRAM[0x0123] = cases[i].operand;
        memory.WRAM[0x0124] = 0x22;

        uint8_t cycles = cpu_step(&cpu, &bus);

        TEST_ASSERT_EQUAL_UINT8(8, cycles);
        TEST_ASSERT_EQUAL_UINT16(0x0101, cpu.pc);
        TEST_ASSERT_EQUAL_UINT8(cases[i].expected_a, cpu.a);
        TEST_ASSERT_EQUAL_UINT8(cases[i].expected_f, cpu.f);
        TEST_ASSERT_EQUAL_UINT8(cases[i].operand, memory.WRAM[0x0123]);
        TEST_ASSERT_EQUAL_UINT8(0x11, memory.WRAM[0x0122]);
        TEST_ASSERT_EQUAL_UINT8(0x22, memory.WRAM[0x0124]);
        TEST_ASSERT_EQUAL_UINT8(0xC1, cpu.h);
        TEST_ASSERT_EQUAL_UINT8(0x23, cpu.l);
        TEST_ASSERT_EQUAL_UINT16(0xDFFE, cpu.sp);
    }
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
    RUN_TEST(test_rlca);
    RUN_TEST(test_rrca);
    RUN_TEST(test_rla);
    RUN_TEST(test_rra);
    RUN_TEST(test_daa_after_addition);
    RUN_TEST(test_daa_after_subtraction);
    RUN_TEST(test_cpl);
    RUN_TEST(test_scf);
    RUN_TEST(test_ccf);
    RUN_TEST(test_jr_imm8_forward);
    RUN_TEST(test_jr_imm8_backward);
    RUN_TEST(test_jr_cond_imm8_taken);
    RUN_TEST(test_jr_cond_imm8_not_taken);
    RUN_TEST(test_ret_cond_taken);
    RUN_TEST(test_ret_cond_not_taken);
    RUN_TEST(test_ret);
    RUN_TEST(test_reti);
    RUN_TEST(test_jp_cond_imm16_taken);
    RUN_TEST(test_jp_cond_imm16_not_taken);
    RUN_TEST(test_jp_imm16);
    RUN_TEST(test_jp_hl);
    RUN_TEST(test_call_cond_imm16_taken);
    RUN_TEST(test_call_cond_imm16_not_taken);
    RUN_TEST(test_call_imm16);
    RUN_TEST(test_rst_tgt3);
    RUN_TEST(test_stop);
    RUN_TEST(test_ld_r8_r8_registers);
    RUN_TEST(test_ld_r8_hl_mem);
    RUN_TEST(test_ld_hl_mem_r8);
    RUN_TEST(test_add_a_r8);
    RUN_TEST(test_adc_a_r8);
    RUN_TEST(test_sub_a_r8);
    RUN_TEST(test_sbc_a_r8);
    RUN_TEST(test_and_a_r8);
    RUN_TEST(test_xor_a_r8);
    RUN_TEST(test_or_a_r8);
    RUN_TEST(test_cp_a_r8);
    RUN_TEST(test_add_a_imm8);
    RUN_TEST(test_adc_a_imm8);
    RUN_TEST(test_sub_a_imm8);
    RUN_TEST(test_sbc_a_imm8);
    RUN_TEST(test_and_a_imm8);
    RUN_TEST(test_xor_a_imm8);
    RUN_TEST(test_or_a_imm8);
    RUN_TEST(test_cp_a_imm8);
    RUN_TEST(test_alu_a_hl_mem_operands);
    return UNITY_END();
}
