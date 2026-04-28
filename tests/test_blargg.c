#include "unity/unity.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/bus.h"
#include "../include/cartridge.h"
#include "../include/cpu.h"
#include "../include/memory.h"

#define BLARGG_SERIAL_SB_ADDR 0xFF01
#define BLARGG_SERIAL_SC_ADDR 0xFF02
#define BLARGG_SERIAL_TRANSFER_START 0x81
#define BLARGG_DEFAULT_MAX_STEPS 5000000ULL
#define BLARGG_DEFAULT_STALL_STEPS 1000000ULL
#define BLARGG_SERIAL_MESSAGE_CHARS 3000

typedef struct {
    bool passed;
    bool failed;
    bool timed_out;
    bool invalid_opcode;
    char serial_output[4096];
} rom_test_result_t;

void setUp(void)
{
}

void tearDown(void)
{
}

static bool file_exists(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return false;
    }

    fclose(fp);
    return true;
}

static bool resolve_blargg_rom_path(char *buffer,
                                    size_t buffer_size,
                                    const char *env_var,
                                    const char *relative_path)
{
    const char *explicit_path = getenv(env_var);
    const char *rom_dir = getenv("BLARGG_ROM_DIR");

    if (explicit_path && file_exists(explicit_path)) {
        snprintf(buffer, buffer_size, "%s", explicit_path);
        return true;
    }

    if (rom_dir) {
        snprintf(buffer, buffer_size, "%s/%s", rom_dir, relative_path);
        if (file_exists(buffer)) {
            return true;
        }
    }

    buffer[0] = '\0';
    return false;
}

static uint64_t blargg_max_steps(void)
{
    const char *env_value = getenv("BLARGG_MAX_STEPS");
    char *end_ptr;
    unsigned long long parsed_value;

    if (!env_value || env_value[0] == '\0') {
        return BLARGG_DEFAULT_MAX_STEPS;
    }

    parsed_value = strtoull(env_value, &end_ptr, 10);
    if (*end_ptr != '\0' || parsed_value == 0) {
        return BLARGG_DEFAULT_MAX_STEPS;
    }

    return (uint64_t)parsed_value;
}

static uint64_t blargg_stall_steps(void)
{
    const char *env_value = getenv("BLARGG_STALL_STEPS");
    char *end_ptr;
    unsigned long long parsed_value;

    if (!env_value || env_value[0] == '\0') {
        return BLARGG_DEFAULT_STALL_STEPS;
    }

    parsed_value = strtoull(env_value, &end_ptr, 10);
    if (*end_ptr != '\0' || parsed_value == 0) {
        return BLARGG_DEFAULT_STALL_STEPS;
    }

    return (uint64_t)parsed_value;
}

static void append_serial_char(rom_test_result_t *result, uint8_t ch)
{
    size_t len = strlen(result->serial_output);

    if (len + 1 < sizeof(result->serial_output)) {
        result->serial_output[len] = (char)ch;
        result->serial_output[len + 1] = '\0';
    }
}

static void append_debug_snapshot(rom_test_result_t *result,
                                  bus_t *bus,
                                  cpu_t *cpu,
                                  const char *label,
                                  uint64_t step)
{
    size_t len = strlen(result->serial_output);

    if (len >= sizeof(result->serial_output)) {
        return;
    }

    snprintf(result->serial_output + len,
             sizeof(result->serial_output) - len,
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

static void blargg_init_post_boot(cpu_t *cpu, Memory_t *memory)
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

static void poll_blargg_serial(bus_t *bus, rom_test_result_t *result)
{
    uint8_t sc = bus_read8(bus, BLARGG_SERIAL_SC_ADDR);

    if (sc == BLARGG_SERIAL_TRANSFER_START) {
        append_serial_char(result, bus_read8(bus, BLARGG_SERIAL_SB_ADDR));
        bus_write8(bus, BLARGG_SERIAL_SC_ADDR, 0x00);
    }
}

static rom_test_result_t run_blargg_rom(const char *rom_path, uint64_t max_steps)
{
    rom_test_result_t result = {0};
    Cartridge_t cartridge = {0};
    Memory_t memory;
    bus_t bus;
    cpu_t cpu = {0};
    uint64_t last_serial_step = 0;
    size_t last_serial_len = 0;
    uint64_t stall_limit = blargg_stall_steps();

    if (!cartridge_load(&cartridge, rom_path)) {
        result.failed = true;
        snprintf(result.serial_output, sizeof(result.serial_output),
                 "Failed to load ROM: %s", rom_path);
        return result;
    }

    blargg_init_post_boot(&cpu, &memory);
    bus_init(&bus, &memory, &cartridge);

    for (uint64_t step = 0; step < max_steps; step++) {
        uint8_t cycles = cpu_step(&cpu, &bus);

        poll_blargg_serial(&bus, &result);

        size_t serial_len = strlen(result.serial_output);
        if (serial_len != last_serial_len) {
            last_serial_len = serial_len;
            last_serial_step = step;
        }

        if (strstr(result.serial_output, "Passed")) {
            result.passed = true;
            break;
        }

        if (strstr(result.serial_output, "Failed")) {
            result.failed = true;
            break;
        }

        if (cycles == 0) {
            result.invalid_opcode = true;
            append_debug_snapshot(&result, &bus, &cpu, "INVALID OPCODE", step);
            break;
        }

        if (step - last_serial_step >= stall_limit) {
            result.timed_out = true;
            append_debug_snapshot(&result, &bus, &cpu, "STALL", step);
            break;
        }
    }

    if (!result.passed && !result.failed && !result.invalid_opcode) {
        result.timed_out = true;
    }

    free(cartridge.rom);
    return result;
}

static void assert_blargg_rom_passes(const char *env_var, const char *relative_path)
{
    char rom_path[1024];
    char message[4608];
    rom_test_result_t result;
    const char *serial_output;

    if (!resolve_blargg_rom_path(rom_path, sizeof(rom_path), env_var, relative_path)) {
        snprintf(message, sizeof(message),
                 "Set %s or BLARGG_ROM_DIR to run %s", env_var, relative_path);
        TEST_IGNORE_MESSAGE(message);
    }

    result = run_blargg_rom(rom_path, blargg_max_steps());
    serial_output = result.serial_output[0] ? result.serial_output : "<none>";

    if (result.passed) {
        return;
    }

    if (result.invalid_opcode) {
        snprintf(message, sizeof(message),
                 "Invalid/unimplemented opcode while running %s. Serial output: %.*s",
                 rom_path,
                 BLARGG_SERIAL_MESSAGE_CHARS,
                 serial_output);
        TEST_FAIL_MESSAGE(message);
    }

    if (result.timed_out) {
        snprintf(message, sizeof(message),
                 "Timed out running %s after %llu steps. Serial output: %.*s",
                 rom_path,
                 (unsigned long long)blargg_max_steps(),
                 BLARGG_SERIAL_MESSAGE_CHARS,
                 serial_output);
        TEST_FAIL_MESSAGE(message);
    }

    snprintf(message, sizeof(message),
             "Blargg ROM failed: %s. Serial output: %.*s",
             rom_path,
             BLARGG_SERIAL_MESSAGE_CHARS,
             serial_output);
    TEST_FAIL_MESSAGE(message);
}

void test_blargg_cpu_instrs(void)
{
    assert_blargg_rom_passes("BLARGG_CPU_INSTRS_ROM", "cpu_instrs/cpu_instrs.gb");
}

void test_blargg_instr_timing(void)
{
    assert_blargg_rom_passes("BLARGG_INSTR_TIMING_ROM", "instr_timing/instr_timing.gb");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_blargg_cpu_instrs);
    RUN_TEST(test_blargg_instr_timing);
    return UNITY_END();
}
