#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TAG="v$(python3 "$SCRIPT_DIR/read-release-config.py" app_version)"
PKG="${1:?Usage: stage-github-release.sh /path/to/validated.pkg}"
NOTES="$ROOT/docs/RELEASE_NOTES-v1.0.0.md"
EVIDENCE="$(dirname "$PKG")/release-evidence.json"
SUMS="$(dirname "$PKG")/SHA256SUMS.txt"
GATE_DOC="$ROOT/docs/RELEASE_GATE_1.md"

if ! rg -q '^\*\*Release gate status: PASS\*\*$' "$GATE_DOC"; then
	echo "Refusing to create a release tag or draft: docs/RELEASE_GATE_1.md is not PASS" >&2
	exit 1
fi
if rg -q '\| (PENDING|BLOCKED) \|' "$GATE_DOC"; then
	echo "Refusing to stage while a release-gate row is pending or blocked" >&2
	exit 1
fi
if [[ -n "$(git -C "$ROOT" status --porcelain)" ]]; then
	echo "Refusing release staging from a dirty worktree" >&2
	exit 1
fi
SOURCE="$(git -C "$ROOT" rev-parse HEAD)"
git -C "$ROOT" fetch origin main
if [[ "$SOURCE" != "$(git -C "$ROOT" rev-parse origin/main)" ]]; then
	echo "Refusing release staging: source SHA must be the current origin/main tip" >&2
	exit 1
fi
if git -C "$ROOT" show-ref --verify --quiet "refs/tags/$TAG"; then
	echo "Refusing to reuse existing tag $TAG" >&2
	exit 1
fi
if gh release view "$TAG" -R MKWorldWide/ps3-airplay >/dev/null 2>&1; then
	echo "A GitHub release already exists for $TAG" >&2
	exit 1
fi
python3 "$SCRIPT_DIR/verify-release-evidence.py" \
	--package "$PKG" --evidence "$EVIDENCE" \
	--expected-source "$SOURCE" \
	--expected-sha256 "$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["artifacts"]["package"]["sha256"])' "$EVIDENCE")" \
	--release-config "$ROOT/release.json"
python3 - "$SUMS" <<'PY'
import hashlib, pathlib, sys
manifest=pathlib.Path(sys.argv[1])
for line in manifest.read_text().splitlines():
    expected, name = line.split(None, 1)
    path=manifest.parent/name.strip()
    actual=hashlib.sha256(path.read_bytes()).hexdigest()
    if actual != expected:
        raise SystemExit(f"checksum mismatch: {path.name}")
print("package/evidence checksums passed")
PY
gh release create "$TAG" "$PKG" "$EVIDENCE" "$SUMS" "$NOTES" \
	"$ROOT/docs/INSTALLATION.md" "$ROOT/THIRD_PARTY_NOTICES.md" \
	"$ROOT/licenses/MBED-TLS-3.6.7-LICENSE.txt" \
	"$ROOT/licenses/PS3DK-LICENSE.txt" "$ROOT/licenses/PSL1GHT-LICENSE.txt" \
	"$ROOT/licenses/PSL1GHT-RSX-LICENSE.txt" \
	-R MKWorldWide/ps3-airplay --draft --title "AIR PS3 $TAG" \
	--target "$SOURCE" --notes-file "$NOTES"
echo "Created a draft release after all local validation; publication remains a separate workflow dispatch."
