CC := cc
TARGET := gb
BUILD_DIR := build
SRC_DIR := src
INC_DIR := include
TEST_DIR := tests
UNITY_DIR := $(TEST_DIR)/unity
ROMS_DIR := $(TEST_DIR)/roms
SCRIPTS_DIR := $(TEST_DIR)/scripts

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

APP_SRCS := $(filter-out $(SRC_DIR)/main.c,$(SRCS))

UNIT_TEST_SRCS := $(shell find $(TEST_DIR) -maxdepth 1 -name 'test_*.c' \
				  ! -name 'test_blargg.c' \
				  ! -name 'test_mooneye.c')

UNIT_TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/tests/%,$(UNIT_TEST_SRCS))

BLARGG_TEST_BIN := $(BUILD_DIR)/tests/test_blargg
MOONEYE_TEST_BIN := $(BUILD_DIR)/tests/test_mooneye

CPPFLAGS := -I$(INC_DIR)
TEST_CPPFLAGS := $(CPPFLAGS) -I$(UNITY_DIR)
CFLAGS := -std=c11 -Wall -Wextra -g
LDFLAGS :=
LDLIBS :=

.PHONY: all clean rebuild \
	test test-unit \
	test-blargg test-blargg-cpu \
	test-mooneye test-mooneye-mbc1 \
	test-roms \
	check-unity

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

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

test-mooneye-mbc1: check-unity $(MOONEYE_TEST_BIN)
	@$(SCRIPTS_DIR)/run_mooneye_mbc1.sh $(MOONEYE_TEST_BIN)

test-blargg-cpu: check-unity $(BLARGG_TEST_BIN)
	@$(SCRIPTS_DIR)/run_blargg_cpu.sh $(BLARGG_TEST_BIN)

test-roms: check-unity $(MOONEYE_TEST_BIN) $(BLARGG_TEST_BIN)
	@$(SCRIPTS_DIR)/run_rom_tests.sh $(MOONEYE_TEST_BIN) $(BLARGG_TEST_BIN)

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
