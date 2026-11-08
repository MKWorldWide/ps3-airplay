# Extend lifecycle fault matrix and rebuild candidate

## Timestamp in EDT

2026-11-08 07:24 EDT

## Scope of the pass

Added host-mock coverage for NetCtl initialization failure and retry after `sysThreadJoin` failure, then ran the lifecycle simulator four times and repeated the full Lima host/PS3DK build and package-integrity workflow. Refreshed ignored candidate artifacts and report hashes.

## Why this pass was needed

The lifecycle matrix did not yet cover an optional NetCtl startup failure or a failed thread join. A final end-to-end build also confirms that the candidate package remains reproducible after the earlier development and test iterations.

## Files changed

- `tests/resident/host_lifecycle_test.c`
- `experimental/resident/README.md`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0724_lifecycle_matrix_and_rebuild.md`

## Architecture / logic changes

- No production PRX logic changed.
- The NetCtl-init mock now verifies that the receiver can become ready with base network available, and that the loaded NetCtl module is still unloaded on stop even when its init call failed.
- The join mock injects one failure after worker exit. The first `module_stop` must fail before network cleanup; the second must join the worker and release the network modules.
- The full build regenerated the resident PRX, SELF containers, harness ELF/SELF, and PKG from the matching Lima source tree.

## Runtime behavior changes

None in the production module. Host-only fault simulation was extended.

## Validation performed

- Ran `bash scripts/test-resident-host-lifecycle.sh` four consecutive times with AddressSanitizer and UndefinedBehaviorSanitizer; all eight cases passed in each run.
- Re-ran `scripts/build-lima-resident-rpcs3-harness.sh`; the full host regression suite, PPU build, `prx-gen check`, harness package build, and extracted-PRX byte comparison passed.
- Compared the production source and build-script SHA-256 values in the worktree and Lima; the checked inputs matched.
- Copied the rebuilt candidate binaries into the ignored `build/rpcs3-candidate/` directory and recomputed SHA-256. PRX, SPRX, fake SPRX, harness ELF, and SELF hashes match prior values; the latest PKG hash is recorded in the overnight report.
- The rebuilt PKG hash differed from the previous PKG build while the PRX and harness code hashes stayed the same. The cause was not isolated; each PKG hash is treated as identifying that exact container.
- RPCS3 remained inaccessible. No title was installed, no emulator setting changed, and no physical PS3 was contacted.

## Immediate results

- Eight host-mocked lifecycle/fault cases passed repeatedly.
- Candidate PRX and test package were rebuilt and package payload integrity passed.
- Hardware readiness remains NOT READY because RPCS3 load/start/stop/unload behavior is unobserved.

## Known limitations / follow-ups

- These mocks do not establish syscall behavior, PRX loader compatibility, Cobra/Evilnat compatibility, real network behavior, or audible output.
- The RPCS3 three-cycle harness still needs to run after the desktop is unlocked.
- The one-time historical receiver observation failure remains unexplained despite ten targeted repetitions passing.

## Risks introduced

- Host stubs can differ from PS3 behavior. The report labels their evidence as host-only.
- Package-container hashes may vary across builds; compare the actual package payload and retain the exact hash of the selected artifact.

## Decision record (embedded)

### Decision: add optional NetCtl and failed-join cases before ending local work

- **Context:** Both branches affect cleanup ownership, and the emulator remains unavailable.
- **Options considered:** Stop at existing host simulation; add the two host fault cases; or attempt additional emulator command-line options.
- **Why chosen:** These cases exercise distinct cleanup decisions in the production control flow without changing the target or claiming emulator validation.
- **Tradeoffs accepted:** The mock suite grows, but its evidence remains bounded to host control flow.

## Next recommended step

After desktop access is restored, install and run `RPRX90001` in RPCS3 through three load/start/stop/unload cycles and record both harness and resident logs. Keep physical hardware testing gated on observed success and a recovery plan.
