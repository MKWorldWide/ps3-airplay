# Resident teardown trace markers

## Timestamp in EDT

2026-11-08 19:07 EDT

## Scope of the pass

Added diagnostics-only runtime trace markers around receiver worker join, audio worker exit, audio thread join, and audio backend cleanup. Rebuilt the PRX and emulator harness package; reran host lifecycle/audio tests. Updated the candidate report and working context.

## Why this pass was needed

The two-port RPCS3 run completed two audio-enabled teardown cycles and crashed in the audio thread during cycle three. The app log ended at the foreground audio submission event and did not show whether the worker exit, join, or resource release was the last completed step.

## Files changed

- `experimental/resident/module.c`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1907_resident_trace_markers.md`

## Architecture / logic changes

No architecture change. A new trace helper emits stdout messages only when test diagnostics are supplied. It shares the log lock and prints milestones around audio worker exit and module-stop stages. Ordinary resident use does not emit these traces.

## Runtime behavior changes

Under the RPCS3 harness only, the resident emits trace messages that can narrow the last reached stage if shutdown fails. Audio behavior and stop ordering are otherwise unchanged.

## Validation performed

- Lima `bash scripts/test-resident-host-lifecycle.sh`: all mocked startup/cleanup scenarios, backend ownership/output checks, and bounded SPSC concurrency tests passed.
- PS3DK 0.27.0 PPU build passed. `prx-gen check` passed with one PT_LOAD, five import libraries, one management export, and 6,298 relocations.
- Removed the previous ignored harness PKG, rebuilt `RPRX90001`, and verified the packaged overlay fake SPRX has the same SHA-256 as the freshly built fake SPRX.
- Candidate hashes: PRX `629f430bf33b5f8020e525cbfac27d25694829133fa835e8b7eb9db3fad976df`; fake SPRX `7e7b0e42d7ac7857bac20691bd979395e525e706b42ab97553ce9c41d70c0d39`; PKG `c91a85999d21c18f2bce244e7c41b2c5e4aa7c5c605bbe202e6aeafb3d1a24ff`.
- RPCS3 install/run was not possible in this pass because the UI reported the Mac locked. The instrumented binary has not run in RPCS3. Previous two-port run remains the last runtime observation.
- No physical console was contacted.

## Immediate results

- PASS: host test suite and PRX structural check.
- PASS: package build and embedded fake-SPRX identity check.
- BLOCKED: RPCS3 rerun pending desktop access.
- FAIL (prior candidate): third audio-enabled cycle faulted before stop/unload/module-count evidence.

## Known limitations / follow-ups

- Trace markers have not yet been observed in RPCS3.
- The prior fatal location remains unsymbolized; do not attribute it to a specific function.
- Audio-enabled repeated unload, AirPlay receive/decode/output, acoustic output, and game coexistence remain unproven.
- Hardware readiness remains NOT READY.

## Risks introduced

The test-only stdout trace shares a short spin/yield lock with event logging. Its lock acquisition is bounded and drops a trace if contended for all attempts. It changes timing slightly, so a passing instrumented run must still be evaluated as a diagnostic observation rather than a timing-identical reproduction.

## Decision record (embedded)

- **Decision:** Add lightweight test-only stage markers and keep the runtime stop sequence unchanged.
- **Context:** The app log stopped before module stop returned, leaving the exact shutdown stage unknown.
- **Options considered:** Change teardown ordering speculatively; replace the worker exit API; or collect stage evidence first.
- **Why this option was chosen:** No root cause has been proven, so observation is safer than altering cleanup semantics without evidence.
- **Tradeoffs accepted:** The markers add a small amount of test-only stdout/timing overhead and do not themselves fix the crash.

## Next recommended step

Once the RPCS3 desktop is unlocked, install the exact `c91a8599...` package, rerun three cycles, save the new TTY output, and correlate the final `AIRPS3_TRACE` event with module stop/unload and module-count results.
