# Resident audio backend and RPCS3 teardown checkpoint

## Timestamp in EDT

2026-11-08 17:56 EDT

## Scope of the pass

Added a bounded PCM path and a `cellAudio` playback worker to the experimental resident PRX. Expanded host mocks and tests, and expanded the `RPRX90001` RPCS3 harness to exercise resident PCM submission alongside a foreground audio port. Captured the current third-cycle RPCS3 crash and updated the overnight report.

## Why this pass was needed

The resident previously decoded audio without sending PCM to an output backend. The next evidence gate was to prove queue-to-output submissions, test two simultaneous audio sources, and check repeated teardown.

## Files changed

- `experimental/resident/CMakeLists.txt`
- `experimental/resident/harness/source/main.c`
- `experimental/resident/module.c`
- `experimental/resident/resident_diagnostics.h`
- `scripts/test-resident-host-lifecycle.sh`
- `src/audio/pcm_ring.c`
- `src/audio/pcm_ring.h`
- `src/audio/pcm_spsc.c`
- `src/audio/pcm_spsc.h`
- `tests/audio/pcm_spsc_test.c`
- `tests/resident/host_lifecycle_test.c`
- `tests/resident/mock_include/cell/audio.h`
- `tests/resident/mock_include/cell/sysmodule.h`
- `tests/resident/mock_include/sys/timer.h`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1756_resident_audio_backend.md`

## Architecture / logic changes

- Added a fixed-capacity, 16,384-frame, single-producer/single-consumer decoded-PCM queue. Each frame carries a generation so the playback worker can discard stale session audio after reconnect or flush.
- The resident audio worker moves queued signed 16-bit PCM through the existing PCM ring/resampler, waits for a 2,048-frame prebuffer, then submits 256-frame blocks to its own stereo `cellAudio` port at 48 kHz.
- System audio initialization ownership is tracked. If the host application already initialized the mixer, the resident borrows it and does not call `cellAudioQuit` during stop.
- A diagnostics-only 440 Hz seed exercises the resident queue and output path when the harness diagnostics argument is supplied. Normal PRX operation does not inject this tone.
- The RPCS3 harness now opens a second `cellAudio` port and submits a controlled foreground tone while the resident submits its diagnostic PCM.

## Runtime behavior changes

When active, the experimental PRX creates a dedicated audio worker and audio port. RAOP decoded PCM is queued for playback instead of being discarded. The backend is bounded and generation-aware. The RPCS3 two-port harness exercises submissions, but it is not a real AirPlay stream or a game audio test.

## Validation performed

- Ran in Lima with the pinned Mbed TLS 3.6.7 include directory: `bash scripts/test-resident-host-lifecycle.sh` — all mocked lifecycle cases, backend ownership/output cases, and bounded SPSC queue/concurrency tests passed.
- Built the PPU PRX and `RPRX90001` package in the existing PS3DK v0.27.0 Lima environment. The generated package installed in RPCS3; its embedded fake SPRX hash matched the expected build hash.
- Ran three cycles in RPCS3 with concurrent foreground/resident audio submissions. Cycles 1 and 2 reached successful stop/unload. Cycle 3 failed before the stop result was recorded.
- Inspected the RPCS3 fatal log and disassembled the built PRX. RPCS3 reported a fatal null-address execution in `ps3-airplay-audio` at offset `0x001201c8`; that offset maps to the instruction immediately after the `sc` in the `sysThreadExit` import stub. This correlation does not establish root cause.
- Recomputed candidate hashes: PRX `9a558a5b994f68e109e0bed8c4564e7d54b4c5463e1190d449df9571d7c1e5b2`; fake SPRX `7bfea5421d546fec46a9020ecedd4fafae61cf03594b85480087d1dd20aa6120`; harness PKG `86db3830dfbd08c73337ea5a4a8ec19d416cee919783f17360b788602bd49934`.
- No physical console was contacted. No synthetic RAOP stream was sent. Acoustic output was not independently confirmed.

## Immediate results

- PASS: host-mocked audio startup, PCM submission, cleanup, and pre-initialized mixer ownership.
- PASS: bounded queue overflow, mono/stereo handling, generation filtering, and concurrent sequence validation.
- PASS: RPCS3 accepted resident and foreground PCM submissions through separate audio ports during the observed test window.
- PASS: RPCS3 cycles 1 and 2 stop/unload completed.
- FAIL: RPCS3 cycle 3 terminated the resident audio thread with a null-address fault; stop/unload and module baseline were not recorded.
- BLOCKED: resident file logging is unavailable in this RPCS3 setup; diagnostics were carried through the harness structure.
- UNTESTED: real RAOP-to-PCM playback, audible output, physical PS3 compatibility, and game coexistence.

## Known limitations / follow-ups

- Diagnose whether the third-cycle fault is a PRX thread-exit defect, RPCS3 behavior, or a resource interaction. Do not claim a cause from the PC correlation alone.
- Repeat three audio-enabled cycles and verify module count returns to six after every unload.
- Add synthetic RAOP traffic through resident session/decrypt/ALAC decode before claiming AirPlay PCM reaches the backend.
- Investigate RPCS3's `EADDRNOTAVAIL` local network discovery behavior separately; it does not explain the audio-thread crash.
- Acoustic output, actual hardware audio mixing, and Cobra compatibility remain untested.

## Risks introduced

- The new PPU worker and concurrent audio-port lifetime have not passed repeated RPCS3 teardown. The third-cycle crash makes this candidate unsafe for hardware testing.
- The fixed queue consumes roughly 128 KiB plus generation tags and metadata in resident memory. Total resident memory pressure has not been measured on PS3.
- The synthetic diagnostic seed proves only the local queue-to-API submission path and may obscure integration failures if mistaken for AirPlay data.

## Decision record (embedded)

### Decision 1: Reuse the existing PCM ring and resampler

- **Context:** The host audio code already provides sample-rate conversion and bounded ring behavior.
- **Options considered:** Write a second resampler in the PRX, submit decoded frames directly, or bridge the host-tested PCM ring to `cellAudio`.
- **Why chosen:** Reuse reduces duplicated signal-processing code and keeps the playback worker focused on buffering and submission.
- **Tradeoffs accepted:** The current tests do not validate long-duration drift, scheduling quality, or real console resource interaction.

### Decision 2: Borrow host audio initialization when already active

- **Context:** The harness and eventual foreground game may initialize system audio before the resident starts.
- **Options considered:** Treat `CELL_AUDIO_ERROR_ALREADY_INIT` as fatal, call quit unconditionally, or track ownership and preserve host state.
- **Why chosen:** Borrowing avoids tearing down audio state owned by another application component.
- **Tradeoffs accepted:** Emulator semantics may not match all CFW/hardware cases; hardware behavior remains untested.

### Decision 3: Keep audio diagnostics isolated to test startup arguments

- **Context:** Synthetic PCM is useful to determine whether queueing and `cellAudio` submission work before a sender exists.
- **Options considered:** Always generate startup audio, add a production config flag, or enable the seed only when harness diagnostics are provided.
- **Why chosen:** The test stays repeatable without introducing audible diagnostic behavior into ordinary resident operation.
- **Tradeoffs accepted:** This validates plumbing only and must not be reported as successful AirPlay playback.

### Decision 4: Record the cycle-three failure and keep hardware readiness closed

- **Context:** RPCS3 accepted two audio-enabled teardown cycles, then faulted in the audio thread before reporting cycle-three stop/unload.
- **Options considered:** Treat the crash as an emulator transient, ignore the incomplete final cycle, or mark the run failed and investigate separately.
- **Why chosen:** There is no evidence supporting a transient classification, and lifecycle safety is a required gate.
- **Tradeoffs accepted:** The candidate cannot be proposed for physical testing until the failure is isolated and repeated teardown passes.

## Next recommended step

Add narrow phase markers around audio-worker stop, `sysThreadExit`, and join; then run a minimal RPCS3 case that compares audio-enabled teardown with foreground-port closure while preserving the same resident build. Require three complete cycles and module count six each time before advancing to synthetic RAOP traffic.
