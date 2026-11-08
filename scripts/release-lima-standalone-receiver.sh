#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the pinned PS3DK v0.27.0 tree}"
CONFIG="$PROJECT_ROOT/scripts/read-release-config.py"
RELEASE_APP_VERSION="$(python3 "$CONFIG" app_version)"
RELEASE_SFO_VERSION="$(python3 "$CONFIG" sfo_version)"
RELEASE_TITLE="$(python3 "$CONFIG" xmb_title)"
RELEASE_FORMAT="$(python3 "$CONFIG" package_format)"
APP_VERSION="${APP_VERSION:-$RELEASE_APP_VERSION}"
SFO_VERSION="${SFO_VERSION:-$RELEASE_SFO_VERSION}"
if [[ "$APP_VERSION" != "$RELEASE_APP_VERSION" || "$SFO_VERSION" != "$RELEASE_SFO_VERSION" ]]; then
	echo "Build versions must match release.json ($RELEASE_APP_VERSION / $RELEASE_SFO_VERSION)" >&2
	exit 1
fi
export XMB_TITLE="${XMB_TITLE:-$RELEASE_TITLE}"
if [[ "$XMB_TITLE" != "$RELEASE_TITLE" ]]; then
	echo "XMB_TITLE must match release.json ($RELEASE_TITLE)" >&2
	exit 1
fi
PS3DEV="${PS3DEV:-$PS3DK_SOURCE/stage/ps3dev}"

if git -C "$PROJECT_ROOT" rev-parse --verify HEAD >/dev/null 2>&1; then
	PROJECT_REV="$(git -C "$PROJECT_ROOT" rev-parse HEAD)"
	if [[ -n "$(git -C "$PROJECT_ROOT" status --porcelain --untracked-files=no)" ]]; then
		echo "Refusing release build from a dirty tracked worktree" >&2
		exit 1
	fi
elif [[ -f "$PROJECT_ROOT/.source-revision" ]]; then
	PROJECT_REV="$(cat "$PROJECT_ROOT/.source-revision")"
else
	echo "Release build requires a Git checkout or stamped source archive" >&2
	exit 1
fi

if [[ ! "$PROJECT_REV" =~ ^[0-9a-f]{40}$ ||
	"${SOURCE_REVISION:-$PROJECT_REV}" != "$PROJECT_REV" ]]; then
	echo "SOURCE_REVISION must match the exact committed or stamped source" >&2
	exit 1
fi
if [[ ! "$APP_VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
	echo "APP_VERSION must use semantic version syntax, for example 0.2.5" >&2
	exit 1
fi
if [[ ! "$SFO_VERSION" =~ ^[0-9]{2}\.[0-9]{2}$ ]]; then
	echo "SFO_VERSION must use XX.YY format" >&2
	exit 1
fi

export PROJECT_ROOT PS3DEV SOURCE_REVISION="$PROJECT_REV" APP_VERSION SFO_VERSION
export PACKAGE_FORMAT="${PACKAGE_FORMAT:-$RELEASE_FORMAT}"
if [[ "$PACKAGE_FORMAT" != "$RELEASE_FORMAT" ]]; then
	echo "PACKAGE_FORMAT must match release.json ($RELEASE_FORMAT)" >&2
	exit 1
fi
python3 "$SCRIPT_DIR/test-release-config.py"
python3 "$SCRIPT_DIR/test-release-evidence.py"
bash "$SCRIPT_DIR/build-lima-crypto.sh"
MBEDTLS_INCLUDE="${DEPS_ROOT:-$HOME/ps3-airplay-host/crypto-deps}/src/mbedtls-3.6.7/include" \
 bash "$SCRIPT_DIR/run-forensic-tests.sh"
bash "$SCRIPT_DIR/build-lima-standalone-receiver.sh"
python3 "$SCRIPT_DIR/prepare-diagnostic-background.py" \
	"$PROJECT_ROOT/assets/air-ps3/artwork-source.png" \
	"$PROJECT_ROOT/build/diagnostic-background.rgb"
python3 "$SCRIPT_DIR/validate-air-ps3-artwork.py" "$PROJECT_ROOT/assets/air-ps3"
bash "$SCRIPT_DIR/package-lima-standalone-receiver.sh"

PACKAGE="$PROJECT_ROOT/build/lima-standalone-package/ps3-airplay-receiver-${APP_VERSION}.pkg"
test -s "$PACKAGE"
sha256sum "$PACKAGE"
wc -c "$PACKAGE"
python3 "$SCRIPT_DIR/write-release-evidence.py" \
	--project-root "$PROJECT_ROOT" --package "$PACKAGE" \
	--source-revision "$PROJECT_REV" --app-version "$APP_VERSION" \
	--sfo-version "$SFO_VERSION" --package-format "$PACKAGE_FORMAT" \
	--output "$PROJECT_ROOT/build/lima-standalone-package/release-evidence.json"
python3 "$SCRIPT_DIR/verify-release-evidence.py" \
	--package "$PACKAGE" \
	--evidence "$PROJECT_ROOT/build/lima-standalone-package/release-evidence.json" \
	--expected-source "$PROJECT_REV" \
	--expected-sha256 "$(sha256sum "$PACKAGE" | awk '{print $1}')" \
	--release-config "$PROJECT_ROOT/release.json"
(cd "$PROJECT_ROOT/build/lima-standalone-package" && sha256sum -c SHA256SUMS.txt)
