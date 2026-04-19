CC := cc
TARGET := gb
BUILD_DIR := build
SRC_DIR := src
INC_DIR := include
TEST_DIR := tests
UNITY_DIR := $(TEST_DIR)/unity

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

APP_SRCS := $(filter-out $(SRC_DIR)/main.c,$(SRCS))
TEST_SRCS := $(shell find $(TEST_DIR) -maxdepth 1 -name 'test_*.c')
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/tests/%,$(TEST_SRCS))

CPPFLAGS := -I$(INC_DIR)
TEST_CPPFLAGS := $(CPPFLAGS) -I$(UNITY_DIR)
CFLAGS := -std=c11 -Wall -Wextra -g
LDFLAGS :=
LDLIBS :=

.PHONY: all clean rebuild test check-unity

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

test: check-unity $(TEST_BINS)
	@for test_bin in $(TEST_BINS); do ./$$test_bin || exit 1; done

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
