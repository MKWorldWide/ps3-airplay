#!/usr/bin/env bash
set -euo pipefail

: "${PS3DEV:?Set PS3DEV to the staged PS3DK toolchain directory}"
: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the PS3DK v0.27.0 source directory}"

PS3DK="${PS3DK:-$PS3DEV/ps3dk}"
PPU_CC="$PS3DEV/ppu/bin/powerpc64-ps3-elf-gcc"
PRX_GEN="$PS3DEV/bin/prx-gen"
BUILD_DIR="${BUILD_DIR:-$(pwd)/build/lima-smoke}"
RUNTIME_DIR="$PS3DK_SOURCE/runtime/lv2/crt"
SAMPLE_DIR="$PS3DK_SOURCE/samples/lv2/hello-sprx-export"

for required in "$PPU_CC" "$PRX_GEN" "$RUNTIME_DIR/lv2-prx.S" \
	"$RUNTIME_DIR/lv2-prx.ld" "$SAMPLE_DIR/source/export.c" \
	"$SAMPLE_DIR/source/exports.S"; do
	if [[ ! -f "$required" ]]; then
		printf 'Required PS3DK file is missing: %s\n' "$required" >&2
		exit 1
	fi
done

mkdir -p "$PS3DK/ppu/lib" "$BUILD_DIR"

# Rebuild the PRX CRT inputs used by this isolated smoke target. The full
# runtime and SDK are installed separately in the Lima guest; this script keeps
# the sample build independent of the rest of the SDK's link libraries.
"$PPU_CC" -c -mcpu=cell -o "$PS3DK/ppu/lib/lv2-prx-crt.o" \
	"$RUNTIME_DIR/lv2-prx.S"
install -m 0644 "$RUNTIME_DIR/lv2-prx.ld" "$PS3DK/ppu/lib/lv2-prx.ld"
install -m 0644 "$RUNTIME_DIR/lv2-prx.specs" "$PS3DK/ppu/lib/lv2-prx.specs"

"$PPU_CC" -mcpu=cell -mhard-float -fmodulo-sched \
	-ffunction-sections -fdata-sections -std=c11 -Wall -Wextra -Werror \
	-Isrc/protocol -Isrc/observability \
	-c src/protocol/rtsp_diagnostic.c \
	-o "$BUILD_DIR/rtsp_diagnostic.o"
"$PPU_CC" -mcpu=cell -mhard-float -fmodulo-sched \
	-ffunction-sections -fdata-sections -std=c11 -Wall -Wextra -Werror \
	-Isrc/protocol -Isrc/observability -I"$PS3DK_SOURCE/sdk/include" \
	-c src/protocol/rtsp_listener.c \
	-o "$BUILD_DIR/rtsp_listener.o"
"$PPU_CC" -mcpu=cell -mhard-float -fmodulo-sched \
	-ffunction-sections -fdata-sections -std=c11 -Wall -Wextra -Werror \
	-Isrc/protocol -Isrc/observability \
	-c src/observability/receiver_log.c \
	-o "$BUILD_DIR/receiver_log.o"

"$PPU_CC" -mcpu=cell -mhard-float -fmodulo-sched \
	-ffunction-sections -fdata-sections -std=c11 \
	-I"$PS3DK_SOURCE/sdk/include" -I"$SAMPLE_DIR/include" \
	-c "$SAMPLE_DIR/source/export.c" -o "$BUILD_DIR/export.o"
"$PPU_CC" -mcpu=cell -mhard-float -fmodulo-sched \
	-ffunction-sections -fdata-sections \
	-I"$PS3DK_SOURCE/sdk/include" \
	-c "$SAMPLE_DIR/source/exports.S" -o "$BUILD_DIR/exports.o"

"$PPU_CC" -specs="$PS3DK/ppu/lib/lv2-prx.specs" \
	-nostartfiles -nostdlib -mcpu=cell -mhard-float -fmodulo-sched \
	-ffunction-sections -fdata-sections -Wl,-q \
	-Wl,--no-warn-rwx-segments -Wl,-T,"$PS3DK/ppu/lib/lv2-prx.ld" \
	"$PS3DK/ppu/lib/lv2-prx-crt.o" \
	"$BUILD_DIR/export.o" "$BUILD_DIR/exports.o" \
	-o "$BUILD_DIR/hello-sprx-export.elf"

"$PRX_GEN" build "$BUILD_DIR/hello-sprx-export.elf" \
	-o "$BUILD_DIR/hello-sprx-export.prx" --name hello_prx --version 1.0
"$PRX_GEN" check "$BUILD_DIR/hello-sprx-export.prx"
file "$BUILD_DIR"/rtsp_diagnostic.o "$BUILD_DIR"/receiver_log.o \
	"$BUILD_DIR"/rtsp_listener.o "$BUILD_DIR"/hello-sprx-export.elf \
	"$BUILD_DIR"/hello-sprx-export.prx
