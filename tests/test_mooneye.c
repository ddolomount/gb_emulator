#include "unity/unity.h"
#include "unity/unity_internals.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../include/bus.h"
#include "../include/cartridge.h"
#include "../include/cpu.h"
#include "../include/memory.h"

#define MOONEYE_MAX_STEPS 20000000ULL
#define MOONEYE_DEBUG_MESSAGE_CHARS 3000

typedef struct {
    bool passed;
    bool failed;
    bool timed_out;
    bool invalid_opcode;
    char debug_output[4096];
} mooneye_test_result_t;

void setUp(void)
{
}

void tearDown(void)
{
}

static bool file_exists(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        return false;
    }

    fclose(fp);
    return true;
}

static uint64_t mooneye_max_steps(void)
{
    const char *env_value = getenv("MOONEYE_MAX_STEPS");
    char *end_ptr;
    unsigned long long parsed_value;

    if (!env_value || env_value[0] == '\0')
    {
        return MOONEYE_MAX_STEPS;
    }

    parsed_value = strtoull(env_value, &end_ptr, 10);
    if (*end_ptr != '\0' || parsed_value == 0)
    {
        return MOONEYE_MAX_STEPS; 
    }

    return (uint64_t)parsed_value;
}

static bool mooneye_passed(cpu_t *cpu)
{
    return cpu->b == 0x03 &&
           cpu->c == 0x05 &&
           cpu->d == 0x08 &&
           cpu->e == 0x0D &&
           cpu->h == 0x15 &&
           cpu->l == 0x22;
}

static bool mooneye_failed(cpu_t *cpu)
{
    return cpu->b == 0x42 &&
           cpu->c == 0x42 &&
           cpu->d == 0x42 &&
           cpu->e == 0x42 &&
           cpu->h == 0x42 &&
           cpu->l == 0x42;
}

static void append_debug_snapshot(mooneye_test_result_t *result,
                                  bus_t *bus,
                                  cpu_t *cpu,
                                  const char *label,
                                  uint64_t step)
{
    size_t len = strlen(result->debug_output);

    if (len >= sizeof(result->debug_output)) {
        return;
    }

    snprintf(result->debug_output + len,
             sizeof(result->debug_output) - len,
             "\n[%s step=%llu PC=%04X OP=%02X SP=%04X A=%02X F=%02X B=%02X C=%02X D=%02X E=%02X H=%02X L=%02X]",
             label,
             (unsigned long long)step,
             cpu->pc,
             bus_read8(bus, cpu->pc),
             cpu->sp,
             cpu->a,
             cpu->f,
             cpu->b,
             cpu->c,
             cpu->d,
             cpu->e,
             cpu->h,
             cpu->l);
}

static void mooneye_init_post_boot(cpu_t *cpu, Memory_t *memory)
{
    memory_init(memory);

    cpu->a = 0x01;
    cpu->f = 0xB0;
    cpu->b = 0x00;
    cpu->c = 0x13;
    cpu->d = 0x00;
    cpu->e = 0xD8;
    cpu->h = 0x01;
    cpu->l = 0x4D;
    cpu->sp = 0xFFFE;
    cpu->pc = 0x0100;
    cpu->ime = false;

    memory->IO[0x05] = 0x00;
    memory->IO[0x06] = 0x00;
    memory->IO[0x07] = 0x00;
    memory->IO[0x10] = 0x80;
    memory->IO[0x11] = 0xBF;
    memory->IO[0x12] = 0xF3;
    memory->IO[0x14] = 0xBF;
    memory->IO[0x16] = 0x3F;
    memory->IO[0x17] = 0x00;
    memory->IO[0x19] = 0xBF;
    memory->IO[0x1A] = 0x7F;
    memory->IO[0x1B] = 0xFF;
    memory->IO[0x1C] = 0x9F;
    memory->IO[0x1E] = 0xBF;
    memory->IO[0x20] = 0xFF;
    memory->IO[0x21] = 0x00;
    memory->IO[0x22] = 0x00;
    memory->IO[0x23] = 0xBF;
    memory->IO[0x24] = 0x77;
    memory->IO[0x25] = 0xF3;
    memory->IO[0x26] = 0xF1;
    memory->IO[0x40] = 0x91;
    memory->IO[0x42] = 0x00;
    memory->IO[0x43] = 0x00;
    memory->IO[0x45] = 0x00;
    memory->IO[0x47] = 0xFC;
    memory->IO[0x48] = 0xFF;
    memory->IO[0x49] = 0xFF;
    memory->IO[0x4A] = 0x00;
    memory->IO[0x4B] = 0x00;
    memory->ie = 0x00;
}

static mooneye_test_result_t run_mooneye_rom(const char *rom_path, uint64_t max_steps)
{
    mooneye_test_result_t result = {0};
    Cartridge_t cart = {0};
    Memory_t memory;
    bus_t bus;
    gb_timer_t timer;
    cpu_t cpu = {0};

    if (!cartridge_load(&cart, rom_path))
    {
        result.failed = true;
        snprintf(result.debug_output, sizeof(result.debug_output), 
                 "Failed to load ROM: %s", rom_path);

        return result;
    }

    mooneye_init_post_boot(&cpu, &memory);
    timer_init(&timer);
    bus_init(&bus, &memory, &cart, &timer);

    for (uint64_t step = 0; step < max_steps; step++)
    {
        uint8_t cycles = cpu_step(&cpu, &bus);

        if (mooneye_passed(&cpu))
        {
            result.passed = true;
            break;
        }

        if (mooneye_failed(&cpu))
        {
            result.failed = true;
        }

        if (cycles == 0)
        {
            result.invalid_opcode = true;
            append_debug_snapshot(&result, &bus, &cpu, "MOONEYE FAIL", step);
            break;
        }
    }

    if (!result.passed && !result.failed && !result.invalid_opcode)
    {
        result.timed_out = true;
        append_debug_snapshot(&result, &bus, &cpu, "TIMEOUT", max_steps);
    }

    free(cart.rom);
    free(cart.ram);

    return result;
}

static void report_mooneye_result(const char *rom_path, mooneye_test_result_t result)
{
    char message[4608];
    const char *debug_output = result.debug_output[0] ? result.debug_output : "<none>";

    if (result.passed)
    {
        return;
    }

    if (result.invalid_opcode)
    {
        snprintf(message, sizeof(message), 
                 "Invalid/unimplemented opcode while running %s. Debug output: %.*s",
                 rom_path,
                 MOONEYE_DEBUG_MESSAGE_CHARS,
                 debug_output);

        TEST_FAIL_MESSAGE(message);
    }

    if (result.timed_out)
    {
        snprintf(message, sizeof(message), 
                 "Timed out running %s after %llu steps. Debug output: %.*s",
                 rom_path,
                 (unsigned long long)mooneye_max_steps(),
                 MOONEYE_DEBUG_MESSAGE_CHARS,
                 debug_output);

        TEST_FAIL_MESSAGE(message);
    }
    
    snprintf(message, sizeof(message), 
                 "Mooneye ROM failed: %s. Debug output: %.*s",
                 rom_path,
                 MOONEYE_DEBUG_MESSAGE_CHARS,
                 debug_output);

    TEST_FAIL_MESSAGE(message);
}

void test_mooneye_custom(void)
{
    const char *rom_path = getenv("MOONEYE_ROM");

    if (!rom_path || rom_path[0] == '\0' || !file_exists(rom_path))
    {
        TEST_IGNORE_MESSAGE("Set ROM to the path of a specific Mooneye .gb ROM to run test");
        return;
    }

    report_mooneye_result(rom_path, run_mooneye_rom(rom_path, mooneye_max_steps()));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mooneye_custom);
    return UNITY_END();
}
