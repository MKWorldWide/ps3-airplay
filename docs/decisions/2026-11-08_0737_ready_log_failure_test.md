# Test receiver cleanup after readiness log failure

## Timestamp in EDT

2026-11-08 07:37 EDT

## Scope of the pass

Added a host-mock fault injection for failure to write the `RECEIVER_READY` event. The case verifies receiver close, early worker completion, module stop, and network cleanup.

## Why this pass was needed

The preceding test covered `raop_receiver_open` failure. A separate early exit occurs after the receiver has opened if the readiness log write fails; that path must close receiver-owned descriptors before the module releases network state.

## Files changed

- `tests/resident/host_lifecycle_test.c`
- `experimental/resident/README.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0737_ready_log_failure_test.md`

## Architecture / logic changes

- The host mock can fail one `RECEIVER_READY` log write and counts receiver-close calls.
- The test waits for the worker-stop diagnostic, then calls `module_stop` and asserts exactly one receiver close and successful NetCtl/network teardown.
- No production source changed; candidate PRX/PKG hashes remain unchanged.

## Runtime behavior changes

None in the product. The host-only lifecycle matrix now covers readiness-log failure.

## Validation performed

- Eleven ASan/UBSan lifecycle cases passed in four consecutive full runs.
- The previous full Lima host/build/package run remains applicable because production source is unchanged; PRX structure and embedded payload comparison passed in that run.
- RPCS3 runtime remains blocked by the locked desktop; no physical PS3 was contacted.

## Immediate results

- Receiver cleanup after readiness-event write failure is repeatably verified in the host mock.
- Hardware readiness remains NOT READY.

## Known limitations / follow-ups

- Mock log and receiver behavior do not prove PS3 filesystem or receiver socket behavior.
- Run the isolated RPCS3 three-cycle harness when the Mac desktop is accessible.

## Risks introduced

None beyond the existing limitation that host mocks cannot reproduce PS3 behavior.

## Decision record (embedded)

### Decision: cover the readiness-log exit separately

- **Context:** Receiver-open failure and readiness-log failure share a worker exit class, but only the latter must close a successfully opened receiver.
- **Options considered:** Infer cleanup from existing normal-stop tests; combine both faults into one ambiguous mock; or isolate the log failure and assert the close count.
- **Why chosen:** A single-purpose injected failure makes descriptor cleanup evidence direct and repeatable.
- **Tradeoffs accepted:** The mock validates call ordering only, not socket closure semantics on the PS3.

## Next recommended step

Run the `RPRX90001` harness under RPCS3 after desktop access is restored; require all three lifecycle cycles and module-count checks to pass before considering physical testing.
