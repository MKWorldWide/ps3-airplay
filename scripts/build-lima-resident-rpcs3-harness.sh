#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="${PROJECT_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the PS3DK v0.27.0 source directory}"
PS3DEV="${PS3DEV:-$PS3DK_SOURCE/stage/ps3dev}"
PS3DK="${PS3DK:-$PS3DEV/ps3dk}"
PS3DK_TOOLS="${PS3DK_TOOLS:-$HOME/ps3-airplay-host/ps3-sdk-tools-v0.27.0-linux-x86_64/bin}"
DEPS_ROOT="${DEPS_ROOT:-$HOME/ps3-airplay-host/crypto-deps}"
RESIDENT_LOG_PATH="${RESIDENT_LOG_PATH:-/dev_hdd0/tmp/ps3-airplay-resident-test.log}"
RESIDENT_RTSP_PORT="${RESIDENT_RTSP_PORT:-5001}"
RESIDENT_RTP_BASE_PORT="${RESIDENT_RTP_BASE_PORT:-6100}"
RESIDENT_BUILD_DIR="${RESIDENT_BUILD_DIR:-$PROJECT_ROOT/build/lima-resident-rpcs3}"
HARNESS_BUILD_DIR="${HARNESS_BUILD_DIR:-$PROJECT_ROOT/build/lima-resident-rpcs3-harness}"

export PS3DEV PS3DK PS3DK_SOURCE DEPS_ROOT RESIDENT_LOG_PATH
export RESIDENT_RTSP_PORT RESIDENT_RTP_BASE_PORT BUILD_DIR="$RESIDENT_BUILD_DIR"
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin:$PATH"

"$PROJECT_ROOT/scripts/build-lima-resident.sh"

cmake -S "$PROJECT_ROOT/experimental/resident/harness" \
	-B "$HARNESS_BUILD_DIR" \
	-DCMAKE_TOOLCHAIN_FILE="$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake" \
	-DRESIDENT_SPRX="$RESIDENT_BUILD_DIR/ps3-airplay-receiver.fake.sprx" \
	-DHARNESS_PKGFILES="$HARNESS_BUILD_DIR/pkg_files"
# The SDK helper checks PKGFILES at configure time but omits overlay files from
# the generated package command's dependencies. Force a fresh package so every
# PRX rebuild is actually reflected in the artifact.
rm -f "$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg"
cmake --build "$HARNESS_BUILD_DIR" \
	--target ps3-airplay-resident-harness_pkg --parallel "${JOBS:-2}"

"$PS3DEV/bin/prx-gen" check "$RESIDENT_BUILD_DIR/ps3-airplay-receiver.prx"
"$PS3DK_TOOLS/pkg" \
	--list "$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg"
VERIFY_DIR="$(mktemp -d "${TMPDIR:-/tmp}/ps3-airplay-resident-pkg.XXXXXX")"
trap 'rm -rf "$VERIFY_DIR"' EXIT
(cd "$VERIFY_DIR" && "$PS3DK_TOOLS/pkg" \
	--extract "$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg")
PACKAGED_SPRX="$(find "$VERIFY_DIR" -type f \
	-name ps3-airplay-receiver.fake.sprx -print -quit)"
if [[ -z "$PACKAGED_SPRX" ]] || ! cmp -s \
	"$RESIDENT_BUILD_DIR/ps3-airplay-receiver.fake.sprx" "$PACKAGED_SPRX"; then
	echo "The PKG does not embed the current fake-signed resident SPRX" >&2
	exit 1
fi
echo "PKG embeds the current fake-signed resident SPRX: PASS"
sha256sum \
	"$RESIDENT_BUILD_DIR/ps3-airplay-receiver.prx" \
	"$RESIDENT_BUILD_DIR/ps3-airplay-receiver.sprx" \
	"$RESIDENT_BUILD_DIR/ps3-airplay-receiver.fake.sprx" \
	"$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.elf" \
	"$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.self" \
	"$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg"
