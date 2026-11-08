# Log worker completion on early receiver exit

## Timestamp in EDT

2026-11-08 07:34 EDT

## Scope of the pass

Added a host-mock receiver-open failure case, then made the resident worker emit `WORKER_STOPPED` on receiver-open and receiver-ready logging failures before publishing its exited flag. Rebuilt the PRX and diagnostic PKG and refreshed candidate hashes and validation notes.

## Why this pass was needed

The new failure test timed out waiting for the worker-stop event. Inspection showed the worker did terminate, but its two early-exit paths omitted the diagnostic event used by the RPCS3 harness and test evidence. This made a clean early exit indistinguishable from a missing log event.

## Files changed

- `experimental/resident/module.c`
- `tests/resident/host_lifecycle_test.c`
- `experimental/resident/README.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0734_worker_exit_diagnostics.md`

## Architecture / logic changes

- Both early worker exits now log `WORKER_STOPPED` with value `-1` before setting `worker_exited` and calling `sysThreadExit`.
- Normal worker cleanup retains its existing `WORKER_STOPPED` event.
- The host mock can fail receiver open. The test verifies the failure event, worker-stop event, successful join, and network resource cleanup.

## Runtime behavior changes

Diagnostic logs now report worker completion consistently for early receiver startup failures. Receiver/network behavior is unchanged.

## Validation performed

- Four consecutive full AddressSanitizer/UndefinedBehaviorSanitizer lifecycle simulations passed; each exercised ten lifecycle/fault cases, including receiver-open failure.
- Full Lima host regression and build/package script passed.
- `prx-gen check` passed; the diagnostic harness PKG was extracted and its PRX matched the just-built PRX byte for byte.
- Latest candidate hashes were refreshed in `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`.
- RPCS3 remained unrun because the desktop is locked. No physical PS3 was contacted.

## Immediate results

- The early-exit event omission was reproduced by a test and fixed.
- The module and harness build cleanly with the event change.
- Hardware readiness remains NOT READY pending real emulator lifecycle evidence.

## Known limitations / follow-ups

- Mock APIs cannot establish PS3 scheduler, syscall, loader, join, or network behavior.
- The RPCS3 app still must be installed and run through all three lifecycle cycles after desktop access is restored.
- No resident PCM output or game coexistence is implemented.

## Risks introduced

- The extra event adds a bounded file-log write on early worker exit. If the log is unavailable or the lock cannot be acquired, event logging may fail; the exit flag and thread termination still proceed.
- `sysThreadJoin` remains without an API timeout, but is called only after `worker_exited` is published.

## Decision record (embedded)

### Decision: add completion diagnostics rather than weaken the test

- **Context:** The worker exited cleanly on receiver-open failure, but omitted a lifecycle event consumed by the harness.
- **Options considered:** Remove the event assertion; teach the harness to infer termination from stop success; or make early exits report the same completion event as normal cleanup.
- **Why chosen:** Consistent worker completion evidence helps diagnose future RPCS3 failures and keeps the harness event contract simple.
- **Tradeoffs accepted:** This slightly expands production logging. It does not change protocol or audio behavior.

## Next recommended step

When RPCS3 UI access is available, install only the isolated `RPRX90001` diagnostic package, run the three load/start/stop/unload cycles, and inspect worker-stop events and module-count baseline results before any hardware proposal.
