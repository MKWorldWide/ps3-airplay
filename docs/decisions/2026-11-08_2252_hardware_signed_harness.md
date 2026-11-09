# Build a signed resident hardware diagnostic package

## Timestamp in EDT

2026-11-08 22:52 EDT

## Scope of the pass

Make the experimental lifecycle harness configurable for a hardware-compatible
signed SPRX, add a dedicated 50-cycle package build script using unique title
ID `RPRX90002`, and build and validate the fresh signed SPRX and package. The
physical test installation is a separate next pass.

## Why this pass was needed

The prior RPCS3 package used a fake SPRX and title ID `RPRX90001`. That
artifact cannot be installed as the production module on the development PS3,
and the shared structured logging ABI had not been rebuilt with the resident.

## Files changed

- `experimental/resident/harness/CMakeLists.txt`
- `experimental/resident/harness/source/main.c`
- `scripts/build-lima-resident-hardware-harness.sh`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_2252_hardware_signed_harness.md`

## Architecture / logic changes

The test harness accepts a configurable title ID, resident SPRX filename,
GameOS module path, package overlay, and cycle count while retaining the
RPCS3 fake-SPRX defaults. The hardware build script rebuilds the resident,
classifies signed and fake SELF types separately, packages the production
`.sprx` under `RPRX90002`, extracts the PKG for verification, and refuses to
continue unless exactly one embedded SPRX matches the fresh signed output.
The script does not change Cobra settings or create an autoload configuration.

The title-ID validator now checks the allowed uppercase alphanumeric alphabet
and then checks exact length separately. This is compatible with the CMake
regex engine used by the pinned build environment.

## Runtime behavior changes

When the hardware diagnostic title runs, its harness points
`sys_prx_load_module` at the production SPRX stored in its own USRDIR and
defaults to 50 start/stop/unload cycles. Existing RPCS3 configuration still
points at its `.fake.sprx`. Production receiver behavior is unchanged.

## Validation performed

- `scripts/build-lima-resident-hardware-harness.sh` host crypto, protocol,
  encrypted RTP, ALAC, PCM, resampling, sanitizer, and PPU object checks — PASS.
- CMake PPU resident build using PS3DK v0.27.0 and Mbed TLS 3.6.7 — PASS.
- `prx-gen check` — PASS: one load segment, 6,300 relocations, one management
  export, and imports `sys_net`, `cellSysmodule`, `cellNetCtl`, `cellAudio`,
  and `sysPrxForUser`.
- Signed/fake SELF container distinction — PASS; production SPRX type is
  `0x00010001`; RPCS3 fake SPRX type is `0x80000001`.
- Harness CMake build and SELF packaging for `RPRX90002`, configured for 50
  cycles — PASS.
- `pkg --list` and independent extraction — PASS; one embedded SPRX matched
  the freshly built signed SPRX byte for byte; package type is 1.
- `bash -n scripts/build-lima-resident-hardware-harness.sh` — PASS.
- A host-only syntax check against the old local mock headers initially failed
  because they do not declare the new module-list APIs. The actual PPU compile
  with the pinned SDK succeeded; the mock check is not treated as a code
  failure or a substitute for the PPU build.
- Physical console installation, launch, log retrieval, sound, and network
  streaming — UNTESTED at this pass boundary.

## Immediate results

Fresh PRX SHA-256: `3b61d7739b371a4ebcb12b8723a0b11e7a29fbd96a248e650bd01a52570e7986`.
Fresh signed SPRX SHA-256:
`9fae9ed955c023aca2077654625510e80766f899c9eaa4998cad4af5f1588817`.
Harness SELF SHA-256:
`013fda9e760b02172c478c5ffd0d2d5496930f211c53f02eb00d01165dd8bb52`.
Installable PKG SHA-256:
`62403e6b11795c6fecf2d5b8a896d213dbf1df11905d771af43448a78c886f4f`.

All four build artifacts are in ignored `build/resident-hardware/`. The
package is ready for the explicitly requested nonpersistent diagnostic
installation; no runtime success is claimed yet.

## Known limitations / follow-ups

- The package build does not establish that Evilnat 4.93 accepts this signed
  module's key table or that the module's imports resolve on the test unit.
- The harness log must be retrieved after launch to confirm logger health,
  worker readiness, stop/join, unload, and the final module count baseline.
- No synthetic AirPlay stream was sent through the resident, no audible output
  was verified, and no foreground/game mixing test was performed.
- `sysThreadJoin` remains unbounded if a worker fails to exit. The diagnostic
  run must be monitored and stopped if lifecycle progress stalls.

## Risks introduced

- A new test title ID and custom module path are compiled into the diagnostic
  harness. The package embedding and SFO identity were checked, but the XMB
  installation and module path still require on-device confirmation.
- The 50-cycle hardware run exercises stop/unload repeatedly. It is a test-only
  package, not a resident boot plugin, and does not alter automatic loading.

## Decision record (embedded)

### Decision 1: separate hardware and emulator package identities

- **Context:** The existing RPCS3 title embeds an emulator-only fake SPRX.
- **Options considered:** Reuse the emulator package; install a standalone
  module; build a new app package with the signed production SPRX.
- **Why this option was chosen:** A title-owned module path allows the existing
  harness to exercise load/start/stop/unload without installing a persistent
  plugin or changing the release title.
- **Tradeoffs accepted:** A second title consumes a small amount of installed
  storage and requires its own package identity.

### Decision 2: require exact package-to-build SPRX identity

- **Context:** A hardware package must not silently contain the RPCS3 fake or a
  stale production module.
- **Options considered:** Trust CMake dependencies; inspect only the package
  file list; extract and byte-compare the packaged module to the fresh build.
- **Why this option was chosen:** Extraction and exact comparison directly
  verify the payload that the console will install.
- **Tradeoffs accepted:** The build takes extra time to extract and check the
  package.

## Next recommended step

Upload the uniquely named package to the development PS3's package directory,
verify the transferred size/hash, install the `RPRX90002` title, launch the
diagnostic harness, then retrieve its log read-only and verify each lifecycle
cycle and the final module-count baseline.
