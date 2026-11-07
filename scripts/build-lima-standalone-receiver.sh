#!/usr/bin/env bash
set -euo pipefail

: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the PS3DK v0.27.0 source tree}"
PROJECT_ROOT="${PROJECT_ROOT:-$(pwd)}"
PS3DEV="${PS3DEV:-$PS3DK_SOURCE/stage/ps3dev}"
PS3DK="${PS3DK:-$PS3DEV/ps3dk}"
PSL1GHT="${PSL1GHT:-$HOME/ps3-airplay-host/psl1ght/source}"
DEPS_ROOT="${DEPS_ROOT:-$HOME/ps3-airplay-host/crypto-deps}"
MBEDTLS_VERSION=3.6.7
MBEDTLS_SOURCE="$DEPS_ROOT/src/mbedtls-$MBEDTLS_VERSION"
MBEDTLS_PPU_BUILD="$DEPS_ROOT/build-ppu-$MBEDTLS_VERSION-ps3-time"
MBEDTLS_INCLUDE="$MBEDTLS_SOURCE/include"
MBEDTLS_CRYPTO_LIBRARY="$MBEDTLS_PPU_BUILD/library/libmbedcrypto.a"
OUTPUT_DIR="$PROJECT_ROOT/build/lima-standalone-receiver"
JOBS="${JOBS:-2}"
APP_VERSION="${APP_VERSION:-0.1.4}"

export PS3DEV PS3DK PSL1GHT MBEDTLS_INCLUDE MBEDTLS_CRYPTO_LIBRARY
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH"

cmake -S "$MBEDTLS_SOURCE" -B "$MBEDTLS_PPU_BUILD" \
	-DCMAKE_TOOLCHAIN_FILE="$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake" \
	-DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF \
	-DCMAKE_C_FLAGS='-DMBEDTLS_NO_PLATFORM_ENTROPY -DMBEDTLS_PLATFORM_MS_TIME_ALT -Dunix -D_POSIX_VERSION=200809L'
cmake --build "$MBEDTLS_PPU_BUILD" --target mbedcrypto --parallel "$JOBS"

make -C "$PROJECT_ROOT/prototype/standalone-receiver" clean
make -C "$PROJECT_ROOT/prototype/standalone-receiver" \
	CC=powerpc64-ps3-elf-gcc VERBOSE=1 APP_VERSION="$APP_VERSION"

mkdir -p "$OUTPUT_DIR"
install -m 0644 \
	"$PROJECT_ROOT/prototype/standalone-receiver/ps3-airplay-receiver.elf" \
	"$OUTPUT_DIR/ps3-airplay-receiver.elf"
sha256sum "$OUTPUT_DIR/ps3-airplay-receiver.elf"
