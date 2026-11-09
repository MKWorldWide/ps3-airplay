# Isolate resident SPRX loader rejection

## Timestamp in EDT

2026-11-09 00:07 EDT

## Scope of the pass

Added two independent production-signing probes to the hardware diagnostic
matrix, improved failed-load repetition and module-count logging, advanced the
isolated package to SFO `00.04`, and compared the generated container/layout
against a released CFW plugin. The production resident SPRX and standalone
receiver were not changed. The `00.04` package was built and inspected but not
installed because the development console was unreachable from the host.

## Why this pass was needed

The `00.03` hardware run showed that both the no-import probe and the standard
SDK-macro reference returned `0x80010007` before `module_start`. Those probes
used the same PS3DK `make_sprx` signing path and the same custom PRX generator as
the resident, so they did not isolate container signing from the module ELF.

## Files changed

- `experimental/resident/harness/sfo.xml`
- `experimental/resident/harness/CMakeLists.txt`
- `experimental/resident/harness/source/main.c`
- `experimental/resident/harness/probes/minimal.c`
- `experimental/resident/harness/probes/reference.c`
- `experimental/resident/harness/probes/reference-exports.S`
- `scripts/build-lima-resident-hardware-harness.sh`
- `scripts/validate-resident-containers.py`
- `scripts/sign-lima-diagnostic-probes-scetool.sh`
- `docs/experimental/RESIDENT_OVERNIGHT_REPORT.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-09_0007_resident_loader_isolation.md`

## Architecture / logic changes

The harness keeps the resident built by the existing PS3DK route and signs
only its minimal and SDK-layout comparison probes through an optional local
scetool path. That path uses the public CEX plugin parameters observed in the
released webMAN MOD build: SELF flags/key revision `0x000A`, the same auth ID,
vendor, program type, SCE version, and control/capability flags. It uses
locally supplied scetool/key data, which is not tracked. The container checker
now parses SELF flags and file type separately, allowing flags `0x0001` for
PS3DK output and `0x000A` for the independent comparison while retaining the
fake-container check.

`run_loader_probe` now continues through all configured attempts after a
rejected load, checks that module count returns to its baseline after each
failure, and reports a rejected cycle. Successful load/start/stop/unload logic
is unchanged. This makes repeated rejection observable without representing it
as a lifecycle pass.

The external reference artifact was inspected locally and not copied into the
repository. It shares the candidate's auth ID, vendor, program type and SCE
version. Its observed SELF flags are `0x000A` rather than `0x0001`; its
decrypted ELF has two `PT_LOAD` segments plus `PT_SCE_PPURELA`, while current
PS3DK output has one RWX `PT_LOAD` plus `PT_SCE_PPURELA`. This comparison
identifies testable differences but does not prove either one causes `ENOEXEC`.
Reference sources: [webMAN MOD release 1.47.50](https://github.com/aldostools/webMAN-MOD/releases/tag/1.47.50)
and its [SPRX build command](https://github.com/aldostools/webMAN-MOD/blob/master/_Projects_/netiso/Makefile).

## Runtime behavior changes

The next hardware package is `RPRX90002` SFO `00.04`. When built with
`SCETOOL` and local `SCETOOL_PS3_DATA`, the harness tests both scetool-signed
probes before the unchanged resident module. Each rejected probe load will be
attempted three times and followed by a module-count check. No Cobra, autoload,
or standalone receiver behavior changed.

## Validation performed

- Read-only inspection of PS3DK PRX generation docs/runtime and the
  `00.03` captured hardware log.
- Compared the local webMAN MOD 1.47.50 released plugin and its public
  Makefile signing command against the PS3DK-signed module headers and PRX
  layout. The external reference was not staged or copied into the repository.
- Built scetool locally and signed both raw probes with the CEX plugin settings;
  the header validator confirmed flags/type `0x000A/0x0001`.
- Ran `scripts/build-lima-resident-hardware-harness.sh` in Lima with the pinned
  PS3DK v0.27.0 toolchain. Host RAOP/protocol/ALAC/PCM/resampling/sanitizer
  checks passed; resident and harness PPU builds passed; `prx-gen check` passed.
- Extracted the `00.04` package and inspected SFO identity/version, module
  count, and hashes. SFO reports title `RPRX90002`, `APP_VER=00.04`,
  `VERSION=00.04`; the PKG contains the resident and both freshly signed probes.
- Checked local FTP/HTTP/ping reachability for the development console. The
  host could not connect, so no upload or install was attempted.
- No secret-scanning tool was already installed. Targeted staged-file searches
  for local paths, private network/MAC identifiers, emails and private-key
  blocks were clear; the complete staged diff was reviewed manually.
- `00.03` hardware log confirms both load errors were `0x80010007`, each
  rejected load preserved module count 11, and final cleanup returned to count
  9. The new three-attempt behavior has compiled but has not run on hardware.

## Immediate results

- `00.03` shows the loader rejects the whole shared PS3DK-generated path before
  `module_start`; import resolution alone is not an adequate explanation
  because the minimal probe has no imports.
- The `00.04` build now varies the signing/container metadata while preserving
  the raw probe PRXs, so its hardware result can separate that dimension from
  PRX layout.
- `00.04` package SHA-256:
  `af75bf0150629064143117d44f38c184f754704336c62e19c30f68e9e5770e2e`.
- Minimal scetool-signed SPRX SHA-256:
  `62d2f94a07dc41dc984e52381683fcd59c537b642cd20a2536e046eacdbff0b7`.
- Reference scetool-signed SPRX SHA-256:
  `d9c0579b80a199690089fd72732499a0d0d395b5282033cb10d158938d51a2b0`.
- Hardware test remains BLOCKED: the console could not be reached, and no
  `00.04` runtime result exists.

## Known limitations / follow-ups

- Do not infer the cause from ELF/SELF header differences alone. Install and
  run `00.04` when console connectivity is available.
- If scetool-signed probes load, complete three load/start/stop/unload cycles
  and compare the PS3DK-signed resident result. If scetool probes also fail,
  next build a standard two-segment PRX to isolate raw layout/relocations.
- The current resident has not reached `module_start`; RAOP receive, PCM
  submission, audible output, background coexistence, and physical hardware
  readiness remain untested or blocked.
- The captured `00.03` run tested only the first failed load for each probe.

## Risks introduced

- The `00.04` probes use a different public signing tool and capability profile.
  Their acceptance is unverified until executed on the target CFW.
- The optional signing script requires locally provided signing data and the
  matching scetool build. It contains no embedded keys and must not be run with
  unreviewed signing parameters.
- No hardware behavior can be inferred from successful PPU build, SELF
  structure inspection, package extraction, or a known-good external artifact.

## Decision record (embedded)

### Decision 1: vary signing metadata before changing resident PRX generation

- **Decision:** Keep the resident artifact unchanged and create separate
  scetool-signed minimal/reference probes.
- **Context:** Both `00.03` probes failed with `ENOEXEC`, but they shared the
  resident's PS3DK signing/generation route.
- **Options considered:** Change the resident signer/PRX generator immediately;
  use a fake SPRX; or add an independently signed comparison while preserving
  raw PRX bytes.
- **Why this option was chosen:** The released plugin comparison showed a
  reproducible SELF flags/key-revision difference; a separate probe can test
  it without risking the resident source or the stable receiver.
- **Tradeoffs accepted:** This creates another diagnostic package and requires
  target access; local signature/header checks still do not prove target
  acceptance.

### Decision 2: continue after rejected loads

- **Decision:** Execute all three configured load attempts when a probe is
  rejected, checking module count after each.
- **Context:** The earlier helper returned after the first failure, so its
  configured three-attempt diagnostic did not actually repeat.
- **Options considered:** Preserve fail-fast behavior or repeat only the load
  operation while keeping the overall probe status failed.
- **Why this option was chosen:** Repeated load rejection can expose cleanup
  leaks or unstable loader behavior without falsely claiming start/stop cycles.
- **Tradeoffs accepted:** The extra attempts add a small amount of test time;
  they still do not substitute for successful lifecycle cycles.

## Next recommended step

Restore access to the development PS3, transfer and byte-verify the local
`00.04` package, install it under the isolated diagnostic title, launch it, and
capture all three load attempts per probe. Do not resume resident audio testing
until the resident itself completes load/start/stop/unload and returns module
count to baseline.
