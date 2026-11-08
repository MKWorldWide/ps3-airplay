# Expand diagnostic capacity and gate every package build

**Timestamp:** 2026-11-07 21:05 EDT

## Scope of the pass

Prepare v0.2.5 / SFO 00.45 with a 5 MiB append-only log budget per launch, expand log retention tests to that budget, and add a one-command Lima release gate that always runs host/sanitizer, PPU, ELF, and package validation.

## Why this pass was needed

v0.2.4 reached its 16 KiB session cap about 37 seconds into an active AirPlay run. The user requested a 10-15 minute observation and consistent tests on every future package. The current v0.2.4 artifact remains the functional rollback baseline.

## Files changed

- `src/observability/receiver_log.h`
- `tests/protocol/rtsp_listener_test.c`
- `scripts/release-lima-standalone-receiver.sh`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `prototype/standalone-receiver/Makefile`
- `docs/BUILD_PIPELINE.md`
- `docs/VERSIONING.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v024_build_and_install.md`
- `docs/decisions/2026-11-07_2105_v025_diagnostics_pipeline.md`
- `docs/reports/2026-11-07_v025_release.md`

## Architecture / logic changes

The logger keeps append mode, UTC APP_START timestamps, and per-launch accounting, but raises each run's cap to 5 MiB. The regression writes through the larger allowance, verifies the cap marker and post-cap rejection, and confirms earlier session data remains. The release wrapper rejects dirty tracked source and mismatched revisions, then fail-fast runs the established host/sanitizer suite, PPU app build/ELF check, and PKG/SFO validation.

## Runtime behavior changes

v0.2.5 is intended to retain enough active-stream diagnostics for a 10-15 minute run. A new per-launch cap still stops logging after 5 MiB; total append-only file growth remains proportional to launches. No changes are made to audio handling or the PS3 app currently running.

## Validation performed

- Reviewed the v0.2.4 log: it contains two timestamped APP_START records; prior 7,270-byte log is an exact prefix of the combined file.
- The later session reached RTSP 200, encrypted 44.1 kHz RTP decrypt, ALAC decode, PCM submission, and a 512-packet batch with 62 missing packets, queue depth 79/256, and ALAC failure total zero before the 16 KiB cap at 37 seconds.
- The user confirms PS-menu open, START exit, working streamed music, dim/frozen resume UI, and brief stutters.
- 5 MiB sanitizer regression and complete release wrapper rerun passed on source `9d9f60657f117daedc47c6b1bc3285a5982e5ae3`. Package is 3,232,048 bytes, SHA-256 `61e8310a6d4330bad1e4d64213365f9ba27de81a6fa14851a315f5b338894440`; standalone ELF is 2,754,776 bytes, SHA-256 `1ff2513b9823792aa4f202ddf700e161bee5f223e3ccc2fc19e53e39a3f1eead`.
- First wrapper attempt stopped before packaging because `PS3DEV` was not exported. The wrapper was corrected and the entire gate rerun successfully.
- No package transfer, install, or launch was performed; hardware validation is pending.
- The requested 12-minute passive observation completed without pause, exit, restart, or install commands. FTP snapshots remained byte-identical after the current session cap; they do not prove continued playback after logging stopped.

## Immediate results

The v0.2.5 candidate and its local package are ready; the v0.2.4 source/artifact remain preserved for rollback. The active console app was not interrupted or replaced.

## Known limitations / follow-ups

The 5 MiB target is based on a short observed event rate and should be evaluated against actual 10-15 minute logging. The current running app uses the older 16 KiB cap and will not produce additional log events after its cap. Audio stutter cause and menu-resume dimming are not fixed by this pass.

## Risks introduced

A 5 MiB session can consume more PS3 storage, and each new launch appends another session. The release wrapper intentionally refuses to run on tracked uncommitted changes; build-only ignored artifacts remain permitted. The candidate has not been tested on the physical console.

## Decision record

### Use a 5 MiB per-launch cap

- **Decision:** Set the per-run logging allowance to 5 MiB while retaining append-only history.
- **Context:** The prior 16 KiB cap stopped diagnostics within 37 seconds of an active stream.
- **Options considered:** Keep 16 KiB; raise modestly; set 5 MiB as requested.
- **Why this option was chosen:** It should accommodate the requested duration while keeping a finite bound per run.
- **Tradeoffs accepted:** More storage may be used; total history is not globally bounded.

### Require one release-gate command

- **Decision:** Add a wrapper that runs all existing host/sanitizer, PPU, ELF, and package checks for each release artifact.
- **Context:** A known-good build pipeline now exists and should be consistently used.
- **Options considered:** Keep manually invoking multiple scripts; add an automatic combined gate.
- **Why this option was chosen:** It reduces accidental skipped checks and binds the build to a clean, exact source revision.
- **Tradeoffs accepted:** Release builds stop on dirty tracked changes and require explicit app/SFO versions.

## Next recommended step

Commit and push the build evidence, preserve v0.2.4 as a tagged rollback baseline, and install v0.2.5 only after v0.2.4 is safely back at XMB.
