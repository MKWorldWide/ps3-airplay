# Test resident worker cleanup after poll errors

## Timestamp in EDT

2026-11-08 07:24 EDT

## Scope of the pass

Extended the host lifecycle simulator to inject a receiver-poll error, verify that the worker exits through its cleanup path, and call `module_stop` afterward to join the finished worker and release network modules.

## Why this pass was needed

The simulator covered requested shutdown and cleanup retry failures, but not a worker that exits on its own after an unexpected receiver error. This is a distinct lifecycle path that should still permit orderly module stop.

## Files changed

- `tests/resident/host_lifecycle_test.c`
- `experimental/resident/README.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0724_worker_poll_error.md`

## Architecture / logic changes

- No production PRX logic changed.
- The mock receiver returns a single injected error after the worker enters poll. The production worker logs the error, closes its receiver/mDNS/decoder state, marks itself exited, and calls the mocked thread exit. The test then invokes the real `module_stop` function and checks network cleanup.
- The test retains mocked socket, receiver, mDNS, logging, and PS3 sysmodule/thread APIs.

## Runtime behavior changes

None in the production module. The host-only lifecycle test now includes an unexpected worker poll-error path.

## Validation performed

- Ran `bash scripts/test-resident-host-lifecycle.sh` four consecutive times in Lima with AddressSanitizer and UndefinedBehaviorSanitizer.
- In every run, all seven cases passed: worker-create rollback, network-init rollback, NetCtl unload retry, network-finalize retry, network-module unload retry, bounded stop timeout/retry, and poll-error worker cleanup/join.
- No RPCS3 package was installed or run, and no physical PS3 was contacted.

## Immediate results

- An independently failing worker is now covered by repeated host-mock lifecycle checks.
- Existing RPCS3 and hardware readiness statuses remain BLOCKED and NOT READY, respectively.

## Known limitations / follow-ups

- This test does not execute real receiver or mDNS code and does not validate PS3/RPCS3 thread or syscall behavior.
- The actual three-cycle PRX load/start/stop/unload test remains outstanding in RPCS3.

## Risks introduced

- Host mocks may diverge from the target runtime; results must remain labeled host-only.

## Decision record (embedded)

### Decision: add spontaneous poll-error cleanup coverage

- **Context:** Worker termination can begin because of receiver failure rather than a stop request.
- **Options considered:** Rely on normal stop coverage; add a mock error injection and exercise subsequent stop; attempt this only in RPCS3 after desktop access.
- **Why chosen:** The test can verify the module's own error-to-exit cleanup path now while preserving the separate emulator gate.
- **Tradeoffs accepted:** The evidence remains limited to the actual C control flow with mocked receiver and platform operations.

## Next recommended step

When RPCS3 is accessible, run the three-cycle test and inspect actual worker, stop, unload, and module-count events before considering hardware.
