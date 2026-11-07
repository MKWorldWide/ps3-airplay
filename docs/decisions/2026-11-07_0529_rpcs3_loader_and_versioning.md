# RPCS3 loader validation and release versioning

## Timestamp in EDT

2026-11-07 05:29 EST (America/New_York; filename follows the requested EDT-style timestamp format)

## Scope of the pass

Correct package entry paths, test v0.1.4 in RPCS3, generate and test v0.1.5 and v0.1.6 candidates, add guarded CellOS LV2 OS/ABI patching, record loader evidence, and document the project's release version milestones.

## Why this pass was needed

RPCS3 had rejected the v0.1.4 package before application startup because package entries used absolute paths. After path correction it installed but rejected EBOOT. The next candidate exposed a generic System V OS/ABI. Version history also needed a clear rule for small fixes versus stable feature milestones.

## Files changed

- `CONTRIBUTING.md`
- `README.md`
- `docs/VERSIONING.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_rpcs3_loader_validation.md`
- `docs/decisions/2026-11-07_0529_rpcs3_loader_and_versioning.md`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-legacy-runtime.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `scripts/patch-ps3-elf.py`

## Architecture / logic changes

The package script now stages files under the content ID, defaults to the PS3DK NPDRM signer, validates extraction and metadata, and rejects absolute entry paths. The legacy-runtime build invokes a small post-link ELF patcher. It accepts only ELF64, big-endian PowerPC64 binaries with an expected OS/ABI value, then sets `EI_OSABI` to CellOS LV2 `0x66` before SELF and PKG creation. Version defaults now align on 0.1.6.

The project policy reserves `0.1.x` for focused changes, `0.2.0` for a stable standalone diagnostic app with settings and diagnostic toggles, and `0.3.0` for stable AirPlay features validated on hardware. Failed packages consume version numbers; SFO, filename, and startup marker stay aligned.

## Runtime behavior changes

The package contents now use relative paths and v0.1.6's linked ELF advertises the PS3 LV2 OS/ABI. These changes affect boot metadata only; receiver behavior did not start in RPCS3 during this pass.

## Validation performed

- Built receiver PPU ELF and package in Lima for v0.1.5 and v0.1.6.
- Inspected PPU ELF architecture, OS/ABI, and section table.
- Extracted each package and checked SFO title/version, content ID, EBOOT, icon, and RGB asset.
- Checked package listing for relative entries.
- Installed v0.1.4 corrected-path package, v0.1.5, and v0.1.6 in RPCS3 using its graphical package installer; attempted one boot per candidate.
- Reviewed RPCS3 log output after each boot.
- Did not run the portable host regression suite in this pass.

## Immediate results

- v0.1.4 corrected-path package installed, but RPCS3 rejected its EBOOT before application startup.
- v0.1.5 (3,183,600 bytes, SHA-256 `dfe2411ce0079d09fd84e7169865fad299dcb5f2759c2b8e0876fb2b7e5a8449`) installed, but RPCS3 reported invalid ELF OS/ABI.
- v0.1.6 (3,183,600 bytes, SHA-256 `c71cf7ee0ec314496971ea32d831c05238746eada6796b017092b3873a2ecc33`) installed and had `EI_OSABI=0x66`; RPCS3 then reported invalid ELF type.
- No in-app logger started; no display, tone, AirPlay connection, RTP, decode, or playback result was produced.

## Known limitations / follow-ups

The build used the available PSL1GHT checkout at commit `dd4e8c9be744e3273d665c393fa6d97a87402e2e`, not the pinned 2020 commit expected by the build script. The OS/ABI patch is partial. The ELF `e_type` and SELF inner payload still need comparison with an artifact proven to boot in RPCS3. Hardware installation remains out of scope until RPCS3 starts the app and RSX/controller exit risk is reviewed.

## Risks introduced

The patcher modifies one ELF identity byte and fails closed on unexpected architecture or OS/ABI, but this metadata change cannot compensate for missing PS3 loader conventions. Do not infer real-console compatibility from the RPCS3 package install result. The guest toolchain revision mismatch may affect reproducibility.

## Decision record (embedded)

### Decision: Keep corrected package paths and fail on absolute entries

- **Context:** The original RPCS3 install path rejected the v0.1.4 archive before reading EBOOT.
- **Options considered:** Keep the legacy staging layout; create a content-ID staging root and validate the package listing.
- **Why chosen:** Content-ID-rooted relative paths pass package installation and prevent repeating the known container defect.
- **Tradeoffs accepted:** Package installability remains separate from executable validity and app startup.

### Decision: Patch CellOS LV2 OS/ABI after link

- **Context:** RPCS3 v0.1.5 reported generic System V OS/ABI as invalid.
- **Options considered:** Change toolchain immediately; apply a guarded, documented ABI-byte patch; rebuild with a different toolchain.
- **Why chosen:** The binary already contained PS3-specific stubs/parameter sections, and the targeted patch is minimal and validated before signing.
- **Tradeoffs accepted:** v0.1.6 still fails the next ELF loader check; this is recorded as partial progress, not a successful fix.

### Decision: Reserve 0.2.0 and 0.3.0 for capability milestones

- **Context:** Frequent package iterations need distinct identifiers without inflating milestone numbers.
- **Options considered:** Increment minor versions for each build; use 0.1.x patches and reserve minor milestones for stable capabilities.
- **Why chosen:** This preserves a clear history and matches the maintainer's requested targets.
- **Tradeoffs accepted:** More patch releases accumulate before 0.2.0; every failed package still consumes its version.

## Next recommended step

Inspect a known RPCS3-booting SELF's inner ELF `e_type`, flags, headers, and post-link tools. Make the smallest required fix and assign v0.1.7 before building it.
