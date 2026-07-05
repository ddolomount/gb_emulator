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
    echo "Usage: $0 /path/to/test_blargg"
    exit 1
fi

TEST_BIN="$1"

ROMS=(
    "tests/roms/cpu_instrs/individual/01-special.gb"
    "tests/roms/cpu_instrs/individual/02-interrupts.gb"
    "tests/roms/cpu_instrs/individual/03-op sp,hl.gb"
    "tests/roms/cpu_instrs/individual/04-op r,imm.gb"
    "tests/roms/cpu_instrs/individual/05-op rp.gb"
    "tests/roms/cpu_instrs/individual/06-ld r,r.gb"
    "tests/roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb"
    "tests/roms/cpu_instrs/individual/08-misc instrs.gb"
    "tests/roms/cpu_instrs/individual/09-op r,r.gb"
    "tests/roms/cpu_instrs/individual/10-bit ops.gb"
    "tests/roms/cpu_instrs/individual/11-op a,(hl).gb" 
)

failed=0
passed=0

for rom in "${ROMS[@]}"; do
    echo "========================================"
    echo "Running Blargg CPU: $rom"
    echo "========================================"

    if [ ! -f "$rom" ]; then
        echo "MISSING: $rom"
        failed=$((failed + 1))
        echo
        continue
    fi

    if BLARGG_ROM="$rom" "$TEST_BIN"; then
        echo "PASS: $rom"
        passed=$((passed + 1))
    else
        echo "FAIL: $rom"
        failed=$((failed + 1))
    fi

    echo
done

echo "========================================"
echo "Blargg CPU summary"
echo -e "${GREEN}Passed: $passed ${RESET}"
echo -e "${RED}Failed: $failed ${RESET}"
echo "========================================"

if [ "$failed" -ne 0 ]; then
    exit 1
fi

exit 0
