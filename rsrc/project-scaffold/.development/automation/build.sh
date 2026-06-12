#!/bin/bash
# Standard automation entry point (ADR-012): build the project.
# Contract: runs from repo root; no args = full build; exit != 0 = failure.
# All stack-specific knowledge (CMake, presets) lives HERE, not in hooks/CI.
set -euo pipefail

PRESET="${1:-linux-debug}"

cmake --preset "$PRESET" -S app
cmake --build --preset "$PRESET"

echo "build: OK (preset $PRESET)"
