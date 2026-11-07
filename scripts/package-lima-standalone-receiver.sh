#!/usr/bin/env bash
set -euo pipefail

: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the PS3DK v0.27.0 source tree}"
PROJECT_ROOT="${PROJECT_ROOT:-$(pwd)}"
PS3DEV="${PS3DEV:-$PS3DK_SOURCE/stage/ps3dev}"
PS3DK_TOOLS="${PS3DK_TOOLS:-$HOME/ps3-airplay-host/ps3-sdk-tools-v0.27.0-linux-x86_64/bin}"
ELF="$PROJECT_ROOT/build/lima-standalone-receiver/ps3-airplay-receiver.elf"
OUT_DIR="$PROJECT_ROOT/build/lima-standalone-package"
STAGE="$OUT_DIR/stage"
EXTRACT="$OUT_DIR/extracted"
PACKAGE="$OUT_DIR/ps3-airplay-receiver.pkg"
APPID=AIRPLAY02
APP_VERSION=01.02
CONTENTID="UP0001-${APPID}_00-0000000000000000"
ICON0="$PS3DEV/bin/ICON0.PNG"
BACKGROUND_RGB="${BACKGROUND_RGB:-$PROJECT_ROOT/build/diagnostic-background.rgb}"

for tool in "$PS3DK_TOOLS/make_self_npdrm" "$PS3DK_TOOLS/pkg" \
	"$PS3DEV/bin/sfo"; do
	if [[ ! -x "$tool" ]]; then
		echo "Missing executable: $tool" >&2
		exit 1
	fi
done
if [[ ! -f "$ELF" || ! -f "$PS3DEV/bin/sfo.xml" || ! -f "$ICON0" ||
	! -f "$BACKGROUND_RGB" ]]; then
	echo "Build the standalone ELF and RGB background, and provide the PSL1GHT SFO template and default icon first" >&2
	exit 1
fi
if [[ "$(wc -c < "$BACKGROUND_RGB")" -ne $((1280 * 720 * 3)) ]]; then
	echo "Background RGB asset must be 1280x720 packed RGB" >&2
	exit 1
fi

rm -rf "$STAGE" "$EXTRACT"
rm -f "$PACKAGE"
mkdir -p "$STAGE/USRDIR" "$EXTRACT"
cp "$ICON0" "$STAGE/ICON0.PNG"
cp "$BACKGROUND_RGB" "$STAGE/USRDIR/background.rgb"
sed "s/01\.00/${APP_VERSION}/g" "$PS3DEV/bin/sfo.xml" > "$OUT_DIR/sfo.xml"
# Bind the EBOOT's NPDRM content ID to the same ID used by PARAM.SFO and PKG.
"$PS3DK_TOOLS/make_self_npdrm" "$ELF" "$STAGE/USRDIR/EBOOT.BIN" \
	"$CONTENTID"
"$PS3DEV/bin/sfo" --title='PS3 AirPlay' --appid="$APPID" \
	--fromxml "$OUT_DIR/sfo.xml" "$STAGE/PARAM.SFO"
"$PS3DK_TOOLS/pkg" --contentid="$CONTENTID" "$STAGE" "$PACKAGE"

(
	cd "$EXTRACT"
	"$PS3DK_TOOLS/pkg" --extract "$PACKAGE"
)
EXTRACTED_EBOOT="$EXTRACT/$CONTENTID/USRDIR/EBOOT.BIN"
EXTRACTED_SFO="$EXTRACT/$CONTENTID/PARAM.SFO"
EXTRACTED_ICON="$EXTRACT/$CONTENTID/ICON0.PNG"
if [[ ! -f "$EXTRACTED_EBOOT" || ! -f "$EXTRACTED_SFO" ||
	! -f "$EXTRACTED_ICON" ||
	! -f "$EXTRACT/$CONTENTID/USRDIR/background.rgb" ]]; then
	echo "PKG extract did not contain EBOOT.BIN, PARAM.SFO, ICON0.PNG, and background.rgb" >&2
	exit 1
fi
"$PS3DEV/bin/sfo" --list "$EXTRACTED_SFO"
cmp -n "$(wc -c < "$STAGE/USRDIR/EBOOT.BIN")" \
	"$STAGE/USRDIR/EBOOT.BIN" "$EXTRACTED_EBOOT"
cmp "$STAGE/ICON0.PNG" "$EXTRACTED_ICON"
cmp "$BACKGROUND_RGB" "$EXTRACT/$CONTENTID/USRDIR/background.rgb"
"$PS3DK_TOOLS/pkg" --list "$PACKAGE"
sha256sum "$PACKAGE" "$STAGE/USRDIR/EBOOT.BIN" "$STAGE/USRDIR/background.rgb" "$STAGE/ICON0.PNG"
