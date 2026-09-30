#!/bin/bash
# Standard automation entry point (ADR-012): run the test suite.
# Contract: runs from repo root; no args = all tests; exit != 0 = failures.
# "No tests configured" is a declared no-op (exit 0), not an error.
set -euo pipefail

PRESET="${1:-linux-debug}"
BUILD_DIR="app/build/$PRESET"

if [[ ! -f "$BUILD_DIR/CTestTestfile.cmake" ]]; then
    echo "test: no build tree configured at $BUILD_DIR — run build.sh first (no-op)"
    exit 0
fi

# --no-tests=ignore: a build configured with -DDEVDASH_BUILD_TESTS=OFF exposes
# no tests; treat that as the declared no-op rather than a hard error.
ctest --test-dir "$BUILD_DIR" --output-on-failure --no-tests=ignore
echo "test: OK (preset $PRESET)"
