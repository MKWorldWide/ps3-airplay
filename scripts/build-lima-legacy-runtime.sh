#!/usr/bin/env bash
set -euo pipefail

: "${PS3DEV:?Set PS3DEV to the isolated PS3DEV bundle containing the 2020 runtime build}"
: "${PSL1GHT:?Set PSL1GHT to the checked-out 2020 PSL1GHT source tree}"
: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the PS3DK source tree used for its PPU CMake toolchain}"
: "${MBEDTLS_SOURCE:?Set MBEDTLS_SOURCE to the verified Mbed TLS 3.6.7 source tree}"
: "${MBEDTLS_BUILD:?Set MBEDTLS_BUILD to a separate legacy-runtime Mbed TLS build directory}"
: "${MBEDTLS_INCLUDE:?Set MBEDTLS_INCLUDE to the Mbed TLS include directory}"
: "${MBEDTLS_CRYPTO_LIBRARY:?Set MBEDTLS_CRYPTO_LIBRARY to the legacy-runtime libmbedcrypto.a}"

PROJECT_ROOT="${PROJECT_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
PS3DK="${PS3DK:-$PS3DEV}"
JOBS="${JOBS:-2}"
APP_VERSION="${APP_VERSION:-0.1.6}"
LEGACY_PSL1GHT_COMMIT="${LEGACY_PSL1GHT_COMMIT:-6e565a70e927f55813babe86dc6f64149535f908}"
ACTUAL_PSL1GHT_COMMIT="$(git -C "$PSL1GHT" rev-parse HEAD)"
if [[ "$ACTUAL_PSL1GHT_COMMIT" != "$LEGACY_PSL1GHT_COMMIT" ]]; then
	echo "Expected PSL1GHT $LEGACY_PSL1GHT_COMMIT, found $ACTUAL_PSL1GHT_COMMIT" >&2
	exit 1
fi
export PS3DEV PSL1GHT PS3DK
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH"

cmake -S "$MBEDTLS_SOURCE" -B "$MBEDTLS_BUILD" \
	-DCMAKE_TOOLCHAIN_FILE="$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake" \
	-DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF \
	-DCMAKE_C_FLAGS='-DMBEDTLS_NO_PLATFORM_ENTROPY -DMBEDTLS_PLATFORM_MS_TIME_ALT -Dunix -D_POSIX_VERSION=200809L'
cmake --build "$MBEDTLS_BUILD" --target mbedcrypto --parallel "$JOBS"
test -f "$MBEDTLS_CRYPTO_LIBRARY"

make -C "$PROJECT_ROOT/prototype/standalone-receiver" \
	LEGACY_RUNTIME=1 BUILD=build-legacy-runtime clean
make -C "$PROJECT_ROOT/prototype/standalone-receiver" \
	LEGACY_RUNTIME=1 BUILD=build-legacy-runtime \
	APP_VERSION="$APP_VERSION" \
	MBEDTLS_INCLUDE="$MBEDTLS_INCLUDE" \
	MBEDTLS_CRYPTO_LIBRARY="$MBEDTLS_CRYPTO_LIBRARY"

ELF="$PROJECT_ROOT/prototype/standalone-receiver/ps3-airplay-receiver.elf"
python3 "$PROJECT_ROOT/scripts/patch-ps3-elf.py" "$ELF"
"$PS3DEV/ppu/bin/powerpc64-ps3-elf-readelf" -h "$ELF" | grep -E 'Class:|Data:|Machine:'
"$PS3DEV/ppu/bin/powerpc64-ps3-elf-nm" "$ELF" | grep -E 'diagnostic_screen_open|raop_receiver_init|main'
