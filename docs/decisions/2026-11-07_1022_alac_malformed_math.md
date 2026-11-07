# Harden ALAC arithmetic against malformed frames

## Timestamp

2026-11-07 10:22 America/New_York (host reports EST)

## Scope of the pass

Extended ALAC malformed-input protection after the first bounds fix. Added deterministic 256-packet sanitizer smoke coverage and made the integration-test invocation fail on the first ASan/UBSan report. Updated memory, lessons, roadmap, risk, and test-matrix records.

## Why this pass was needed

The newly added malformed-frame test first passed with sanitizer recovery enabled but printed undefined behavior for a shift exponent above 31 and signed overflow in predictor accumulation. A nonzero process status alone did not reveal those reports because UBSan recovered and continued.

## Files changed

- `src/codec/alac.c`
- `src/codec/raop_alac.c`
- `tests/protocol/raop_audio_pipeline_test.c`
- `scripts/build-lima-crypto.sh`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/RISKS.md`
- `docs/decisions/2026-11-07_1022_alac_malformed_math.md`

## Architecture / logic changes

ALAC now rejects unsupported prediction modes, invalid quantization/sample widths, and interlace shifts above 31 before those fields drive predictor or reconstruction code. Predictor accumulation and weighted stereo reconstruction use bounded 64-bit intermediates and explicit sign extension/saturation. RAOP codec negotiation rejects Rice k-modifier values above 30, which would otherwise be used as unsafe shift counts. The deterministic test generates 256 bounded malformed payloads from a fixed seed, while preserving the encrypted RTP/decrypt/ALAC/reference-PCM path.

## Runtime behavior changes

Unsupported or malformed ALAC frames now fail decoding cleanly rather than proceeding through unsupported predictor code or undefined arithmetic. The valid supported 16-bit reference fixture retains its golden output.

## Validation performed

- Full `scripts/build-lima-crypto.sh` in the existing Lima guest with `UBSAN_OPTIONS=halt_on_error=1` and `ASAN_OPTIONS=halt_on_error=1` on the composed pipeline test.
- All host tests passed: crypto/session, RTSP/RTP loopback including timing, mDNS, listener/log, ALAC golden and encrypted RTP-to-PCM integration, 353-vs-352 rejection, `UINT32_MAX` rejection, 256 deterministic malformed packets, PCM ring/gain, and tone.
- PPU component objects compiled for crypto, receiver/session, ALAC, and PCM.
- No standalone v0.1.20 application/PKG build yet; it must follow from the final commit.
- RPCS3 remains unavailable through Computer Use while the Mac is locked; no emulator or console runtime evidence was produced.

## Immediate results

- The first malformed corpus run identified the shift and signed-overflow cases.
- The final strict sanitizer run emitted no ASan/UBSan findings, and the valid encrypted fixture remained byte-identical to the expected PCM.
- Full host suite and PPU component compilation pass.

## Known limitations / follow-ups

- The corpus is deterministic smoke coverage, not exhaustive fuzzing.
- Build AIRPLAY02 v0.1.20 from the exact commit, validate ELF/package metadata and hash, then install/run in RPCS3 when the desktop is unlocked.
- Real iPhone compatibility, PS3 audio output, display/exit behavior, Spotify playback, and game coexistence remain unverified.

## Risks introduced

Malformed or unsupported predictor frames are now rejected, which can expose nonconforming senders as decode drops. The supported valid fixture remains unchanged; broader sender interoperability testing is still required.

## Decision record (embedded)

### Decision 1: Reject unsupported modes and bound arithmetic

- **Context:** Sanitizer smoke inputs reached predictor branches and triggered invalid shifts and signed overflow.
- **Options considered:** Increase all arithmetic widths without validating fields; clamp unsafe fields; reject unsupported mode/shift values and use wide bounded arithmetic for supported 16-bit reconstruction.
- **Why chosen:** Rejection preserves bounded behavior and avoids decoding a stream the implementation does not support; 64-bit intermediates protect supported math from malformed values.
- **Tradeoffs accepted:** Some malformed/nonconforming frames are dropped rather than best-effort decoded.

### Decision 2: Halt on sanitizer findings in the integration test

- **Context:** UBSan's default recover mode printed diagnostics but let the test exit successfully.
- **Options considered:** Parse sanitizer output after the run; terminate at the first error through sanitizer environment options.
- **Why chosen:** Halt-on-error turns undefined behavior into a deterministic failing test without custom output parsing.
- **Tradeoffs accepted:** The test stops at the first issue, so later malformed cases run only after earlier findings are fixed.

## Next recommended step

Commit and push this ALAC hardening pass after the brief privacy review; then build and validate AIRPLAY02 v0.1.20 from that exact commit. Retry RPCS3 when the desktop is accessible.
