#!/bin/bash
# Standard automation entry point (ADR-012): build the project.
# Contract: runs from repo root; no args = full build; exit != 0 = failure.
# All stack-specific knowledge (CMake, presets) lives HERE, not in hooks/CI.
set -euo pipefail

PRESET="${1:-linux-debug}"

# --build --preset resolves CMakePresets.json from the CWD, and ours lives
# in app/ — so the build step must run from there.
cmake --preset "$PRESET" -S app
(cd app && cmake --build --preset "$PRESET")

echo "build: OK (preset $PRESET)"
