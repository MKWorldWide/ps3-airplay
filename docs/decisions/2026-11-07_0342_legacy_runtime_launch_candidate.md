# Build a legacy-runtime launch candidate

## Timestamp (America/New_York)

2026-11-07 03:42 EST (New York local time; daylight saving time has ended).

## Scope of the pass

Added a selectable legacy-runtime path for the standalone receiver, made the package builder accept an explicit version/output/ELF, built a separate AIRPLAY02 v0.1.3 candidate, validated it, ran the existing host test executables, and recorded the hardware hypothesis and approval gate.

## Why this pass was needed

The installed v0.1.2 app returns XMB error `80028F21`; its installed SFO and EBOOT match the validated package and no fresh application log was written. The precise meaning of the error remains unknown. An independent PS3 homebrew project reports a pre-`main()` failure with PSL1GHT 2021+ runtimes on HFW 4.93 + HEN and success using a 2020 runtime. The report does not cover Evilnat CFW, but it suggests a concrete, reversible toolchain comparison.

## Files changed

- `prototype/standalone-receiver/Makefile`
- `scripts/package-lima-standalone-receiver.sh`
- `scripts/build-lima-legacy-runtime.sh`
- `docs/TEST_MATRIX.md`
- `docs/DEV_SETUP.md`
- `docs/ROADMAP.md`
- `docs/RISKS.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/lessons-learned.md`
- `docs/reports/2026-11-07_v013_legacy_runtime_candidate.md`
- `docs/decisions/2026-11-07_0342_legacy_runtime_launch_candidate.md`

## Architecture / logic changes

The Makefile now selects a legacy `select()` header path without defining `__SELECT_H__` when `LEGACY_RUNTIME=1`; the normal PS3DK build keeps its existing header workaround. A dedicated build script verifies the exact legacy PSL1GHT commit, configures Mbed TLS with the selected PPU toolchain, builds it, and then rebuilds the receiver in an isolated object directory. The packaging script can now take `APP_VERSION`, `ELF`, and `OUT_DIR` overrides while preserving its existing defaults.

The candidate used PSL1GHT commit `6e565a70e927f55813babe86dc6f64149535f908` dated 2020-11-25 and the verified PS3DEV Linux x86_64 bundle with PPU GCC 7.2. The current package remains unchanged and the candidate has not been sent to the console.

## Runtime behavior changes

No runtime behavior has been verified to change on hardware. The candidate contains the same receiver and display sources as v0.1.2, linked against the earlier PSL1GHT runtime. A launch result can test whether that runtime path avoids the current XMB error; it cannot by itself prove a sole root cause.

## Validation performed

- Built the standalone PPU ELF using `scripts/build-lima-legacy-runtime.sh`; `readelf` reports ELF64, big-endian, PowerPC64.
- Rebuilt Mbed TLS 3.6.7 with the same legacy PPU GCC and linked the complete receiver successfully.
- Ran 11 existing host executables: RAOP crypto, session crypto, session state, SDP, mDNS, persistent RTSP/RTP receiver loopback, RTSP listener/log, ALAC, PCM ring, diagnostic tone, and the additional session test; all passed in this run.
- Built and extracted the AIRPLAY02 v0.1.3 NPDRM package. SFO reports `APP_VER=0.1.3` and `TITLE_ID=AIRPLAY02`; package and EBOOT content IDs match; EBOOT, icon, and 2,764,800-byte RGB background are present and compare against staged inputs.
- Candidate PKG: 3,183,408 bytes; SHA-256 `98d8701ab018df882e30ba28f851a994c195d0a144dafe7c68c77d86d94126aa`.
- Guest free space after build: 12 GiB. No console files or settings were changed during this pass.
- `git diff --cached --check` passed. Manual privacy review and targeted scans found no private identifiers, secrets, or agent instruction files in the staged diff; `gitleaks` and `trufflehog` were unavailable.

## Immediate results

The older-runtime receiver compiles and packages cleanly with a matching crypto dependency. All 11 existing host test executables passed. The candidate is now ready for a distinct approved hardware launch test. No launch, display, tone, controller exit, RTSP, RTP, or music behavior is yet established for v0.1.3.

## Known limitations / follow-ups

- No public reference located defines `80028F21` for this precise context.
- The upstream compatibility evidence is HFW 4.93 + HEN only, not Evilnat CFW.
- A hardware launch test needs separate approval for the exact package transfer and installation. The existing v0.1.2 app and local rollback artifacts should be preserved.
- Earlier direct receiver-loopback test flakiness remains a concern despite this run passing.

## Risks introduced

The alternate toolchain may have different SDK headers or behavior on this console. The legacy build path is opt-in, not the default. Installing a package with the same title ID can replace the existing application when the user confirms in XMB; retain the prior packages and stop after any freeze or abnormal shutdown.

## Decision record (embedded)

### Decision 1: Test a 2020 PSL1GHT runtime before altering receiver logic

- **Context:** v0.1.2 launch fails before a fresh startup log, while package identity matches.
- **Options considered:** Change graphics waits, alter NPDRM packaging, produce a minimal unrelated test app, or compare a documented older runtime while keeping receiver sources constant.
- **Why chosen:** A maintained project reports a closely matching pre-`main()` failure and an older-runtime success on 4.93 HEN; compiling the same receiver creates a direct, reversible comparison.
- **Tradeoffs accepted:** This evidence is not for Evilnat CFW, and toolchain versions change several variables together. The candidate can test the hypothesis but cannot isolate a specific upstream commit as causal.

### Decision 2: Keep the candidate local pending console authorization

- **Context:** The task's deployment rules require approval before replacing a staged package or installing a new build.
- **Options considered:** Upload now under the same filename, upload under a versioned filename after presenting the exact hash and rollback, or keep the artifact local.
- **Why chosen:** Complete build, package inspection, and host checks first; request approval with exact transfer details only after a reviewable candidate exists.
- **Tradeoffs accepted:** Hardware evidence awaits the user's deployment approval.

## Next recommended step

Commit and push the validated build path and report after privacy review. Then ask approval to upload `ps3-airplay-receiver-0.1.3.pkg` (SHA-256 above) as a new file in `/dev_hdd0/packages/`, retain the staged v0.1.2 package, and install/launch through XMB only after the user approves.
