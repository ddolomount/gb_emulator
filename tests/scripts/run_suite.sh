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


if [ $# -ne 3 ]; then
    echo "Usage: $0 /path/to/suite.txt /path/to/text_mooneye /path/to/test_blargg"
    exit 1
fi

SUITE_FILE="$1"
MOONEYE_TEST_BIN="$2"
BLARGG_TEST_BIN="$3"

if [ ! -f "$SUITE_FILE" ]; then
    printf "%bMissing suite file: %b\n" "$RED" "$RESET" "$SUITE_FILE"
    exit 1
fi


failed=0
passed=0
missing=0
skipped=0

while IFS= read -r line || [ -n "$line" ]; do
    # Skip blank lines and comments.
    case "$line" in
        ""|\#*) continue ;;
    esac

    runner="${line%% *}"
    rom="${line#* }"

    # Optional skip support:
    # SKIP path/to/test.gb
    if [ "$runner" = "SKIP" ]; then
        printf "%bSKIP:%b %s\n" "$YELLOW" "$RESET" "$rom"
        skipped=$((skipped + 1))
        continue
    fi

    if [ ! -f "$rom" ]; then
        printf "%bMISSING:%b %s\n" "$YELLOW" "$RESET" "$rom"
        missing=$((missing + 1))
        failed=$((failed + 1))
        continue
    fi

    printf "%bRUN:%b [%s] %s\n" "$BLUE" "$RESET" "$runner" "$rom"

    case "$runner" in
        MOONEYE)
            if ROM= MOONEYE_ROM="$rom" "$MOONEYE_TEST_BIN"; then
                printf "%bPASS:%b %s\n\n" "$GREEN" "$RESET" "$rom"
                passed=$((passed + 1))
            else
                printf "%bFAIL:%b %s\n\n" "$RED" "$RESET" "$rom"
                failed=$((failed + 1))
            fi
            ;;

        BLARGG)
            if ROM= BLARGG_ROM="$rom" "$BLARGG_TEST_BIN"; then
                printf "%bPASS:%b %s\n\n" "$GREEN" "$RESET" "$rom"
                passed=$((passed + 1))
            else
                printf "%bFAIL:%b %s\n\n" "$RED" "$RESET" "$rom"
                failed=$((failed + 1))
            fi
            ;;

        *)
            printf "%bUNKNOWN RUNNER:%b %s\n" "$RED" "$RESET" "$runner"
            failed=$((failed + 1))
            ;;
    esac
done < "$SUITE_FILE"

printf "========================================\n"
printf "%bSuite summary%b\n" "$BOLD" "$RESET"
printf "%bPassed:%b  %d\n" "$GREEN" "$RESET" "$passed"
printf "%bFailed:%b  %d\n" "$RED" "$RESET" "$failed"
printf "%bMissing:%b %d\n" "$YELLOW" "$RESET" "$missing"
printf "%bSkipped:%b %d\n" "$YELLOW" "$RESET" "$skipped"
printf "========================================\n"  


if [ "$failed" -ne 0 ]; then
    exit 1
fi

exit 0
