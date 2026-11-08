# Add raw controller-mask fallback for AIRPLAY02 v0.2.2

## Timestamp

2026-11-07 20:08 America/New_York

## Scope of the pass

Extend the standalone input poller to decode PSL1GHT raw digital masks as a fallback, log raw mask changes and count positive reports. Advance the app package label to 0.2.2 and SFO version to 00.42.

## Why this pass was needed

The v0.2.1 log reports a connected pad and a PS menu-open callback, but no START or button transitions. The user reports one successful START exit followed by unresponsive controls after relaunch. Pad info success is not proof fresh button data reached the app.

## Files changed

- `prototype/standalone-receiver/source/main.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v021_controller_diagnostics.md`
- this decision log

## Architecture / logic changes

No new subsystem. Button normalization now accepts either the existing PSL1GHT bitfields or corresponding bits in `padData.button[PAD_BUTTON_OFFSET_DIGITAL1/2]`. Raw digital words are logged only when they change; a bounded count of positive input reports is emitted at each 10-second heartbeat.

## Runtime behavior changes

START exit now has an additive raw-mask path. Package defaults advance to app version 0.2.2 / SFO 00.42. PS-menu behavior remains owned by sysutil.

## Validation performed

- Retrieved and inspected the latest hardware log, preserving raw data only in ignored local work storage.
- Reviewed upstream PSL1GHT `io/pad.h`, which documents zero-length unchanged reports and digital word offsets/masks.
- `git diff --check` and source-level review; cross-build and package validation remain pending in the next build pass.

## Immediate results

No hardware result yet. This change makes missing report input distinguishable from a raw/bitfield mismatch in the next run.

## Known limitations / follow-ups

- Must compile with the PS3DK PPU toolchain and pass the existing host/sanitizer suite.
- Must install and test on PS3; log `PAD_REPORTS`, `PAD_RAW_MASKS`, `PAD_BUTTONS`, `USER_EXIT_START`, and sysutil callbacks separately.
- A successful PS callback does not prove the XMB menu became visible or responsive.

## Risks introduced

The raw masks are ORed with bitfields, so they can only add detected controls; an incorrect mask definition could spuriously activate input. Values come directly from the published PSL1GHT header and will be exercised on the console. Raw logs remain private and are not committed.

## Decision record

### Decision: Add a raw Start-mask fallback instead of replacing bitfield polling

- **Context:** The app exited to XMB once, but later input was absent in the log; the failure may be intermittent or controller reports may not be reaching `ioPadGetData`.
- **Options considered:** Replace field decoding with raw masks; add raw masks and keep fields; make no code change until another user observation.
- **Why this option was chosen:** An additive fallback preserves the observed working behavior while checking an independent representation.
- **Tradeoffs accepted:** This alone cannot fix a disconnected or silent controller; report counters provide the evidence needed to distinguish that case.

## Next recommended step

Run the full Lima host/sanitizer and PPU standalone build, validate a finalized versioned package, then FTP-install and launch v0.2.2 using the standing deployment authorization.
