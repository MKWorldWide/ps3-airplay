#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
: "${PS3DK_SOURCE:?Set PS3DK_SOURCE to the pinned PS3DK v0.27.0 tree}"
: "${APP_VERSION:?Set the unique app/package version for this build}"
: "${SFO_VERSION:?Set the matching XX.YY SFO version for this build}"

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

export PROJECT_ROOT SOURCE_REVISION="$PROJECT_REV" APP_VERSION SFO_VERSION
bash "$SCRIPT_DIR/build-lima-crypto.sh"
bash "$SCRIPT_DIR/build-lima-standalone-receiver.sh"
bash "$SCRIPT_DIR/package-lima-standalone-receiver.sh"

PACKAGE="$PROJECT_ROOT/build/lima-standalone-package/ps3-airplay-receiver-${APP_VERSION}.pkg"
test -s "$PACKAGE"
sha256sum "$PACKAGE"
wc -c "$PACKAGE"
