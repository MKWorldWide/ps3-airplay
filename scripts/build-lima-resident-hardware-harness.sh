#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="${PROJECT_ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the pinned PS3DK v0.27.0 source tree}"
PS3DEV="${PS3DEV:-$PS3DK_SOURCE/stage/ps3dev}"
PS3DK="${PS3DK:-$PS3DEV/ps3dk}"
PS3DK_TOOLS="${PS3DK_TOOLS:-$PS3DEV/bin}"
DEPS_ROOT="${DEPS_ROOT:-$HOME/ps3-airplay-host/crypto-deps}"
RESIDENT_BUILD_DIR="${RESIDENT_BUILD_DIR:-$PROJECT_ROOT/build/lima-resident-hardware}"
HARNESS_BUILD_DIR="${HARNESS_BUILD_DIR:-$PROJECT_ROOT/build/lima-resident-hardware-harness}"
TITLE_ID="RPRX90002"
MODULE_NAME="ps3-airplay-receiver.sprx"
MODULE_PATH="/dev_hdd0/game/${TITLE_ID}/USRDIR/${MODULE_NAME}"

export PS3DEV PS3DK PS3DK_SOURCE PS3DK_TOOLS DEPS_ROOT
export PATH="$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH"
cd "$PROJECT_ROOT"

BUILD_DIR="$RESIDENT_BUILD_DIR" ./scripts/build-lima-resident.sh
python3 scripts/validate-resident-containers.py \
	--signed-sprx "$RESIDENT_BUILD_DIR/ps3-airplay-receiver.sprx" \
	--fake-sprx "$RESIDENT_BUILD_DIR/ps3-airplay-receiver.fake.sprx"

cmake -S experimental/resident/harness -B "$HARNESS_BUILD_DIR" \
	-DCMAKE_TOOLCHAIN_FILE="$PS3DK_SOURCE/cmake/ps3-ppu-toolchain.cmake" \
	-DRESIDENT_SPRX="$RESIDENT_BUILD_DIR/ps3-airplay-receiver.sprx" \
	-DHARNESS_TEST_CYCLES=50 \
	-DHARNESS_TITLE_ID="$TITLE_ID" \
	-DHARNESS_RESIDENT_SPRX_NAME="$MODULE_NAME" \
	-DHARNESS_RESIDENT_MODULE_PATH="$MODULE_PATH" \
	-DHARNESS_PKGFILES="$HARNESS_BUILD_DIR/pkg_files"
rm -f "$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg"
cmake --build "$HARNESS_BUILD_DIR" \
	--target ps3-airplay-resident-harness_pkg --parallel "${JOBS:-2}"

"$PS3DK_TOOLS/prx-gen" check \
	"$RESIDENT_BUILD_DIR/ps3-airplay-receiver.prx"

"$PS3DK_TOOLS/pkg" \
	--list "$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg"
VERIFY_DIR="$(mktemp -d "${TMPDIR:-/tmp}/airps3-hardware-pkg.XXXXXX")"
trap 'rm -rf "$VERIFY_DIR"' EXIT
(cd "$VERIFY_DIR" && "$PS3DK_TOOLS/pkg" \
	--extract "$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg")
PACKAGED_SPRX="$(find "$VERIFY_DIR" -type f -name "$MODULE_NAME" -print -quit)"
SPRX_COUNT="$(find "$VERIFY_DIR" -type f -name '*.sprx' | wc -l | tr -d ' ')"
if [[ "$SPRX_COUNT" != 1 || -z "$PACKAGED_SPRX" ]] || ! cmp -s \
	"$RESIDENT_BUILD_DIR/$MODULE_NAME" "$PACKAGED_SPRX"; then
	echo "Hardware test PKG must contain exactly the fresh production SPRX" >&2
	exit 1
fi
if ! grep -Fq "<value name=\"TITLE_ID\" type=\"string\">${TITLE_ID}</value>" \
	"$HARNESS_BUILD_DIR/sfo.xml"; then
	echo "Hardware test SFO title ID does not match the isolated test title" >&2
	exit 1
fi
echo "Hardware test package embeds the fresh signed SPRX: PASS"
sha256sum \
	"$RESIDENT_BUILD_DIR/ps3-airplay-receiver.prx" \
	"$RESIDENT_BUILD_DIR/$MODULE_NAME" \
	"$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.self" \
	"$HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg"
echo "Hardware test title ID: $TITLE_ID"
echo "Installable test package: $HARNESS_BUILD_DIR/ps3-airplay-resident-harness.pkg"
echo "The module is loaded only by the test app; no Cobra autoload is configured."
