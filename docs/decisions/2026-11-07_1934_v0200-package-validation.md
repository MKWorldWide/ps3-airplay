# Validate the v0.2.0 / SFO 00.40 package candidate

## Timestamp

2026-11-07 19:34 America/New_York

## Scope of the pass

Build the standalone receiver and finalized NPDRM package from the committed source revision, validate package structure and SFO metadata, preserve the artifact locally, and document the verification boundary.

## Why this pass was needed

The user requested a 0.2.0 package while retaining the next SFO sequence value. The source/test pass was committed and pushed; a concrete package artifact was needed to verify the version values and tone removal.

## Files changed

- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/decisions/2026-11-07_1926_startup-tone-and-v020-candidate.md`
- `docs/memory.md`
- `docs/reports/2026-11-07_v0200_candidate.md`
- `docs/scratchpad.md`
- this decision log

## Architecture / logic changes

No runtime source changes. The package was built from commit `b8bab2015551a7044dba26717611e524ed43f4ec` using the established PS3DK/PSL1GHT pipeline inside the existing Lima VM.

## Runtime behavior changes

This pass generates a candidate artifact only. It was not installed or launched in RPCS3 or on a physical PS3.

## Validation performed

- Full Lima host/sanitizer suite and PPU component compilation passed.
- Standalone ELF validation passed: 2,754,472 bytes, CellOS PPC64 executable, five PT_LOAD segments; SHA-256 `bc2c1f951624d0124f7771a50c0c3febe3d48c85cfd341691bc34cf4c5136875`.
- Finalized package validation passed: AIRPLAY02, SFO APP_VER `00.40`, five package entries, correct relative paths, EBOOT/SFO/icon/background extraction and equality checks, and NPDRM envelope. Package size 3,231,472 bytes; SHA-256 `924a01ef0ef812e1badb8e94011930f5187c4dba91b253e578e16ab72de4f29b`.
- The linked ELF contains `PKG_VERSION_0.2.0`; startup tone event strings are absent.
- Lima toolchain: PS3DK v0.27.0 PPU compiler 16.1.0, GNU ld 2.42, PSL1GHT `dd4e8c9be744e3273d665c393fa6d97a87402e2e`.
- Local artifact: ignored `work/builds/0.2.0/ps3-airplay-receiver-0.2.0.pkg`.

## Immediate results

The requested label and continuing SFO value are present in a valid finalized package. Tone-free behavior is confirmed at source/link level; hardware audio behavior is not yet tested.

## Known limitations / follow-ups

- No RPCS3 or physical PS3 launch, display, controller, connection, or streamed-audio evidence for this artifact.
- Current 0.2.0 label remains candidate status; stable diagnostic milestone requirements are not met.
- Continue measured resampler comparisons, RTP arrival-jitter and PCM buffer telemetry, and controller event diagnostics.

## Risks introduced

The version label can be mistaken for a stable release despite pending acceptance gates. Documentation explicitly describes it as a candidate. No console state was changed.

## Decision record

### Decision: Build and retain, but do not deploy, the package

- **Context:** The user requested package-version changes and removal of startup tone; no fresh hardware test was requested in this pass.
- **Options considered:** Stop after changing defaults; build and validate a concrete package; deploy to the console.
- **Why chosen:** A finalized artifact verifies SFO, package identity, and link contents while respecting the hardware boundary.
- **Tradeoffs accepted:** Runtime behavior on the console remains unknown until separately tested.

## Next recommended step

Commit and push these package-verification records after the routine privacy review, then continue the audio continuity diagnostics without representing this package as hardware-validated.
