#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build/gcc"
mkdir -p "$BUILD_DIR"
g++ -std=c++20 -I"$SCRIPT_DIR" -I"$SCRIPT_DIR/runtime" -c "$SCRIPT_DIR/generated_unity.cpp" -o "$BUILD_DIR/generated_unity.o"
