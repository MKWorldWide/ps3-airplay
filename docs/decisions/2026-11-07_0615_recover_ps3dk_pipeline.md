# Recover the PS3DK standalone pipeline

**Timestamp:** 2026-11-07 06:15 EDT

## Scope of the pass

Recover the standalone AIRPLAY02 build path using the known v01.01 artifact as an executable-format reference. Add a strict ELF validation gate, make package creation safer, document the canonical Lima/PS3DK workflow, and update project state and evidence.

## Why this pass was needed

RPCS3 rejected the v0.1.6 package before app startup. Its CellOS OS/ABI marker was correct, but the complete executable layout had not been compared with the v01.01 artifact that the user reports ran and played audio.

## Files changed

- README.md
- docs/BUILD_PIPELINE.md
- docs/DEV_SETUP.md
- docs/RISKS.md
- docs/ROADMAP.md
- docs/TEST_MATRIX.md
- docs/lessons-learned.md
- docs/memory.md
- docs/scratchpad.md
- docs/reports/2026-11-07_build_pipeline_recovery.md
- docs/decisions/2026-11-07_0615_recover_ps3dk_pipeline.md
- prototype/standalone-receiver/Makefile
- scripts/build-lima-standalone-receiver.sh
- scripts/package-lima-standalone-receiver.sh
- scripts/validate-ps3-elf.py

## Architecture / logic changes

The canonical standalone build is the PS3DK v0.27.0 PPU toolchain in Lima with the pinned PSL1GHT checkout and matching Mbed TLS PPU library. A new Python validator checks ELF64/big-endian/PPC64, CellOS OSABI, ET_EXEC, the expected entry point, and program-header flags from the preserved v01.01 artifact. Both build and packaging scripts run it. The package flow now uses PSL1GHT package_finalize, validates extracted files, and refuses to overwrite an existing versioned package or intermediate.

The v01.01 and current PS3DK ELF have the same executable type/layout markers and initial PS3-specific PT_LOAD flags. The legacy v0.1.6 ELF does not. This is a verified difference, not a proven explanation of the RPCS3 error.

## Runtime behavior changes

No application runtime code changed. Invalid or mismatched ELF output is now rejected before package creation. The package script will not silently replace a package for an already-used version.

## Validation performed

- Built the receiver with the Lima PS3DK PPU toolchain and confirmed ELF validation passed.
- Ran the validator against the v01.01 reference (pass) and v0.1.6 legacy ELF (expected failure).
- Ran scripts/build-lima-crypto.sh: crypto/session/SDP/mDNS, RTSP/RTP loopback, capped log, ALAC reference decode, PCM/resampling, diagnostic tone, and PPU object/archive checks passed.
- Package generation, RPCS3 installation/launch, and physical-console checks have not yet been performed for v0.1.7.

## Immediate results

The recovered toolchain now produces an ELF matching the known reference's key loader-facing structure. The specific 0.1.6 loader failure cause remains unresolved; the next package test will evaluate whether the structural correction advances RPCS3 beyond its previous loader boundary.

## Known limitations / follow-ups

- Build and test v0.1.7 from this exact committed revision, finalize/extract it, then install and launch it in RPCS3.
- If RPCS3 reaches app startup, inspect its fresh log and then diagnose receiver socket startup. RPCS3 success still does not prove PS3 output.
- Physical display, START exit, iPhone connection, actual AirPlay audio, and game-audio coexistence are unverified.

## Risks introduced

- The validator intentionally pins assumptions derived from one known-good artifact. A legitimate linker change can fail closed and will require deliberate review/update.
- Matching ELF structure does not prove NPDRM signing, package installation, or runtime compatibility.

## Decision record

### Decision 1: Use PS3DK/Lima as the canonical receiver build path

- **Context:** The v01.01 reference and modern PS3DK output share PS3-specific program-header values; the legacy v0.1.6 ELF differs.
- **Options considered:** Continue the legacy runtime path; patch ELF bytes after linking; use the PS3DK PPU toolchain already staged in Lima.
- **Chosen:** Use the PS3DK/Lima toolchain, with the legacy route retained only for comparison.
- **Tradeoffs:** This prioritizes a matching executable layout, but increases dependence on the pinned PS3DK and Lima environment. It does not prove this was the exact historical v01.01 pipeline.

### Decision 2: Fail before packaging when ELF markers differ

- **Context:** Header-only OSABI correction let v0.1.6 progress to a later but still failing loader stage.
- **Options considered:** Check only OSABI; record warnings; enforce the reference executable markers.
- **Chosen:** Enforce the reference markers in a standalone validator used by both stages.
- **Tradeoffs:** Builds may stop when a valid future toolchain changes layout; that prevents accidental release until the change is reviewed.

### Decision 3: Follow PSL1GHT package finalization and preserve versioned artifacts

- **Context:** The upstream package rule finalizes NPDRM packages, while prior scripts could overwrite artifacts.
- **Options considered:** Keep direct PKG output; finalize a temporary package and rename; overwrite same-version output.
- **Chosen:** Finalize a non-overwriting intermediate and preserve final versioned package names.
- **Tradeoffs:** Failed finalization can leave an intermediate that must be inspected; it is not silently replaced.

## Next recommended step

Commit this implementation after privacy review, rebuild from that exact source revision as AIRPLAY02 v0.1.7, validate its finalized package, and test install/launch in RPCS3 before considering physical-console deployment.
