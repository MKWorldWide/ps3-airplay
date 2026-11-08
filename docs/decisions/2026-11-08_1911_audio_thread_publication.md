# Atomically publish resident audio-thread readiness

## Timestamp in EDT

2026-11-08 19:11 EDT

## Scope of the pass

Changed `audio_thread_started` to use atomic release/acquire operations across module startup/stop and the receiver worker. Rebuilt the PRX and RPCS3 package, reran host tests, and refreshed project documentation.

## Why this pass was needed

While preparing an instrumented RPCS3 rerun, source review found the receiver worker read `audio_thread_started` while module startup and stop callbacks wrote it as a plain integer. That is a cross-thread data race and could make the PCM queue handoff unreliable.

## Files changed

- `experimental/resident/module.c`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1911_audio_thread_publication.md`

## Architecture / logic changes

The queue is initialized before publishing audio-thread readiness. Startup release-stores the state immediately before thread creation; creation failure release-stores false. The receiver worker acquire-loads the flag before enqueueing decoded PCM. Stop acquire-loads before waiting for worker exit, and clears it with a release store after join.

## Runtime behavior changes

The receiver worker now has a defined atomic handoff for whether it may write decoded PCM to the queue. If audio worker creation fails, the receiver observes the disabled state and skips queue writes. No RAOP protocol or output behavior was otherwise changed.

## Validation performed

- Lima `bash scripts/test-resident-host-lifecycle.sh`: all mocked startup/cleanup, audio ownership/output, SPSC bounds, and million-frame concurrent ordering tests passed.
- PS3DK 0.27.0 PPU build passed. `prx-gen check` passed with one PT_LOAD, five imports, one management export, and 6,298 relocations.
- Removed the prior ignored PKG output, rebuilt `RPRX90001`, and verified the overlay fake-SPRX hash matches the fresh fake-SPRX build.
- Latest hashes: PRX `5ebf2ab703df8986d3a62864db03adcc9989d3eb9b39b6507eacce0865fe7355`; fake SPRX `e4507877dce48fcf4f7874de352e90ead57c283a84a47fc66b4504603ddb9c14`; PKG `d803bfd2daf94e6dbf7a61434330077f5e5b55418831d389a12ffbf0fb888a3c`.
- RPCS3 install/run remains blocked because the UI reported the Mac locked; the atomic change and current trace markers have not been exercised in the emulator.
- No physical PS3 was contacted.

## Immediate results

- PASS: host lifecycle/audio/queue tests.
- PASS: PPU build, PRX structural validation, and fresh package payload identity.
- BLOCKED: RPCS3 rerun pending desktop access.
- FAIL (earlier candidate): cycles 1–2 completed, cycle 3 faulted before stop/unload/module-count evidence.

## Known limitations / follow-ups

- Atomic publication removes one identified race but is not a diagnosis or fix for the third-cycle crash.
- Run the latest package in RPCS3, capture trace markers, and verify three clean cycles with module count baseline after each.
- Real RAOP decoded playback, acoustic output, game coexistence, and physical compatibility remain untested.

## Risks introduced

The flag is set just before `sysThreadCreate`; if creation fails, it is reset immediately. During that short interval the receiver could enqueue PCM into the already-initialized queue before the failed creation is observed. The queue remains bounded; no consumer is assumed until thread creation succeeds.

## Decision record (embedded)

- **Decision:** Publish worker readiness with release/acquire atomics instead of a plain shared integer.
- **Context:** The receiver thread uses this flag to decide whether decoded PCM can be enqueued.
- **Options considered:** Leave the flag plain, protect every read/write with the existing log lock, or use a dedicated atomic flag.
- **Why this option was chosen:** A dedicated atomic expresses the cross-thread state transition directly without coupling queue ownership to logging.
- **Tradeoffs accepted:** Thread creation failure may briefly expose a true flag; the initialized queue safely accepts bounded frames during that interval, and failure resets the flag.

## Next recommended step

Install the exact latest `RPRX90001` PKG after RPCS3 desktop access returns. Check whether trace output identifies the third-cycle stop stage, and require three complete audio-enabled unload cycles with module count six before proceeding to synthetic RAOP playback.
