# Linux host clock declarations

**Timestamp (EDT):** 2026-11-08 02:37

## Scope of the pass

Added a host-only POSIX feature-test macro to the diagnostic logger and documented the exact-source Lima gate failure and rerun boundary.

## Why this pass was needed

The pinned Ubuntu release build uses strict C flags and failed because `clock_gettime` and `CLOCK_MONOTONIC` were not declared. The same host code had compiled on macOS, demonstrating that local host success was insufficient evidence for this Linux build path.

## Files changed

- `src/observability/receiver_log.c`
- `docs/TEST_MATRIX.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/air_ps3_v028_reliability_branding.md`
- `docs/decisions/2026-11-08_0237_linux_host_clock_declarations.md`

## Architecture / logic changes

For non-PSL1GHT builds, `_POSIX_C_SOURCE=200809L` is defined before system headers so the standard host declarations are available. The PS3 logger continues to use `sysGetSystemTime()` and does not take the POSIX clock path.

## Runtime behavior changes

No PS3 runtime behavior changes. The host logger can compile in strict Linux C mode once the corrected exact-source release gate passes.

## Validation performed

- Ran the pinned Lima release gate on the earlier candidate commit. It passed protocol, codec, audio, RTP timing and display mock stages before stopping at the strict-C host logger compile.
- Inspected the failing diagnostics and confirmed the error was declaration visibility for `clock_gettime` / `CLOCK_MONOTONIC`.
- Corrected full Lima gate: pending at time of this record.

## Immediate results

- Failure is isolated to the host compilation declaration environment; no PPU-specific clock implementation failure was observed.
- The corrected source must pass a full exact-source rerun before producing a release-valid package.

## Known limitations / follow-ups

- Lima/x86 emulation is slow for the 131,072-packet RTP forensic workload.
- Physical display, XMB resume, AirPlay session, audible playback and clean-exit behavior remain untested.

## Risks introduced

- The feature macro affects declarations only for non-PSL1GHT compilation. Verify the complete PPU build after this source change to guard against unintended conditional-build interactions.

## Decision record

### Decision

Define `_POSIX_C_SOURCE=200809L` locally at the top of the logger translation unit for host builds.

### Context

The Linux release gate uses strict C compilation and needs POSIX monotonic time; changing project-wide compiler flags would broaden scope.

### Options considered

- Add the feature-test macro to the translation unit.
- Weaken strict compiler flags or add a global build flag.
- Replace the host monotonic clock implementation.

### Why this option was chosen

It is the smallest portable correction, is scoped to the host branch, and preserves the PS3 clock implementation and strict build checks.

### Tradeoffs accepted

The source file now explicitly opts into the POSIX.1-2008 API set on host builds; this is appropriate for the Linux/macOS host test targets but should remain guarded from PSL1GHT.

## Next recommended step

Run the full release gate in the pinned Lima guest from this exact committed source, then record the package metadata and remaining validation boundaries.
