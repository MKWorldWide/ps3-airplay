# Audio worker stack correction and RPCS3 retest

## Timestamp in EDT

2026-11-08 19:05 EDT

## Scope of the pass

Moved the resident audio worker's large PCM ring, staging buffer, and output buffer from its stack to single-worker-owned static module storage. Rebuilt and installed the diagnostic package, reran three audio-enabled cycles in RPCS3, and refreshed the candidate report and working memory.

## Why this pass was needed

The previous two cycles passed but the third audio worker hit a fatal null-address execution fault before teardown could be recorded. Source measurement found that the worker's local buffers exceeded its configured stack size.

## Files changed

- `experimental/resident/module.c`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1905_audio_worker_stack_retest.md`

## Architecture / logic changes

The PCM ring (65,864 bytes), refill staging buffer (4,096 bytes), and output block buffer (2,048 bytes) now occupy static module storage. Only the audio worker accesses these buffers, and the existing lifecycle requires joining it before the module can be unloaded. This avoids placing at least 72,008 bytes of buffers on a worker configured with a 65,536-byte stack.

## Runtime behavior changes

No intended audio algorithm or receiver protocol changed. Runtime state is held in module storage instead of the audio worker stack. The rebuilt harness package then completed three audio-enabled resident cycles, with separate foreground and resident audio-port submissions.

## Validation performed

- Ran `bash scripts/test-resident-host-lifecycle.sh` in the existing Lima guest: passed host lifecycle/audio mocks, startup and cleanup fault cases, and SPSC concurrency checks under ASan/UBSan.
- Ran `bash scripts/build-lima-resident-rpcs3-harness.sh` in Lima: passed PPU build, PRX structure, package creation/extraction, and embedded fake-SPRX identity comparison.
- Installed and ran the generated `RPRX90001` package in RPCS3 after the user unlocked the Mac. The isolated harness log records three complete cycles; each returned stop/unload zero and module count six after unload, from an initial count of six. Final event: `HARNESS_PASS value=3`.
- Recomputed artifact SHA-256 values and verified the installed fake SPRX matches the local built fake SPRX.
- The harness submitted 940 foreground tone blocks while the resident reported synthetic diagnostic PCM submission on its own port. This verifies API submission, not acoustic output or game mixing.
- No physical PS3 was contacted.

## Immediate results

- Three audio-enabled RPCS3 load/start/stop/join/unload cycles passed, with the module list returning to baseline every cycle.
- Resident and foreground audio submissions were both accepted by RPCS3 during the same test run.
- RPCS3's accumulated GUI log retains an earlier fatal audio-thread fault. The isolated latest harness log is a fresh complete pass after the storage fix.
- RPCS3 networking reported `EADDRNOTAVAIL`, and the resident's file logger remains unavailable in the emulator.

## Known limitations / follow-ups

- Oversized worker stack use is the leading explanation for the earlier failure; the old fault address has not been mapped to an exact instruction.
- No synthetic RAOP packet stream ran through the resident PRX. Host receive-to-render testing does not prove that integration.
- No audible output was independently recorded; two ports accepting submissions does not prove acoustic mixing or game coexistence.
- The module-count check does not measure all sockets, allocations, or resource leaks. Real PS3 and Cobra compatibility are untested.

## Risks introduced

The static buffers increase module static-data use by roughly 72 KiB. Their safe reuse depends on the existing one-audio-worker-at-a-time lifecycle and stop/join ordering. No concurrent second worker may share them.

## Decision record (embedded)

### Decision

Keep the large, single-worker-owned audio buffers in static module storage and retain the existing audio algorithm.

### Context

The previous audio worker's buffer locals totaled 72,008 bytes before call frames, exceeding its configured 65,536-byte stack. The first rebuilt emulator rerun after this change completed all three cycles.

### Options considered

- Increase every resident audio-worker stack to exceed the full local buffer footprint plus margin.
- Move the large buffers to static storage under the current single-worker lifecycle.
- Dynamically allocate the buffers on worker startup and free them on shutdown.

### Why this option was chosen

Static module storage avoids a stack overrun without changing the PS3 thread allocation contract or adding allocation failure and cleanup paths. The worker already has exclusive ownership and is joined before unload.

### Tradeoffs accepted

The module reserves approximately 72 KiB of static data, and correctness relies on serialized worker lifecycle. The emulator pass supports the change but does not prove the exact cause of the previous fault or establish physical-console behavior.

## Next recommended step

Add a synthetic RAOP test path through the resident PRX in an environment with working network interfaces, verify decoded PCM reaches `cellAudio` submission, and gather an independent observation of audible output. Keep physical hardware testing gated until the emulator networking and output gaps are resolved.
