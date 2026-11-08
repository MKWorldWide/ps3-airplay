# Exercise NetCtl module-load failure cleanup

## Timestamp in EDT

2026-11-08 07:27 EDT

## Scope of the pass

Added a host-mock case where loading NetCtl fails before initialization, then verified that the base receiver can become ready and that stop releases the network resources without trying to terminate or unload a NetCtl module that was never acquired.

## Why this pass was needed

The prior matrix covered network-init failure and optional NetCtl-init failure, but not a failed `CELL_SYSMODULE_NETCTL` load. The module should preserve its optional-control-path behavior without retaining false ownership state.

## Files changed

- `tests/resident/host_lifecycle_test.c`
- `experimental/resident/README.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0727_netctl_load_failure.md`

## Architecture / logic changes

- No production PRX logic changed.
- The mock fails one NetCtl module-load call. The existing `module_start` continues to the worker; the test observes `NETCTL_LOAD_FAILED` and `RECEIVER_READY`, then stops the module.
- Assertions verify NetCtl init/term/unload are not called after the failed load, while base network finalize/unload still occur.

## Runtime behavior changes

None in the production module. The host-only fault matrix now covers NetCtl module-load failure.

## Validation performed

- Ran the full host lifecycle simulation four consecutive times with AddressSanitizer and UndefinedBehaviorSanitizer; all nine cases passed each time.
- No RPCS3 package was installed or run, and no physical PS3 was contacted.

## Immediate results

- Optional NetCtl-load failure has repeatable host control-flow coverage.
- RPCS3 lifecycle remains unobserved and hardware readiness remains NOT READY.

## Known limitations / follow-ups

- The test uses mocked NetCtl, network, receiver, thread, mDNS, logging, and socket behavior. It cannot establish target API behavior or PRX loader compatibility.
- The actual RPCS3 three-cycle test remains outstanding.

## Risks introduced

- A passing mock case may differ from the PS3 implementation; it is reported only as host simulation evidence.

## Decision record (embedded)

### Decision: treat NetCtl load as optional and verify cleanup when it fails

- **Context:** The current production code permits receiver startup without NetCtl-provided MAC information.
- **Options considered:** Treat any NetCtl load failure as fatal; keep existing optional behavior and test it; or make no check until emulator access returns.
- **Why chosen:** The code already has a bounded fallback identity path; the host simulation can confirm that cleanup state is consistent without altering product behavior.
- **Tradeoffs accepted:** The test confirms only control flow, not whether the target network stack permits all advertised receiver behavior without NetCtl.

## Next recommended step

Run the dedicated package in RPCS3 after the desktop is unlocked and inspect the real NetCtl, worker, PRX unload, and module-count behavior before considering hardware.
