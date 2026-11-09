#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="${PROJECT_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build/resident-diagnostics}"
CC="${CC:-cc}"

mkdir -p "$BUILD_DIR"
"$CC" -std=c11 -Wall -Wextra -Werror -pthread \
	-fsanitize=address,undefined \
	-I"$PROJECT_ROOT" \
	"$PROJECT_ROOT/tests/resident/resident_diagnostics_test.c" \
	-o "$BUILD_DIR/resident_diagnostics_test"
"$BUILD_DIR/resident_diagnostics_test"
