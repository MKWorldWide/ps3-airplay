# v0.1.3 Launch Failure: SDK Runtime Comparison

**Timestamp:** 2026-11-07 04:00 America/New_York (EST)

## Scope of the pass

Recorded read-only hardware validation of installed AIRPLAY02 v0.1.3, corrected the v0.1.3 candidate report, updated project state/risks/roadmap/test matrix/lessons, and compared available PS3 SDK paths for the next launch diagnostic.

## Why this pass was needed

The user reports v0.1.3 returns the same XMB launch error (`80028F21`) as v0.1.2. The earlier report still said v0.1.3 was not installed and contained the pre-package SELF input hash in the extracted EBOOT row. We also need a useful diagnostic path beyond rebuilding with a different PSL1GHT revision.

## Files changed

- `docs/DEV_SETUP.md`
- `docs/RISKS.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v013_legacy_runtime_candidate.md`
- `docs/reports/2026-11-07_v013_launch_failure.md`
- `docs/decisions/2026-11-07_0400_sdk-runtime-comparison.md`

## Architecture / logic changes

No runtime source or package code changed. Documentation now separates three identities: the 402,880-byte pre-package SELF input, the 402,928-byte package-extracted EBOOT, and the installed EBOOT, which matches the package extraction. The hardware report states that the user observed the same launch error, while the unchanged log only means no new log bytes were retrieved; it does not prove a pre-`main()` failure.

## Runtime behavior changes

None. This was a verification and documentation pass. The next proposed diagnostic is a minimal native PS3DK launch control built in Lima, separate from the AirPlay application. No new package was transferred or installed in this pass.

## Validation performed

- Reviewed repository memory and active scratchpad before editing.
- Verified repository branch and working tree.
- Checked current source for the hard-coded startup event; v0.1.3 source still emits `PKG_VERSION_0.1.2`.
- Compared installed identity and FTP log evidence recorded in the current-turn hardware inspection.
- Read-only inspected the existing Lima VM: it is running; PS3DK source/toolchain staging and host package tools are present. Read `scripts/build-lima-smoke.sh`; it produces protocol objects and a PRX sample, not a standalone app for XMB launch comparison.
- Researched upstream [Thermal Bench build documentation](https://github.com/mohasi/ps3-dev) and [PS3DK documentation](https://github.com/FirebirdTA01/PS3DK).
- No host tests were rerun: this pass changed documentation only.
- Staged-diff review and `git diff --cached --check` pass. Targeted privacy searches found no sensitive values; one generic match was the report's statement that unique console identifiers were omitted. No local AGENTS files are tracked or staged. `gitleaks` and `trufflehog` are unavailable.

## Immediate results

- v0.1.3 is confirmed installed by matching SFO and EBOOT identity, but launch still fails by user report with `80028F21`.
- No fresh startup log event was observed; cause remains unknown.
- Legacy PSL1GHT runtime alone did not resolve the launch failure.
- Thermal Bench uses Sony's official proprietary SDK 4.75 in a Windows 7 VM/MSBuild workflow. This is an alternative only for a properly licensed SDK installation and requires a port.
- PS3DK provides a native runtime/API path alongside PSL1GHT compatibility, making a small native launch control the lowest-cost next comparison in existing Lima.

## Known limitations / follow-ups

- No console-side loader trace or fresh logger result identifies where startup fails.
- `main.c`'s version event is stale (`PKG_VERSION_0.1.2`) and needs correction before the next receiver package.
- Availability of a properly licensed Sony SDK environment is unknown.
- Native PS3DK control build and hardware validation remain outstanding.
- Display, clean exit, iPhone connection, RTP, ALAC, and real streamed audio are untested for v0.1.3.

## Risks introduced

No implementation risk was introduced. Hardware tests that repeatedly launch an app which currently fails are not warranted until a different diagnostic build provides a concrete change. Any new hardware installation must preserve the staged v0.1.2 rollback package and requires a separate transfer/install approval.

## Decision record

### Decision 1: Use native PS3DK for the next launch-control comparison

- **Context:** Both the regular and legacy PSL1GHT receiver packages fail to launch. Lima already contains PS3DK tooling, and upstream PS3DK documents a native runtime/API separate from its PSL1GHT compatibility layer.
- **Options considered:** Build yet another PSL1GHT variant; immediately port the full app to Thermal Bench's Sony SDK; build a minimal native PS3DK control first.
- **Why chosen:** The control is the smallest test that can isolate the runtime/package path while reusing the current Linux host and avoiding a premature receiver rewrite.
- **Tradeoffs accepted:** The control only tests its own small startup path. Passing it will not prove AirPlay networking, codecs, audio, or every receiver dependency work natively.

### Decision 2: Keep Sony SDK as a conditional route

- **Context:** Thermal Bench states it builds against Sony SDK 4.75 in a Windows 7 VM with MSBuild.
- **Options considered:** Adopt the same proprietary SDK workflow now; first check whether a properly licensed installation exists; stay with open-source PS3DK.
- **Why chosen:** Use open-source PS3DK first, and check the licensed SDK only as an available diagnostic alternative. Do not obtain or use an unlicensed SDK.
- **Tradeoffs accepted:** Porting to Sony SDK may offer a useful compatibility comparison but entails API/runtime/packaging work and a separate Windows VM build path.

### Decision 3: Preserve uncertainty about the failure point

- **Context:** The retrieved log was unchanged, but logger open/write failure is possible.
- **Options considered:** Label failure as pre-`main()`; state only that no fresh log was observed.
- **Why chosen:** The evidence supports only the latter.
- **Tradeoffs accepted:** Root cause remains unresolved until stronger loader or startup evidence is obtained.

## Next recommended step

Build and inspect a minimal native PS3DK launch-control package in Lima, correct the receiver's hard-coded version marker and logger-open diagnostics, then present exact artifact details and rollback before asking to deploy it. Continue with the receiver only after the launch boundary is understood.
