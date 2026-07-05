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


if [ $# -ne 2 ]; then
    echo "Usage: $0 /path/to/test_mooneye /path/to/test_blargg"
    exit 1
fi

MOONEYE_TEST_BIN="$1"
BLARGG_TEST_BIN="$2"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

failed=0

echo "========================================"
echo "Running all configured ROM tests"
echo "========================================"
echo

if "$SCRIPT_DIR/run_mooneye_mbc1.sh" "$MOONEYE_TEST_BIN"; then
    echo "Mooneye MBC1 suite passed."
else
    echo "Mooneye MBC1 suite failed"
    failed=$((failed + 1))
fi

echo


if "$SCRIPT_DIR/run_blargg_cpu.sh" "$BLARGG_TEST_BIN"; then
    echo "Blargg CPU suite passed."
else
    echo "Blargg CPU suite failed."
    failed=$((failed + 1))
fi

echo

echo "========================================"
echo "Overall ROM test summary"
echo -e "${RED}Failed suites: $failed ${RESET}"
echo "========================================"

if [ "$failed" -ne 0 ]; then
    exit 1
fi

exit 0
