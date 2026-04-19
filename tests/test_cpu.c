#include "unity.h"
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

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cpu_init_sets_registers_to_zero);
    return UNITY_END();
}
