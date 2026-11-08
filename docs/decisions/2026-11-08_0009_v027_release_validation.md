# Validate and retain v0.2.7 candidate

Timestamp (EDT): 2026-11-08T00:09:33.696784-04:00

## Scope of the pass

Complete the exact-source release gate, retain the candidate, add reproducible performance/spectrum comparisons to future tests and record results.

## Why this pass was needed

A source proposal needs a validated, identifiable artifact and explicit hardware boundary.

## Files changed

- `scripts/run-forensic-tests.sh`
- `tests/audio/resampler_fidelity.py`
- `tests/audio/pcm_cost_test.c`
- `tests/observability/display_copy_cost.py`
- `docs/reports/ps3_airplay_sol61_fidelity_optimization.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/lessons-learned.md`
- `docs/decisions/2026-11-08_0009_v027_release_validation.md`

## Architecture / logic changes

No application changes after packaged source `39e918c`. Future gates add SRC block-cost and actual framebuffer-copy benchmarks; offline reference test preserves spectrum and alternative-kernel comparisons.

## Runtime behavior changes

None beyond the previously committed candidate. Console remains untouched.

## Validation performed

- `APP_VERSION=0.2.7 SFO_VERSION=00.47 bash scripts/release-lima-standalone-receiver.sh` in the pinned Linux build environment: full gate PASS.
- Independent NumPy/SciPy reference, analytical tone thresholds, music and spectrum probes passed.
- `cc -O2 -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -Isrc/audio tests/audio/pcm_cost_test.c src/audio/pcm_ring.c` and execution passed.
- `python3 tests/observability/display_copy_cost.py` passed; supplemental benchmarks also executed separately in Linux.
- Local PKG SHA-256 matches guest; expected SFO 00.47, debug package structure/extraction/NPDRM and assets validated.

Privacy review: complete staged diff/exact files inspected, whitespace and targeted identifier/credential scans clear. No secret scanner was installed; raw material and local instructions remain ignored.

## Immediate results

PKG 3,273,456 bytes, SHA-256 `48251b3d741328fa78a5cda9bccd76ab02a79ec664d49ce29edfd819eb9f9993`. Source`39e918ce15d70e4dec1a25ec2369a166651057f0`. Exact 600-second frame count, bounded resends, callbacks/flip timeout, all host protocol/codec/log checks and PowerPC build pass. Music reference SNR improves 33.79→63.75dB. Physical behavior remains untested.

## Known limitations / follow-ups

Finite 20–22k transition band, unknown independent drift and actual GameOS deadline/log latency;350 ms guard requires physical compositor acceptance. No deployment authorized by this mission.

## Risks introduced

No additional app risk in this pass. Benchmark results are platform-dependent and do not enforce a misleading universal CPU deadline. Raw audio/logs/artifacts stay ignored.

## Decision record

- Decision:retain candidate without auto-install; Context:all software gates passed but hardware evidence is pending; Options:auto-deploy or controlled authorized test; Reason:mission explicitly forbids automatic deployment; Tradeoff:physical acceptance deferred.
- Decision:report both band-limited and fullband results; Context:finite FIR transition differs from long reference; Options:publish tone-only success or complete limitations; Reason:accurate fidelity assessment; Tradeoff:more explicit acceptance boundary.

## Next recommended step

Review candidate and perform an explicitly authorized short hardware comparison, PS/menu/resume/START twice and timing-counter review before any longer soak.
