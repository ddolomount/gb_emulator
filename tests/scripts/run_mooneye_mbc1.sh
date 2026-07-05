#!/usr/bin/env bash
set -u

if [ -t 1 ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[0;33m'
    BLUE='\033[0;34m'
    BOLD='\033[1m'
    RESET='\033[0m'
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    BOLD=''
    RESET=''
fi

if [ $# -ne 1 ]; then
    echo "Usage: $0 /path/to/test_mooneye"
    exit 1
fi

TEST_BIN="$1"

ROMS=(
    "tests/roms/mooneye/emulator-only/mbc1/bits_bank1.gb"
    "tests/roms/mooneye/emulator-only/mbc1/bits_bank2.gb"
    "tests/roms/mooneye/emulator-only/mbc1/bits_mode.gb"
    "tests/roms/mooneye/emulator-only/mbc1/bits_ramg.gb"
    "tests/roms/mooneye/emulator-only/mbc1/ram_64kb.gb"
    "tests/roms/mooneye/emulator-only/mbc1/ram_256kb.gb"
    "tests/roms/mooneye/emulator-only/mbc1/rom_1Mb.gb"
    "tests/roms/mooneye/emulator-only/mbc1/rom_2Mb.gb"
    "tests/roms/mooneye/emulator-only/mbc1/rom_4Mb.gb"
    "tests/roms/mooneye/emulator-only/mbc1/rom_8Mb.gb"
    "tests/roms/mooneye/emulator-only/mbc1/rom_16Mb.gb"
)

failed=0
passed=0

for rom in "${ROMS[@]}"; do
    echo "========================================"
    echo "Running Mooneye MBC1: $rom"
    echo "========================================"

    if [ ! -f "$rom" ]; then
        echo "MISSING: $rom"
        failed=$((failed + 1))
        echo
        continue
    fi

    if MOONEYE_ROM="$rom" "$TEST_BIN"; then
        echo "PASS: $rom"
        passed=$((passed + 1))
    else
        echo "FAIL: $rom"
        failed=$((failed + 1))
    fi

    echo
done

echo "========================================"
echo "Mooneye MBC1 summary"
echo -e "${GREEN}Passed: $passed ${RESET}"
echo -e "${RED}Failed: $failed ${RESET}"
echo "========================================"

if [ "$failed" -ne 0 ]; then
    exit 1
fi

exit 0
