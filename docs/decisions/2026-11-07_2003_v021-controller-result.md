# Diagnose inconsistent AIRPLAY02 v0.2.1 controller input

## Timestamp

2026-11-07 20:03 America/New_York

## Scope of the pass

Collected the latest v0.2.1 PS3 diagnostic log after the user's report that START had exited once, then START and PS stopped responding after reopening the app. Updated current project evidence and the next diagnostic step.

## Why this pass was needed

Pad presence and API health do not prove button reports, while the user-visible result changed across launches.

## Files changed

- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v021_controller_diagnostics.md`
- this decision log

## Architecture / logic changes

No runtime code changed. Analysis distinguishes the `ioPadGetInfo` presence result, `ioPadGetData` button transitions, and sysutil PS-menu callbacks.

## Runtime behavior changes

None. The log's current app continues running unless exited by the user or via the known recovery path.

## Validation performed

- Retrieved `/dev_hdd0/tmp/ps3-airplay.log` read-only over FTP; 10,658 bytes, SHA-256 `8fd3a3e922808cca9b5182149f0a1aed6c236770381dea2c7a2a59a64602e8e7`.
- Confirmed one startup, connected pad and initial zero button mask, one `SYSUTIL_MENU_OPEN`, no later button transitions/START exit/menu close/APP_STOP.
- Correlated a completed RTSP setup/record path, ALAC frames, PCM queue/consumption/submission, RTP receive/decrypt/recovery, and client TEARDOWN. These are pipeline facts, not audible-output proof.
- Compared polling semantics with upstream PSL1GHT `io/pad.h`: `ioPadGetData` zero-fills the report when no input changed; raw digital button words are defined at offset 2 with `PAD_CTRL_START` bit 3.

## Immediate results

The PS event reached the application's system-menu callback, but the log cannot establish visible menu response. No START or button transition was observed in the retrieved run. The user also reported one prior successful START exit, so failure is intermittent or run-specific.

## Known limitations / follow-ups

- The latest capped log may not include every previous run; it has one startup and its exact relationship to each user action is limited by available events.
- Add raw digital masks and counts of positive pad reports; use both `BTN_START` and the documented raw Start mask.
- Check whether the menu-open overlay was visibly shown, then test one control on the next build.

## Risks introduced

None. This pass is read-only on the console and contains no source changes.

## Decision record

### Decision: Use raw-mask fallback and add report evidence

- **Context:** PS3 pad info remains healthy, but no button transition is logged. PSL1GHT documents the raw digital word and bit mask as an independent representation.
- **Options considered:** Replace field-based decoding; add raw-mask fallback and diagnostics; treat the event as an external controller fault without a build.
- **Why this option was chosen:** The fallback is additive and bounded, and telemetry will distinguish missing input reports from a decode/mapping mismatch.
- **Tradeoffs accepted:** A hardware retest is still required to prove the controller mapping and visible sysmenu behavior.

## Next recommended step

Build v0.2.2 / SFO `00.42` with bounded pad-report telemetry and raw Start-mask fallback, validate/package it in Lima, then FTP-install and launch under the standing deployment authorization.
