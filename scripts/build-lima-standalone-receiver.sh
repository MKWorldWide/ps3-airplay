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
APP_VERSION="${APP_VERSION:-0.1.21}"
RTSP_PORT="${RTSP_PORT:-5000}"
PROJECT_REV="$(git -C "$PROJECT_ROOT" rev-parse HEAD 2>/dev/null || true)"
SOURCE_REVISION="${SOURCE_REVISION:-$PROJECT_REV}"
if [[ ! "$RTSP_PORT" =~ ^[0-9]+$ ]] ||
	(( 10#$RTSP_PORT < 1 || 10#$RTSP_PORT > 65535 )); then
	echo "RTSP_PORT must be an integer from 1 through 65535" >&2
	exit 1
fi
if [[ ! "$SOURCE_REVISION" =~ ^[0-9a-f]{7,40}$ ]]; then
	echo "Set SOURCE_REVISION to the exact source commit being built" >&2
	exit 1
fi

export PS3DEV PS3DK PSL1GHT MBEDTLS_INCLUDE MBEDTLS_CRYPTO_LIBRARY
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH"

cmake -S "$MBEDTLS_SOURCE" -B "$MBEDTLS_PPU_BUILD" \
	-DCMAKE_TOOLCHAIN_FILE="$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake" \
	-DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF \
	-DCMAKE_C_FLAGS='-DMBEDTLS_NO_PLATFORM_ENTROPY -DMBEDTLS_PLATFORM_MS_TIME_ALT -Dunix -D_POSIX_VERSION=200809L'
cmake --build "$MBEDTLS_PPU_BUILD" --target mbedcrypto --parallel "$JOBS"

make -C "$PROJECT_ROOT/prototype/standalone-receiver" clean
make -C "$PROJECT_ROOT/prototype/standalone-receiver" \
	CC=powerpc64-ps3-elf-gcc VERBOSE=1 APP_VERSION="$APP_VERSION" \
	RTSP_PORT="$RTSP_PORT"

mkdir -p "$OUTPUT_DIR"
install -m 0644 \
	"$PROJECT_ROOT/prototype/standalone-receiver/ps3-airplay-receiver.elf" \
	"$OUTPUT_DIR/ps3-airplay-receiver.elf"
python3 "$PROJECT_ROOT/scripts/validate-ps3-elf.py" \
	"$OUTPUT_DIR/ps3-airplay-receiver.elf"
PSL1GHT_REV="$(git -C "$PSL1GHT" rev-parse HEAD 2>/dev/null || printf 'unavailable')"
{
	printf 'pipeline=PS3DK-v0.27.0-PPU + PSL1GHT\n'
	printf 'app_version=%s\n' "$APP_VERSION"
	printf 'rtsp_port=%s\n' "$RTSP_PORT"
	printf 'project_revision=%s\n' "$SOURCE_REVISION"
	printf 'psl1ght_revision=%s\n' "$PSL1GHT_REV"
	printf 'ppu_compiler='; powerpc64-ps3-elf-gcc -dumpfullversion
	printf 'ppu_linker='; "$PS3DEV/ppu/bin/ppu-ld" --version | sed -n '1p'
	printf 'elf_sha256='; sha256sum "$OUTPUT_DIR/ps3-airplay-receiver.elf" | awk '{print $1}'
} > "$OUTPUT_DIR/build-info.txt"
cat "$OUTPUT_DIR/build-info.txt"
sha256sum "$OUTPUT_DIR/ps3-airplay-receiver.elf"
