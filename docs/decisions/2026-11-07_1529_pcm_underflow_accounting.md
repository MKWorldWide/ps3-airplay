# Correct active-stream PCM underflow accounting

## Timestamp in EDT

2026-11-07 15:29 EDT

## Scope of the pass

Added a PCM silence helper that emits intentional silence without increasing the ring's underflow counter. Standalone audio block generation uses it until a decoder exists and the RTP prebuffer is ready. Per-session underflow/drop log baselines reset when recording starts or ends. Assigned AIRPLAY02 v0.1.35 / SFO 00.35, retaining v0.1.34 RTP recovery.

## Why this pass was needed

The v0.1.33 hardware log reported 51,200 underflow frames, but source inspection showed the callback counted empty-ring frames during expected startup prebuffer silence. That made a diagnostic field look like active-playback failure without proving a dropout.

## Files changed

- `src/audio/pcm_ring.c`
- `src/audio/pcm_ring.h`
- `tests/audio/pcm_ring_test.c`
- `prototype/standalone-receiver/source/main.c`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/BUILD_PIPELINE.md`
- `docs/VERSIONING.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/ROADMAP.md`
- `docs/RISKS.md`
- `docs/TEST_MATRIX.md`
- `docs/reports/2026-11-07_v0133_audio_followup.md`
- `docs/decisions/2026-11-07_1516_live_audio_diagnosis.md`
- `docs/decisions/2026-11-07_1529_pcm_underflow_accounting.md`

## Architecture / logic changes

The PCM ring now exposes a zero-fill helper for intentional silence. The app still outputs silence before audio is ready, but only calls the PCM ring renderer once the ALAC decoder exists and the 128-packet RTP prebuffer is complete. This keeps normal startup silence from inflating `PCM_UNDERFLOW`. Underflow and drop report baselines reset with each recording lifecycle so counter deltas cannot underflow across sessions.

## Runtime behavior changes

Audible behavior is unchanged before `AUDIO_PREBUFFER_READY`; the output blocks remain silent. After prebuffer readiness, rendering and real underrun counting continue as before. v0.1.35 retains v0.1.34's pending/outstanding RTP resend recovery.

## Validation performed

- Built and ran the focused host PCM-ring regression with `cc -std=c11 -Wall -Wextra -Werror src/audio/pcm_ring.c tests/audio/pcm_ring_test.c -lm`; it verifies intentional silence leaves the underflow counter unchanged and a true active empty render increments it.
- Lima ASan/UBSan suite and PPU build/package validation remain pending.
- No console files were changed; v0.1.33 remains running, and the prebuilt v0.1.34 PKG remains staged but uninstalled.
- Pre-commit privacy scan inspected the exact staged diff and paths for secrets, local identifiers/paths, private network data, and raw logs; no matches. No gitleaks/trufflehog scanner is installed. `AGENTS.md` and generated `work/` artifacts are ignored and untracked.

## Immediate results

- Focused PCM host test passes.
- Hardware sample analysis and the existing log do not establish Spotify Lossless as the source of crackle. The 88 RTP sequence gaps remain measurable; the previous underflow delta is reclassified as contaminated by expected prebuffer silence.
- The latest PS3 log still indicated the v0.1.33 process was running, so installation is deferred until it exits cleanly.

## Known limitations / follow-ups

The fix corrects telemetry semantics; it does not by itself improve audible playback. v0.1.35 must be built and tested on hardware. Compare RTP gaps, resend recovery, and valid underflows while XMB is idle and under navigation load. Test Spotify source quality only after this controlled run.

## Risks introduced

An incorrectly gated active state could hide genuine underflows. The host helper test covers only zero-fill semantics; PPU code must compile, and the hardware log must show `PCM_UNDERFLOW` only after prebuffer readiness.

## Decision record

### Decision: Keep intentional silence out of underflow metrics

- **Context:** The 51,200-frame underflow delta could represent the initial prebuffer interval rather than a playback dropout.
- **Options considered:** Leave the metric unchanged; subtract startup frames after the fact; do not count ring underflow until the app marks PCM playback active.
- **Why chosen:** The app has an explicit decoder/prebuffer readiness state, so gating measures the defined event directly and avoids unreliable heuristics.
- **Tradeoffs accepted:** This does not fix packet loss or prove the cause of crackling; a separate active-stream test is needed.

### Decision: Assign v0.1.35 instead of modifying v0.1.34

- **Context:** v0.1.34 was already built and staged; a telemetry correction is a distinct implementation change.
- **Options considered:** Replace v0.1.34 artifact/version; combine the correction into the next patch increment.
- **Why chosen:** Preserves artifact provenance and the user's incremental version rule while keeping the resend recovery.
- **Tradeoffs accepted:** v0.1.34 itself remains untested on hardware; v0.1.35 becomes the next hardware candidate.

## Next recommended step

Run the full Lima host/sanitizer and PPU/package pipeline for v0.1.35. Upload the validated package to a new versioned console path, verify bytes, and install after AIRPLAY02 v0.1.33 has exited.
