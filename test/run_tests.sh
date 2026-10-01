#!/bin/sh
#
# run_tests.sh - Build and run host-based unit tests for ws2812.h and main.c.
#
# Usage: sh test/run_tests.sh
#
# Compiles two standalone test binaries against the mock RMT/FreeRTOS/ESP
# headers and the real ws2812.h / main.c, then runs them. Exits non-zero if
# any test fails.
#
set -e

cd "$(dirname "$0")"

CC="${CC:-gcc}"
# -I mock  : mock ESP/FreeRTOS/RMT headers
# -I ../main : so #include "main.c" and "ws2812.h" resolve from the main/ dir
CFLAGS="-Wall -Wextra -g -I mock -I ../main"
# CONFIG_* macros that main.c / ws2812.h rely on (normally from Kconfig
# -> sdkconfig.h). Match the Kconfig defaults (GPIO 48, 500 ms).
DEFINES="-D CONFIG_LED_GPIO=48 -D CONFIG_BLINK_INTERVAL_MS=500"

BUILD="$(mktemp -d)"
trap 'rm -rf "$BUILD"' EXIT

echo "=== Building test_ws2812 ==="
$CC $CFLAGS $DEFINES mock/driver/rmt_tx_stub.c test_ws2812.c -o "$BUILD/test_ws2812"
echo "=== Building test_main ==="
$CC $CFLAGS $DEFINES mock/driver/rmt_tx_stub.c test_main.c -o "$BUILD/test_main"

overall=0

echo ""
echo "=== Running test_ws2812 ==="
if $BUILD/test_ws2812; then
    echo "[test_ws2812] PASS"
else
    echo "[test_ws2812] FAIL"
    overall=1
fi

echo ""
echo "=== Running test_main ==="
if $BUILD/test_main; then
    echo "[test_main] PASS"
else
    echo "[test_main] FAIL"
    overall=1
fi

echo ""
if [ $overall -eq 0 ]; then
    echo "=== ALL TESTS PASSED ==="
else
    echo "=== TESTS FAILED ==="
fi
exit $overall
