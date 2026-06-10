#!/bin/bash
# Standard automation entry point (ADR-012): run the test suite.
# Contract: runs from repo root; no args = all tests; exit != 0 = failures.
# "No tests configured" is a declared no-op (exit 0), not an error.
set -euo pipefail

PRESET="${1:-linux-debug}"
BUILD_DIR="app/build/$PRESET"

if [[ ! -f "$BUILD_DIR/CTestTestfile.cmake" ]]; then
    echo "test: no test target yet (planned in feature-release-readiness, phase 1)"
    exit 0
fi

ctest --test-dir "$BUILD_DIR" --output-on-failure
echo "test: OK"
