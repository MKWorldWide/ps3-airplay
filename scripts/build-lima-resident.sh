#!/usr/bin/env bash
set -euo pipefail

: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the PS3DK v0.27.0 source directory}"
PS3DEV="${PS3DEV:-$PS3DK_SOURCE/stage/ps3dev}"
PS3DK="${PS3DK:-$PS3DEV/ps3dk}"
DEPS_ROOT="${DEPS_ROOT:-$HOME/ps3-airplay-host/crypto-deps}"
MBEDTLS_VERSION=3.6.7
MBEDTLS_SOURCE="$DEPS_ROOT/src/mbedtls-$MBEDTLS_VERSION"
MBEDTLS_PPU_BUILD="$DEPS_ROOT/build-ppu-$MBEDTLS_VERSION"
MBEDTLS_INCLUDE_DIR="$MBEDTLS_SOURCE/include"
MBEDTLS_CRYPTO_LIBRARY="$MBEDTLS_PPU_BUILD/library/libmbedcrypto.a"
BUILD_DIR="${BUILD_DIR:-$(pwd)/build/lima-resident}"
JOBS="${JOBS:-2}"

export PS3DEV PS3DK PS3DK_SOURCE DEPS_ROOT MBEDTLS_SOURCE MBEDTLS_INCLUDE_DIR
export MBEDTLS_CRYPTO_LIBRARY
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PATH"

./scripts/build-lima-crypto.sh

cmake -S experimental/resident -B "$BUILD_DIR" \
	-DCMAKE_TOOLCHAIN_FILE="$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake"
BUILD_ARGS=(--build "$BUILD_DIR" --target ps3_airplay_resident --parallel "$JOBS")
if [[ "${CLEAN:-0}" == 1 ]]; then
	BUILD_ARGS+=(--clean-first)
fi
cmake "${BUILD_ARGS[@]}"
