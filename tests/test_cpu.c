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
    return UNITY_END();
}
