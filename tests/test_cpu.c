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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cpu_init_sets_registers_to_zero);
    RUN_TEST(test_cpu_nop_instruction);
    return UNITY_END();
}
