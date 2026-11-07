#!/usr/bin/env bash
set -euo pipefail

: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the PS3DK v0.27.0 source tree}"
: "${PS3DEV:?Set PS3DEV to a PSL1GHT-compatible PS3DEV tree for SFO/icon assets}"
PROJECT_ROOT="${PROJECT_ROOT:-$(pwd)}"
PS3DK_TOOLS="${PS3DK_TOOLS:-$HOME/ps3-airplay-host/ps3-sdk-tools-v0.27.0-linux-x86_64/bin}"
SELF_TOOL="${SELF_TOOL:-$PS3DK_TOOLS/make_self_npdrm}"
PKG_TOOL="${PKG_TOOL:-$PS3DK_TOOLS/pkg}"
PACKAGE_FINALIZE="${PACKAGE_FINALIZE:-$PS3DK_TOOLS/package_finalize}"
ELF="${ELF:-$PROJECT_ROOT/build/lima-standalone-receiver/ps3-airplay-receiver.elf}"
OUT_DIR="${OUT_DIR:-$PROJECT_ROOT/build/lima-standalone-package}"
APPID=AIRPLAY02
APP_VERSION="${APP_VERSION:-0.1.22}"
CONTENTID="UP0001-${APPID}_00-0000000000000000"
STAGE="$OUT_DIR/$CONTENTID"
EXTRACT="$OUT_DIR/extracted"
PACKAGE="$OUT_DIR/ps3-airplay-receiver-${APP_VERSION}.pkg"
UNFINALIZED_PACKAGE="$PACKAGE.unfinalized.pkg"
ICON0="$PS3DEV/bin/ICON0.PNG"
BACKGROUND_RGB="${BACKGROUND_RGB:-$PROJECT_ROOT/build/diagnostic-background.rgb}"

for tool in "$SELF_TOOL" "$PKG_TOOL" "$PACKAGE_FINALIZE" "$PS3DEV/bin/sfo"; do
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
if [[ -e "$PACKAGE" ]]; then
	echo "Refusing to overwrite existing versioned package: $PACKAGE" >&2
	exit 1
fi
if [[ -e "$UNFINALIZED_PACKAGE" ]]; then
	echo "Refusing to overwrite existing intermediate package: $UNFINALIZED_PACKAGE" >&2
	exit 1
fi
if [[ "$(wc -c < "$BACKGROUND_RGB")" -ne $((1280 * 720 * 3)) ]]; then
	echo "Background RGB asset must be 1280x720 packed RGB" >&2
	exit 1
fi

python3 "$PROJECT_ROOT/scripts/validate-ps3-elf.py" "$ELF"

rm -rf "$STAGE" "$EXTRACT"
mkdir -p "$STAGE/USRDIR" "$EXTRACT"
cp "$ICON0" "$STAGE/ICON0.PNG"
cp "$BACKGROUND_RGB" "$STAGE/USRDIR/background.rgb"
sed "s/01\.00/${APP_VERSION}/g" "$PS3DEV/bin/sfo.xml" > "$OUT_DIR/sfo.xml"
# Bind the EBOOT's NPDRM content ID to the same ID used by PARAM.SFO and PKG.
"$SELF_TOOL" "$ELF" "$STAGE/USRDIR/EBOOT.BIN" \
	"$CONTENTID"
"$PS3DEV/bin/sfo" --title='PS3 AirPlay' --appid="$APPID" \
	--fromxml "$OUT_DIR/sfo.xml" "$STAGE/PARAM.SFO"
"$PKG_TOOL" --contentid="$CONTENTID" "$STAGE" "$UNFINALIZED_PACKAGE"

(
	cd "$EXTRACT"
	"$PKG_TOOL" --extract "$UNFINALIZED_PACKAGE"
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
SFO_OUTPUT="$("$PS3DEV/bin/sfo" --list "$EXTRACTED_SFO")"
printf '%s\n' "$SFO_OUTPUT"
if ! printf '%s\n' "$SFO_OUTPUT" | grep -F "'APP_VER': '${APP_VERSION}'" >/dev/null; then
	echo "Extracted SFO does not report APP_VER ${APP_VERSION}" >&2
	exit 1
fi
cmp -n "$(wc -c < "$STAGE/USRDIR/EBOOT.BIN")" \
	"$STAGE/USRDIR/EBOOT.BIN" "$EXTRACTED_EBOOT"
cmp "$STAGE/ICON0.PNG" "$EXTRACTED_ICON"
cmp "$BACKGROUND_RGB" "$EXTRACT/$CONTENTID/USRDIR/background.rgb"
PKG_LIST="$("$PKG_TOOL" --list "$UNFINALIZED_PACKAGE")"
printf '%s\n' "$PKG_LIST"
if printf '%s\n' "$PKG_LIST" | awk '/(raw data|directory|NPDRM SELF):/ && $NF ~ /^\// { bad=1 } END { exit !bad }'; then
	echo "PKG contains absolute entry paths; refusing to publish this package" >&2
	exit 1
fi
cp "$UNFINALIZED_PACKAGE" "$PACKAGE"
"$PACKAGE_FINALIZE" "$PACKAGE"
python3 "$PROJECT_ROOT/scripts/validate-ps3-pkg.py" "$PACKAGE" "$CONTENTID"
sha256sum "$UNFINALIZED_PACKAGE" "$PACKAGE" \
	"$STAGE/USRDIR/EBOOT.BIN" "$STAGE/USRDIR/background.rgb" "$STAGE/ICON0.PNG"
