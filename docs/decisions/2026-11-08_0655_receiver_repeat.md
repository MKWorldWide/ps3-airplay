# Repeat RAOP receiver loopback timing test

## Timestamp in EDT

2026-11-08 06:55 EDT

## Scope of the pass

Ran the existing RAOP receiver integration test ten times consecutively in Lima and recorded the results. No production source changed.

## Why this pass was needed

One earlier full-suite invocation failed its 500 ms audio-observation assertion. The next isolated/full runs and three repeats passed; ten more targeted runs help determine whether the failure is readily reproducible.

## Files changed

- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0655_receiver_repeat.md`

## Architecture / logic changes

- None. This was verification and documentation only.
- The report retains the historical one-time failure while adding ten new passing repetitions.

## Runtime behavior changes

None.

## Validation performed

- Ran `build/lima-crypto/raop_receiver_test` ten consecutive times from the Linux/Lima workspace.
- Each run reported `RAOP persistent RTSP + UDP RTP loopback and bounded slow-reader disconnect: PASS`.
- All ten processes exited successfully; no timeout assertion recurred.
- No RPCS3 or physical PS3 activity occurred.

## Immediate results

- The receiver integration test passed 10/10 in this repeat batch.
- The earlier 500 ms timeout remains unexplained and is not marked fixed.

## Known limitations / follow-ups

- Ten runs do not reproduce long-duration or highly contended hardware timing.
- Repeat under RPCS3/PS3 networking when desktop access and later hardware approval gates allow.

## Risks introduced

- None. No code, package, or device state changed.

## Decision record (embedded)

### Decision: preserve the isolated failure in the report

- **Context:** The targeted test passed 10/10, but a prior full-suite run did fail its observation deadline once.
- **Options considered:** Remove the failure from the report after repeated passes; mark the issue fixed; or keep it as an unresolved historical observation.
- **Why chosen:** The root cause is unknown, so a passing repeat batch cannot establish a fix.
- **Tradeoffs accepted:** The report continues to show one non-reproduced test failure alongside the new pass evidence.

## Next recommended step

If this timing assertion recurs, capture scheduler/load conditions and packet-arrival timestamps before changing the timeout or receiver logic.
