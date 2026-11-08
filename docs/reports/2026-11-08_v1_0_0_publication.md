# AIR PS3 v1.0.0 publication verification

**Date:** 2026-11-08 04:51 EDT
**GitHub Release:** https://github.com/MKWorldWide/ps3-airplay/releases/tag/v1.0.0
**Workflow run:** 42468858833

## Published artifact identity

- App version: `1.0.0`
- SFO `APP_VER` and `VERSION`: `01.00`
- Title ID: `AIRPLAY02`
- XMB title: `AIR PS3`
- AirPlay receiver name: `AirPS3`
- Package type: PS3 type-1 debug/homebrew PKG
- Source tag and commit: `v1.0.0` → `e1fdb44b03e2beb048954aded0eda5ae11248215`
- Package: `ps3-airplay-receiver-1.0.0.pkg`
- Package size: 3,906,640 bytes
- Package SHA-256: `b815536168c5c1994ff6a5bb79f4aa16ffed6720691239bd9066e0a9e2075c43`

## Publication and artifact checks

Workflow run `42468858833` passed tag/source verification, draft release lookup, the exact expected ten-file asset inventory, uploaded-state and byte-size checks, `SHA256SUMS.txt`, release-evidence validation, package SHA-256 verification, and the separate publish job. The GitHub release now reports `isDraft=false` and tag `v1.0.0`.

After publication, the PKG, checksum manifest, and release evidence were downloaded from the public release. The PKG matched the expected SHA-256, and `shasum -a 256 -c SHA256SUMS.txt` passed for the downloaded PKG and evidence file.

The complete pinned-Lima host/sanitizer, PPU, ELF/SELF, SFO, artwork, package extraction/envelope, and release-evidence gate had passed for the exact tagged source before staging. Workflow validation independently tied the published package and evidence to that source.

The release contains ten expected assets: the PKG, release evidence, checksum manifest, release notes, installation guide, third-party notices, and four license texts. The README links to the latest release, the direct v1.0.0 PKG, and its checksum file.

## Documentation and issue tracker

The README and installation/docs links were verified against the published URLs. Gate 1, roadmap, test matrix, and public release plan now show the actual published state and preserve the distinction between host validation and user-reported hardware results. Earlier issue #1 and #2 closures remain bounded to historical startup/tone/idle-exit evidence; #3–#6 remain open for evidence not yet established.

## Hardware boundary

No PS3 or RPCS3 operation occurred during publication. The maintainer's report that v1.0.0 is “running beautifully” remains qualitative user-reported evidence. This publication does not add a duration, fresh log analysis, or individually confirmed playback/control results.

## Failed workflow attempts and correction

Runs `42468111811` and `42468417377` failed at `gh release view` with `release not found`. Run `42468710312` then returned no draft records because the validation job had only `contents: read`. GitHub documents that draft release listings require push access. The validator was granted `contents: write` for GET-only release/asset inspection, while the separate publish job retained responsibility for changing draft state. Run `42468858833` passed end to end.

## Remaining limitations

- This is a debug/homebrew package for compatible CFW environments; retail firmware installation is not promised.
- Hardware evidence and limitations remain as described in the release notes and test matrix; publication is not new hardware evidence.
- `contents: write` on the validation job is broader than its GET-only API use. It is scoped to the manual validation job because GitHub's draft listing requires push access.
