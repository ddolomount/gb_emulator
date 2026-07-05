#!/usr/bin/env bash
set -u

if [ $# -ne 4 ]; then
    echo "Usage: $0 /path/to/suites_dir /path/to/run_suite.sh /path/to/test_mooneye /path/to/test_blargg"
    exit 1
fi

SUITES_DIR="$1"
RUN_SUITE="$2"
MOONEYE_TEST_BIN="$3"
BLARGG_TEST_BIN="$4"

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

if [ ! -d "$SUITES_DIR" ]; then
    printf "%bMissing suites directory:%b %s\n" "$RED" "$RESET" "$SUITES_DIR"
    exit 1
fi

if [ ! -x "$RUN_SUITE" ]; then
    printf "%bMissing or non-executable suite runner:%b %s\n" "$RED" "$RESET" "$RUN_SUITE"
    exit 1
fi

failed_suites=0
passed_suites=0
total_suites=0

for suite in "$SUITES_DIR"/*.txt; do
    if [ ! -f "$suite" ]; then
        continue
    fi

    suite_name="$(basename "$suite" .txt)"

    # Optional: skip disabled suite files named *.disabled.txt
    case "$suite" in
        *.disabled.txt)
            printf "%bSKIP SUITE:%b %s\n" "$YELLOW" "$RESET" "$suite_name"
            continue
            ;;
    esac

    total_suites=$((total_suites + 1))

    printf "\n%b========================================%b\n" "$BOLD" "$RESET"
    printf "%bRunning suite:%b %s\n" "$BLUE" "$RESET" "$suite_name"
    printf "%b========================================%b\n\n" "$BOLD" "$RESET"

    if "$RUN_SUITE" "$suite" "$MOONEYE_TEST_BIN" "$BLARGG_TEST_BIN"; then
        printf "%bSUITE PASS:%b %s\n" "$GREEN" "$RESET" "$suite_name"
        passed_suites=$((passed_suites + 1))
    else
        printf "%bSUITE FAIL:%b %s\n" "$RED" "$RESET" "$suite_name"
        failed_suites=$((failed_suites + 1))
    fi
done

printf "\n%b========================================%b\n" "$BOLD" "$RESET"
printf "%bAll suite summary%b\n" "$BOLD" "$RESET"
printf "%bTotal suites:%b  %d\n" "$BLUE" "$RESET" "$total_suites"
printf "%bPassed suites:%b %d\n" "$GREEN" "$RESET" "$passed_suites"
printf "%bFailed suites:%b %d\n" "$RED" "$RESET" "$failed_suites"
printf "%b========================================%b\n" "$BOLD" "$RESET"

if [ "$failed_suites" -ne 0 ]; then
    exit 1
fi

exit 0
