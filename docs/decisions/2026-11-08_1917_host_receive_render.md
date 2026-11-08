# Host receive-to-render integration

**Timestamp in EDT:** 2026-11-08 19:17 EDT

## Scope of the pass

Extend the host RAOP socket integration test across decoded PCM queue handoff,
production PCM ring resampling, and output-frame rendering. Update the build
script and current resident development report/context.

## Why this pass was needed

The existing receive test stopped after pushing decoded PCM into the ring. A
successful decrypt/decode did not prove that the bounded queue handoff and
resampling path could produce a usable output buffer.

## Files changed

- `scripts/build-lima-crypto.sh`
- `tests/protocol/raop_audio_pipeline_test.c`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_1917_host_receive_render.md`

## Architecture / logic changes

The host integration test now feeds the exact decoded ALAC PCM through the
bounded SPSC queue, transfers all decoded frames into the production PCM ring,
and renders 256 stereo float frames at 48 kHz from the 44.1 kHz source. It
asserts exact decoded samples, full queue transfer, no output underflow, and a
non-silent rendered buffer. The Lima test build links `pcm_spsc.c` into this
test executable.

## Runtime behavior changes

No PS3 or emulator runtime behavior changes. The host test now exercises more
of the receive-to-render pipeline.

## Validation performed

- Ran the targeted Lima `raop_audio_pipeline_test` with AddressSanitizer and
  UndefinedBehaviorSanitizer configured to halt on error.
- Observed PASS for encrypted RTP decrypt and ALAC decode, exact PCM,
  SPSC transfer, and 44.1-to-48 kHz output rendering with no underflow.
- Reran the complete `scripts/build-lima-crypto.sh` gate after copying the
  edited files to Lima and confirming their SHA-256 hashes matched local
  sources. All host tests and PPU component-object checks passed.
- No RPCS3 or physical-console run occurred in this pass. RPCS3 remains
  inaccessible while the desktop is locked.

## Immediate results

- PASS: all 353 decoded frames transferred and 256 rendered frames were
  produced without underflow; output exceeded the non-silence threshold.
- The result is host buffer-generation evidence only; it does not prove
  resident PRX playback, RPCS3 output, audible sound, or game mixing.

## Known limitations / follow-ups

- The previous RPCS3 audio run passed cycles 1 and 2 and faulted during cycle
  3 in the resident audio worker. The exact fault and post-cycle module count
  are unresolved.
- Run the latest instrumented/atomic candidate in RPCS3 after access returns,
  require three clean audio-enabled cycles and module count six after every
  unload, then exercise an end-to-end synthetic RAOP session in the resident.
- Hardware readiness remains NOT READY.

## Risks introduced

The new assertion relies on the deterministic ALAC fixture producing
non-silent rendered samples. It is confined to host tests and does not alter
the shipping receiver path.

## Decision record (embedded)

### Decision

Extend the existing socket test to cover queue handoff and render-buffer
production, without changing receiver runtime code.

### Context

RPCS3 could not be rerun while the desktop was locked, while the host test had
not yet covered the final PCM buffering/resampling stage.

### Options considered

- Wait for RPCS3 access and leave the host integration gap open.
- Add the integration assertion to the existing ASan/UBSan receive test.
- Add a separate test executable duplicating the receiver fixture.

### Why this option was chosen

The existing encrypted RTP fixture already verifies the upstream network,
decrypt, and ALAC path. Extending it reaches further through production audio
buffering with less duplicated setup.

### Tradeoffs accepted

The test verifies host-side PCM generation, not PRX scheduling or a real audio
device. The later RPCS3 gate remains necessary.

## Next recommended step

After the Mac is unlocked, install and run the latest `RPRX90001` candidate in
RPCS3, inspect exact loader and thread traces, and complete three audio-enabled
load/start/stop/unload cycles with module count returning to baseline each time.
