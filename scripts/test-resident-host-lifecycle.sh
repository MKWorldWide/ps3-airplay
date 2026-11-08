#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="${PROJECT_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build/resident-host}"
: "${MBEDTLS_INCLUDE:?Set MBEDTLS_INCLUDE to the pinned Mbed TLS 3.6.7 include directory}"
: "${CC:=cc}"

mkdir -p "$BUILD_DIR"
LOG_PATH_DEFINE="-DRECEIVER_LOG_PATH=\"$BUILD_DIR/receiver.log\""
COMMON_FLAGS=(
	-O2 -std=c11 -Wall -Wextra -Werror -pthread
	-fsanitize=address,undefined
	-I"$PROJECT_ROOT/tests/resident/mock_include"
	-I"$PROJECT_ROOT/tests/resident"
	-I"$PROJECT_ROOT/src/protocol"
	-I"$PROJECT_ROOT/src/discovery"
	-I"$PROJECT_ROOT/src/codec"
	-I"$PROJECT_ROOT/src/crypto"
	-I"$PROJECT_ROOT/src/observability"
	-I"$PROJECT_ROOT/src/audio"
	-I"$MBEDTLS_INCLUDE"
	-include "$PROJECT_ROOT/tests/resident/mock_crypto_rng.h"
	"$LOG_PATH_DEFINE"
)

"$CC" "${COMMON_FLAGS[@]}" \
	-Dsocket=resident_host_socket \
	-Dconnect=resident_host_connect \
	-Dgetsockname=resident_host_getsockname \
	-Dclose=resident_host_close \
	-c "$PROJECT_ROOT/experimental/resident/module.c" \
	-o "$BUILD_DIR/module.o"

"$CC" "${COMMON_FLAGS[@]}" \
	"$PROJECT_ROOT/tests/resident/host_lifecycle_test.c" \
	"$PROJECT_ROOT/src/audio/pcm_ring.c" \
	"$PROJECT_ROOT/src/audio/pcm_spsc.c" \
	"$BUILD_DIR/module.o" \
	-lm -o "$BUILD_DIR/host_lifecycle_test"

"$BUILD_DIR/host_lifecycle_test"

"$CC" -O2 -std=c11 -Wall -Wextra -Werror -pthread \
	-fsanitize=address,undefined -I"$PROJECT_ROOT/src/audio" \
	"$PROJECT_ROOT/src/audio/pcm_spsc.c" \
	"$PROJECT_ROOT/tests/audio/pcm_spsc_test.c" \
	-o "$BUILD_DIR/pcm_spsc_test"
"$BUILD_DIR/pcm_spsc_test"
