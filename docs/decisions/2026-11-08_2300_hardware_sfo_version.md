# Correct the resident hardware harness SFO version

## Timestamp in EDT

2026-11-08 23:00 EDT

## Scope of the pass

Change both PS3 SFO version fields for the separate hardware diagnostic title
from `0.1.0` to the requested fixed-width `00.01`, rebuild the package, and
independently inspect its embedded `PARAM.SFO`.

## Why this pass was needed

The user reported the same launch error as a previous invalid-version case
after installing the first `RPRX90002` package and directed that the SFO show
version `00.01`.

## Files changed

- `experimental/resident/harness/sfo.xml`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_2300_hardware_sfo_version.md`

## Architecture / logic changes

The harness source now writes `00.01` to both `APP_VER` and `VERSION`. The
title ID remains `RPRX90002`; the build still embeds the same freshly built
production signed SPRX. The package is a replacement update for the isolated
diagnostic title and does not affect AIRPLAY02 or Cobra settings.

## Runtime behavior changes

The installed title reports fixed-width PS3 SFO version `00.01`. No resident
module behavior or audio/network code changed.

## Validation performed

- Reconfigured and rebuilt the `RPRX90002` package using the pinned PS3DK PPU
  toolchain — PASS.
- Extracted `PARAM.SFO` from the generated package and parsed the binary fields
  independently — PASS: `TITLE_ID=RPRX90002`, `APP_VER=00.01`, `VERSION=00.01`.
- Corrected package SHA-256:
  `54d5d299c6831a18e7c0474aac3577a8a02de443a7a3b98e03f64d242b9010b9`.
- Previous console installation's SPRX matched the production signed build
  byte for byte. The first launch attempt did not produce a retrieved harness
  log; the user reported the version-related launch error.
- Corrected package install and launch — UNTESTED at this pass boundary.

## Immediate results

The corrected package contains the requested SFO values and is ready to
replace the first test build. The resident binary did not change. The previous
package remains staged on the console and installed under the same diagnostic
title ID until the corrected package is installed.

## Known limitations / follow-ups

- Installing the corrected package and confirming its launch are still
  required.
- The 50-cycle harness has not yet produced a retrieved log on the PS3.
- RAOP input, valid resident PCM from a real stream, audible sound, game audio
  coexistence, and Cobra compatibility remain untested.

## Risks introduced

- The package updates the existing `RPRX90002` title. Its SFO version changes
  from `0.1.0` to `00.01`; XMB installation and app startup are the next
  validation points.
- No boot plugin, automatic startup, stable release, or standalone receiver
  was modified.

## Decision record (embedded)

### Decision: use `00.01` in both SFO version fields

- **Context:** The user identified a prior launch error associated with a bad
  version string and specified the expected SFO value.
- **Options considered:** Change only `APP_VER`; change only `VERSION`; set
  both fields to `00.01`.
- **Why this option was chosen:** Both fields describe the title's version in
  the generated package, so setting both avoids a split identity.
- **Tradeoffs accepted:** The diagnostic title version no longer follows a
  semantic-version format; it follows the PS3 fixed-width value requested for
  this loader test.

## Next recommended step

Commit and push this SFO correction to `experimental/resident-airplay`, upload
the corrected PKG under a new filename, install it as the `RPRX90002` update,
launch the title, and retrieve its verbose log before interpreting lifecycle
or audio results.
