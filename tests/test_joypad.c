#include "core/joypad.h"
#include "unity/unity.h"

#define JOYP_SELECT_D_PAD 0x10
#define JOYP_SELECT_BUTTONS 0x20

void setUp(void)
{
}

void tearDown(void)
{
}

void test_joypad_init_clears_state_and_selects_both_groups(void)
{
    joypad_t joypad = {
        .joypad_state =
            {
                .a = true,
                .b = true,
                .start = true,
                .select = true,
                .up = true,
                .down = true,
                .left = true,
                .right = true,
            },
        .select_bits = 0x30,
    };

    joypad_init(&joypad);

    TEST_ASSERT_FALSE(joypad.joypad_state.a);
    TEST_ASSERT_FALSE(joypad.joypad_state.b);
    TEST_ASSERT_FALSE(joypad.joypad_state.start);
    TEST_ASSERT_FALSE(joypad.joypad_state.select);
    TEST_ASSERT_FALSE(joypad.joypad_state.up);
    TEST_ASSERT_FALSE(joypad.joypad_state.down);
    TEST_ASSERT_FALSE(joypad.joypad_state.left);
    TEST_ASSERT_FALSE(joypad.joypad_state.right);
    TEST_ASSERT_EQUAL_HEX8(0x00, joypad.select_bits);
}

void test_joypad_set_state_copies_all_buttons(void)
{
    joypad_t joypad;
    joypad_init(&joypad);

    const gb_joypad_state_t state = {
        .a = true,
        .b = false,
        .start = true,
        .select = false,
        .up = true,
        .down = false,
        .left = true,
        .right = false,
    };

    joypad_set_state(&joypad, state);

    TEST_ASSERT_TRUE(joypad.joypad_state.a);
    TEST_ASSERT_FALSE(joypad.joypad_state.b);
    TEST_ASSERT_TRUE(joypad.joypad_state.start);
    TEST_ASSERT_FALSE(joypad.joypad_state.select);
    TEST_ASSERT_TRUE(joypad.joypad_state.up);
    TEST_ASSERT_FALSE(joypad.joypad_state.down);
    TEST_ASSERT_TRUE(joypad.joypad_state.left);
    TEST_ASSERT_FALSE(joypad.joypad_state.right);
}

void test_joypad_write_only_stores_selection_bits(void)
{
    joypad_t joypad;
    joypad_init(&joypad);

    joypad_write(&joypad, 0xFF);
    TEST_ASSERT_EQUAL_HEX8(0x30, joypad.select_bits);

    joypad_write(&joypad, 0x1F);
    TEST_ASSERT_EQUAL_HEX8(0x10, joypad.select_bits);

    joypad_write(&joypad, 0x20);
    TEST_ASSERT_EQUAL_HEX8(0x20, joypad.select_bits);

    joypad_write(&joypad, 0xC0);
    TEST_ASSERT_EQUAL_HEX8(0x00, joypad.select_bits);
}

void test_joypad_read_with_no_group_selected_returns_ff(void)
{
    joypad_t joypad;
    joypad_init(&joypad);
    joypad_set_state(&joypad, (gb_joypad_state_t){
                                  .a = true,
                                  .b = true,
                                  .start = true,
                                  .select = true,
                                  .up = true,
                                  .down = true,
                                  .left = true,
                                  .right = true,
                              });

    joypad_write(&joypad, JOYP_SELECT_D_PAD | JOYP_SELECT_BUTTONS);

    TEST_ASSERT_EQUAL_HEX8(0xFF, joypad_read(&joypad));
}

void test_joypad_read_d_pad_uses_active_low_bit_mapping(void)
{
    joypad_t joypad;
    joypad_init(&joypad);
    joypad_set_state(&joypad, (gb_joypad_state_t){
                                  .right = true,
                                  .up = true,
                              });

    joypad_write(&joypad, JOYP_SELECT_BUTTONS);

    /* P15 P14 = 10 selects directions; Down Up Left Right = 1010. */
    TEST_ASSERT_EQUAL_HEX8(0xEA, joypad_read(&joypad));
}

void test_joypad_read_buttons_uses_active_low_bit_mapping(void)
{
    joypad_t joypad;
    joypad_init(&joypad);
    joypad_set_state(&joypad, (gb_joypad_state_t){
                                  .a = true,
                                  .select = true,
                              });

    joypad_write(&joypad, JOYP_SELECT_D_PAD);

    /* P15 P14 = 01 selects buttons; Start Select B A = 1010. */
    TEST_ASSERT_EQUAL_HEX8(0xDA, joypad_read(&joypad));
}

void test_joypad_read_ignores_buttons_from_unselected_group(void)
{
    joypad_t joypad;
    joypad_init(&joypad);
    joypad_set_state(&joypad, (gb_joypad_state_t){
                                  .a = true,
                                  .b = true,
                                  .start = true,
                                  .select = true,
                              });

    joypad_write(&joypad, JOYP_SELECT_BUTTONS);

    TEST_ASSERT_EQUAL_HEX8(0xEF, joypad_read(&joypad));
}

void test_joypad_read_combines_groups_when_both_are_selected(void)
{
    joypad_t joypad;
    joypad_init(&joypad);
    joypad_set_state(&joypad, (gb_joypad_state_t){
                                  .b = true,
                                  .right = true,
                              });

    joypad_write(&joypad, 0x00);

    /* Right clears bit 0 and B clears bit 1. */
    TEST_ASSERT_EQUAL_HEX8(0xCC, joypad_read(&joypad));
}

void test_joypad_read_always_sets_unused_bits(void)
{
    joypad_t joypad;
    joypad_init(&joypad);

    joypad_write(&joypad, 0x00);

    TEST_ASSERT_BITS_HIGH(0xC0, joypad_read(&joypad));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_joypad_init_clears_state_and_selects_both_groups);
    RUN_TEST(test_joypad_set_state_copies_all_buttons);
    RUN_TEST(test_joypad_write_only_stores_selection_bits);
    RUN_TEST(test_joypad_read_with_no_group_selected_returns_ff);
    RUN_TEST(test_joypad_read_d_pad_uses_active_low_bit_mapping);
    RUN_TEST(test_joypad_read_buttons_uses_active_low_bit_mapping);
    RUN_TEST(test_joypad_read_ignores_buttons_from_unselected_group);
    RUN_TEST(test_joypad_read_combines_groups_when_both_are_selected);
    RUN_TEST(test_joypad_read_always_sets_unused_bits);
    return UNITY_END();
}
