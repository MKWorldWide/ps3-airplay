# Detect module accumulation in resident lifecycle harness

## Timestamp in EDT

2026-11-08 06:36 EDT

## Scope of the pass

Extended the RPCS3 test harness to record a pre-test PRX module count and require that count to return after each of three successful stop/unload cycles. Updated resident documentation to describe this check.

## Why this pass was needed

The harness already waited for worker cleanup events and checked successful stop/unload return codes, but it did not independently verify that repeated cycles returned the emulator's PRX module list to its baseline size.

## Files changed

- `experimental/resident/harness/source/main.c`
- `experimental/resident/README.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0636_harness_module_count.md`

## Architecture / logic changes

- The harness calls `sys_prx_get_module_list` before cycling and records the count.
- After each successful `sys_prx_unload_module` and `MODULE_STOP` event, it queries the module list again and fails if the count differs from baseline.
- The query uses a 256-entry array and fails if the returned count reaches capacity, avoiding a truncated list being reported as a successful leak check.
- The installed RPCS3 build's matching source registers `sys_prx_get_module_list`; the PS3DK header exposes the same API. Neither API's behavior in this homebrew title was executed in the current pass.

## Runtime behavior changes

When run, the harness logs `MODULE_COUNT_BASELINE` and `MODULE_COUNT_AFTER_CYCLE`. It reports `MODULE_COUNT_LEAK` and stops if the count does not return to baseline. A module-list query failure also fails the harness. This affects only the experimental test app.

## Validation performed

- Full `scripts/build-lima-resident-rpcs3-harness.sh` passed after this change: all host tests, PPU build, PRX structural check, harness SELF/PKG build, and extracted PRX byte comparison.
- Harness build now includes the `sys_prx_get_module_list` call. The package contains the updated harness and the current resident PRX.
- The three emulator cycles and module-count comparisons remain untested because the Mac desktop is locked.
- No physical console or emulator settings were changed.

## Immediate results

- The test app has a direct way to detect a PRX module entry left behind after unload.
- This is an additional safeguard, not proof that sockets, worker threads, heap allocations, or every OS resource have returned to baseline.

## Known limitations / follow-ups

- RPCS3 must actually run the package to validate the module-list API pointer layout, counts, and three-cycle behavior.
- The 256-module capacity is a conservative test bound; if reached, the harness fails as inconclusive instead of passing.
- The harness cannot directly enumerate network sockets or measure per-cycle heap/RSS on the PS3.

## Risks introduced

- A runtime that counts application modules differently or reports a full 256-entry list will make the harness fail closed. That result requires investigation, not bypassing the check.
- A stable module count does not prove all non-PRX resources were cleaned up; worker cleanup events remain a separate required signal.

## Decision record (embedded)

### Decision: compare the module-list count after every unload

- **Context:** Three successful syscall return values alone do not demonstrate that module objects stopped accumulating.
- **Options considered:** Rely on stop/unload results and log events; query module identity after every cycle; compare total module-list count before and after each cycle.
- **Why chosen:** A baseline count is a simple way to detect residual PRX entries using an API available in the target SDK and registered by the installed emulator build.
- **Tradeoffs accepted:** The check is runtime-dependent, has a fixed safe capacity, and cannot detect sockets or allocations not represented in the PRX module list.

## Next recommended step

Run the package in RPCS3 and verify all three cycles return to the baseline count, while retaining worker-stop, module-stop, unload, and receiver-ready evidence. Then update the overnight report; keep hardware readiness NOT READY unless runtime cleanup passes.
