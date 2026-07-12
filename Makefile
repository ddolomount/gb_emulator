CC := cc
TARGET := gb
BUILD_DIR := build
SRC_DIR := src
INC_DIR := include
CORE_INC_DIR := $(INC_DIR)/core
TEST_DIR := tests
UNITY_DIR := $(TEST_DIR)/unity
ROMS_DIR := $(TEST_DIR)/roms
SCRIPTS_DIR := $(TEST_DIR)/scripts

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

APP_SRCS := $(filter-out $(SRC_DIR)/core/main.c,$(SRCS))

UNIT_TEST_SRCS := $(shell find $(TEST_DIR) -maxdepth 1 -name 'test_*.c' \
				  ! -name 'test_blargg.c' \
				  ! -name 'test_mooneye.c')

UNIT_TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/tests/%,$(UNIT_TEST_SRCS))

BLARGG_TEST_BIN := $(BUILD_DIR)/tests/test_blargg
MOONEYE_TEST_BIN := $(BUILD_DIR)/tests/test_mooneye

CPPFLAGS := -I$(CORE_INC_DIR) -I$(INC_DIR)
TEST_CPPFLAGS := $(CPPFLAGS) -I$(UNITY_DIR)
CFLAGS := -std=c11 -Wall -Wextra -g
LDFLAGS :=
LDLIBS :=

.PHONY: help all clean rebuild \
	test test-unit \
	test-blargg test-blargg-cpu \
	test-mooneye test-mooneye-mbc1 \
	test-roms \
	check-unity

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

help:
	@echo "Game Boy Emulator Makefile"
	@echo
	@echo "Build targets:"
	@echo "  make all                         Build the emulator binary: $(TARGET)"
	@echo "  make                             Same as make all"
	@echo "  make clean                       Remove build output and emulator binary"
	@echo "  make rebuild                     Clean and rebuild everything"
	@echo
	@echo "Unit tests:"
	@echo "  make test                        Run normal Unity unit tests"
	@echo
	@echo "Single ROM tests:"
	@echo "  make test-blargg ROM=path        Run one Blargg ROM test"
	@echo "  make test-mooneye ROM=path       Run one Mooneye ROM test"
	@echo
	@echo "ROM suite tests:"
	@echo "  make test-suite SUITE=name       Run tests from tests/suites/name.txt"
	@echo "  make test-mooneye-mbc1           Shortcut for: make test-suite SUITE=mooneye-mbc1"
	@echo "  make test-blargg-cpu             Shortcut for: make test-suite SUITE=blargg-cpu"
	@echo "  make test-roms                   Shortcut for: make test-suite SUITE=all"
	@echo
	@echo "Suite files:"
	@echo "  tests/suites/mooneye-mbc1.txt    Mooneye MBC1 cartridge tests"
	@echo "  tests/suites/blargg-cpu.txt      Blargg CPU instruction tests"
	@echo "  tests/suites/all.txt             Main configured ROM test suite"
	@echo
	@echo "Suite file format:"
	@echo "  MOONEYE path/to/rom.gb"
	@echo "  BLARGG  path/to/rom.gb"
	@echo "  SKIP    path/to/rom.gb"
	@echo
	@echo "Examples:"
	@echo "  make test"
	@echo "  make test-suite SUITE=mooneye-mbc1"
	@echo "  make test-suite SUITE=blargg-cpu"
	@echo "  make test-mooneye ROM=tests/roms/mooneye/emulator-only/mbc1/bits_bank2.gb"
	@echo "  make test-blargg ROM=tests/roms/cpu_instrs/individual/01-special.gb"
	@echo
	@echo "Available suites:"
	@ls tests/suites/*.txt 2>/dev/null | sed 's|tests/suites/|  |; s|\.txt||' || echo "  No suite files found"

test: check-unity $(UNIT_TEST_BINS)
	@for test_bin in $(UNIT_TEST_BINS); do \
		echo "Running $$test_bin"; \
		./$$test_bin || exit 1; \
	done

test-blargg: check-unity $(BLARGG_TEST_BIN)
	@if [ -z "$(ROM)" ]; then \
		echo "Usage: make test-blargg ROM=/path/to/rom.gb"; \
		exit 1; \
	fi
	@BLARGG_ROM="$(ROM)" ./$(BLARGG_TEST_BIN)

test-mooneye: check-unity $(MOONEYE_TEST_BIN)
	@if [ -z "$(ROM)" ]; then \
		echo "Usage: make test-mooneye ROM=/path/to/rom.gb"; \
		exit 1; \
	fi
	@MOONEYE_ROM="$(ROM)" ./$(MOONEYE_TEST_BIN)

test-suite: check-unity $(MOONEYE_TEST_BIN) $(BLARGG_TEST_BIN)
	@if [ -z "$(SUITE)" ]; then \
		echo "Usage: make test-suite SUITE=name"; \
		echo; \
		echo "Available suites:"; \
		ls tests/suites/*.txt 2>/dev/null | sed 's|tests/suites/||; s|\.txt||'; \
		exit 1;\
	fi
	@$(SCRIPTS_DIR)/run_suite.sh \
		"tests/suites/$(SUITE).txt" \
		"$(MOONEYE_TEST_BIN)" \
		"$(BLARGG_TEST_BIN)"

test-mooneye-mbc1: 
	@$(MAKE) test-suite SUITE=mooneye-mbc1

test-blargg-cpu:
	@$(MAKE) test-suite SUITE=blargg-cpu

test-roms: check-unity $(MOONEYE_TEST_BIN) $(BLARGG_TEST_BIN)
	@$(SCRIPTS_DIR)/run_all_suites.sh \
		"tests/suites" \
		"$(SCRIPTS_DIR)/run_suite.sh" \
		"$(MOONEYE_TEST_BIN)" \
		"$(BLARGG_TEST_BIN)"

check-unity:
	@test -f $(UNITY_DIR)/unity_internals.h || { \
		echo "Missing $(UNITY_DIR)/unity_internals.h"; \
		echo "Add unity_internals.h next to unity.c and unity.h, then run make test again."; \
		exit 1; \
	}

$(BUILD_DIR)/tests/%: $(TEST_DIR)/%.c $(UNITY_DIR)/unity.c $(APP_SRCS) | $(BUILD_DIR) check-unity
	mkdir -p $(dir $@)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

rebuild: clean all

-include $(DEPS)
