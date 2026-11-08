# Add mocked resident lifecycle simulation

## Timestamp in EDT

2026-11-08 07:08 EDT

## Scope of the pass

Added a Linux host test target that compiles the actual resident `module.c` with mocked PS3 platform APIs, receiver, logging, and socket calls. Exercised startup rollback, thread creation failure, worker-stop timeout, and retryable cleanup errors. Updated resident docs and report.

## Why this pass was needed

RPCS3 runtime execution remains blocked by the locked desktop. Host protocol coverage existed, but the resident PRX start/stop control flow had no locally executable test. The host mock adds evidence for those C paths without presenting itself as PRX/runtime validation.

## Files changed

- `tests/resident/host_lifecycle_test.c`
- `tests/resident/mock_crypto_rng.h`
- `tests/resident/mock_include/cell/libnet.h`
- `tests/resident/mock_include/cell/libnetctl.h`
- `tests/resident/mock_include/cell/sysmodule.h`
- `tests/resident/mock_include/sys/prx.h`
- `tests/resident/mock_include/sys/prx_module.h`
- `tests/resident/mock_include/sys/sys_time.h`
- `tests/resident/mock_include/sys/thread.h`
- `scripts/test-resident-host-lifecycle.sh`
- `experimental/resident/README.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0708_host_lifecycle_simulation.md`

## Architecture / logic changes

- No production PRX logic changed.
- The host test links the real resident start/stop translation unit against POSIX-thread-backed `sysThread*` shims and injected return values for system-module load/unload and network init/finalize.
- Receiver, mDNS, logging, crypto initialization, and socket calls are stubbed. The address-probe stubs assert the expected multicast destination and emit no network packets or socket operations.
- The test runs with AddressSanitizer and UndefinedBehaviorSanitizer.

## Runtime behavior changes

None in the production module. A new host-only test binary is generated under ignored `build/resident-host/` when run.

## Validation performed

- `MBEDTLS_INCLUDE=<pinned 3.6.7 include directory> bash scripts/test-resident-host-lifecycle.sh` completed successfully in Lima.
- PASS cases: worker-create failure rollback; network-init failure rollback; clean worker stop plus retry after NetCtl unload failure; retry after network-finalize failure; retry after network-module unload failure; bounded stop timeout followed by worker completion and successful second stop.
- Repeated the complete ASan/UBSan simulation three times after its initial pass, then refined timeout synchronization to wait for mock poll entry and ran four more complete passes. All six cases passed in all eight runs.
- Inspected the simulator object imports with `nm -u`; the module's `socket`, `connect`, `getsockname`, and `close` references resolve to `resident_host_*` stubs, not POSIX network functions.
- ASan/UBSan reported no issue.
- The simulated socket API arguments were checked; no real network socket or packet was used.
- No RPCS3 package was installed, no emulator setting changed, and no physical PS3 was contacted.
- During authoring, a path mistake briefly placed newly-created untracked test files in the stable checkout. Those files were moved to this experimental worktree before staging; verification confirmed the stable checkout returned to clean `main`. No stable file was committed or pushed.

## Immediate results

- Six mocked lifecycle scenarios passed against the actual `module_start` and `module_stop` functions.
- The stop timeout test waits for a worker completion event with a bounded deadline rather than assuming the stop callback consumes its full configured timeout.

## Known limitations / follow-ups

- Mocked syscalls cannot validate RPCS3 or PS3 syscall semantics, import resolution, thread scheduling, network module behavior, resource accounting, or loader compatibility.
- The receiver and mDNS internals are stubs; no RAOP protocol session ran in this test.
- RPCS3's real three-cycle load/start/stop/unload test remains required. Hardware remains NOT READY.

## Risks introduced

- Mock behavior can diverge from target behavior. The report and README explicitly keep that boundary visible.
- The test includes POSIX socket declarations but compiles the module with socket/connect/getsockname/close remapped to assertions, so accidental unmocked calls should fail at link time.

## Decision record (embedded)

### Decision: simulate selected lifecycle behavior with platform mocks

- **Context:** RPCS3 cannot be operated while the desktop is locked, and start/stop cleanup is safety critical.
- **Options considered:** Leave lifecycle source compile-only; add a host simulator around the same production translation unit; or attempt unverified emulator command-line install/run options.
- **Why chosen:** Linking the production translation unit to narrow mocks can execute rollback and retry control flow now, while keeping emulator and hardware claims separate.
- **Tradeoffs accepted:** The simulation increases test and mock code but does not prove target system-call behavior.

## Next recommended step

Run this host simulation and the existing protocol suites in CI or Lima, then run the dedicated three-cycle harness in RPCS3 after the desktop is unlocked. Keep hardware readiness NOT READY until the actual emulator lifecycle passes.
