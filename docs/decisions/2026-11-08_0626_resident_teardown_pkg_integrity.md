# Resident teardown and package integrity

## Timestamp in EDT

2026-11-08 06:26 EDT

## Scope of the pass

Made resident network cleanup errors visible and retryable, and made the harness build force and verify a fresh package payload. Ran the full resident build/test path and the existing forensic host suite.

## Why this pass was needed

Lifecycle review found that the module ignored `sys_net_finalize_network` and `cellSysmoduleUnloadModule` errors, cleared resource flags unconditionally, and could report a clean stop with dependencies still loaded. Package inspection also showed the SDK helper could produce an unchanged PKG after the embedded PRX had changed.

## Files changed

- `experimental/resident/module.c`
- `experimental/resident/README.md`
- `scripts/build-lima-resident-rpcs3-harness.sh`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0626_resident_teardown_pkg_integrity.md`

## Architecture / logic changes

- Network module cleanup now stops at the first failed dependency release, logs the returned error, retains the active state flag, and reports failure. A later call can retry from the exact remaining state.
- If cleanup after a failed startup cannot release network modules, the PRX stays resident without a worker so the module stop callback can retry cleanup. The separate harness handles that case through a bounded ready wait and a stop attempt.
- The harness build removes its previous PKG before asking the SDK to package the target, extracts the built package into a temporary directory, and rejects the output unless its embedded PRX exactly matches the current raw PRX.

## Runtime behavior changes

A normal stop reports `SYS_PRX_STOP_FAILED` if Net module finalize or module unload fails and keeps the PRX loaded for retry. Startup cleanup failure is logged and leaves the PRX resident until stop retries. Normal successful teardown remains ordered: worker resources close, crypto releases, NetCtl module releases, networking finalizes, Net module releases, then the module logs stop and closes its log.

## Validation performed

- Full `scripts/build-lima-resident-rpcs3-harness.sh`: host crypto/session/SDP/mDNS, RTSP/RTP, encrypted receive/decode, ALAC, PCM, gain, tone, PPU build, PRX check, SELF/PKG build all passed.
- `prx-gen check`: one PT_LOAD, 6094 relocations, one management export, four imports; OK.
- Extracted package payload compared byte-for-byte with freshly built PRX; `PKG embeds the current resident PRX: PASS`.
- `scripts/run-forensic-tests.sh`: RTP sequence-wrap/reorder/resend/flush, 600-second-equivalent SRC loop, 10,000-packet jitter schedule, bounded resend retries, ALAC reference decode, display resume, and host-only cost probes passed.
- Three separate receiver-test repeats and three encrypted audio pipeline repeats passed in the prior pass; the full suite passed again after teardown changes.
- RPCS3 was not run because the Mac remains locked. No physical console or emulator settings were changed.

## Immediate results

- A false-success path in PRX cleanup was removed; remaining network module state can be retried.
- The installed diagnostic PKG is now mechanically checked against the final built PRX, rather than merely listing the file path.
- Host loopback tests confirm synthetic encrypted RTP produces reference-matching ALAC PCM. The resident module still clears/discards decoded PCM.

## Known limitations / follow-ups

- RPCS3 load/start/stop/unload execution remains blocked by unavailable UI access. Do not interpret build or host tests as runtime lifecycle evidence.
- `cellNetCtlTerm` has a void API and cannot report failure. The module can only observe the subsequent module-unload result.
- `sysThreadJoin` has no explicit timeout in the used API; the worker is checked as fully cleaned and at its exit path before join, but this still needs emulator observation.
- No PRX output audio, physical PS3/Cobra compatibility, or background/game-audio coexistence is established.

## Risks introduced

- Retaining the PRX after startup cleanup failure requires the loader to invoke the stop callback before releasing it; this needs actual RPCS3 and hardware validation.
- Retrying a failed network finalize/unload may return another error if the PS3 runtime has changed shared network state. The PRX now avoids dropping its ownership flags in that case.
- Package verification depends on SDK `pkg --extract` support and `cmp`; these were observed working with the current Lima toolchain.

## Decision record (embedded)

### Decision: retain cleanup ownership until release succeeds

- **Context:** Module teardown must not claim success or unload code while networking modules may still be owned.
- **Options considered:** Ignore failures; clear flags and continue; retain flags, stop cleanup at the dependency boundary, and return stop failure for retry.
- **Why chosen:** Preserves ownership truth and avoids using an absent dependency to finalize the network stack.
- **Tradeoffs accepted:** A persistent error can leave the module resident; diagnostics now expose that state rather than hiding it.

### Decision: force package regeneration and compare extracted payload

- **Context:** SDK helper's generated dependency list did not include overlay files; a package could contain stale bytes while listing the expected pathname.
- **Options considered:** Trust CMake target ordering; touch unrelated inputs; always remove the package output and compare the extracted member.
- **Why chosen:** Explicit invalidation plus byte comparison is simple and independently verifies the exact artifact.
- **Tradeoffs accepted:** The diagnostic package is rebuilt each run and requires temporary disk space for extraction.

## Next recommended step

Complete local RPCS3 installation and three lifecycle cycles once the desktop is accessible. Record exact syscall results and module logs, then write the overnight report and mark hardware readiness only from verified evidence.
