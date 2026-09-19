#include "core/ppu.h"
#include "unity/unity.h"
#include "unity/unity_internals.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_ppu_init_sets_registers_to_zero(void)
{
    ppu_t ppu = {0};

    ppu_init(&ppu);

    TEST_ASSERT_EQUAL_INT(PPU_MODE_2, ppu.mode);
    TEST_ASSERT_FALSE(ppu.frame_ready);

    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.lcdc);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.stat);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.scy);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.scx);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.ly);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.lyc);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.bgp);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.obp0);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.obp1);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.wy);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.registers.wx);
}

void test_ppu_transition_mode_2_to_mode_3_after_80_cyles()
{
    ppu_t ppu;

    ppu_init(&ppu);

    ppu_step(&ppu, 80);

    TEST_ASSERT_EQUAL_INT(PPU_MODE_3, ppu.mode);
}

void test_ppu_transition_mode_2_to_mode_3_after_84_cycles()
{
    ppu_t ppu;

    ppu_init(&ppu);

    ppu_step(&ppu, 80);

    TEST_ASSERT_EQUAL_INT(PPU_MODE_3, ppu.mode);
    TEST_ASSERT_EQUAL_UINT16(4, ppu.mode_cycles);
}

void test_ppu_transition_mode_3_to_mode_0_after_172_cyles()
{
    ppu_t ppu = {
        .mode = PPU_MODE_3,
    };

    ppu_step(&ppu, 172);

    TEST_ASSERT_EQUAL_INT(PPU_MODE_0, ppu.mode);
}
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ppu_init_sets_registers_to_zero);
    RUN_TEST(test_ppu_transition_mode_2_to_mode_3_after_80_cyles);
    RUN_TEST(test_ppu_transition_mode_3_to_mode_0_after_172_cyles);
    return UNITY_END();
}
