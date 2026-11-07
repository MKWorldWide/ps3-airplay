# v0.1.21 package validation and display follow-up

**Timestamp (EDT):** 2026-11-07 11:10

## Scope of the pass

Built and validated AIRPLAY02 v0.1.21 from the already-pushed `main` revision, preserved its PKG/ELF/build metadata in the ignored local artifact directory, and updated package/test/roadmap/context records. Rechecked the requested image-only renderer cleanup against source history.

## Why this pass was needed

The RAOP TXT server version fix had passed host tests and PPU component compilation, but lacked full-app and finalized-package evidence. The user also requested removal of display decorations after reporting that the app opens.

## Files changed

- `docs/VERSIONING.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_1110_v0-1-21-package-validation.md`

The image-only renderer change was already committed and pushed in `4a06c2a` (`Remove standalone diagnostic text overlay`); this pass did not alter display source.

## Architecture / logic changes

No runtime logic changed. The package builds the existing standalone receiver with RAOP `_raop._tcp` TXT `vs=105.1`, consistent with its RTSP `AirTunes/105.1` server header. The screen renderer copies the supplied background without diagnostic borders or text overlays.

## Runtime behavior changes

The candidate advertises corrected RAOP server-version metadata. Runtime behavior has not yet been verified in RPCS3 or on hardware. The user's latest visible-app observation is not matched to a known package version/platform.

## Validation performed

- `limactl shell s84d-vdfs -- bash -lc '... bash scripts/package-lima-standalone-receiver.sh'`
- `scripts/validate-ps3-elf.py`: PASS, CellOS PPC64 executable, five PT_LOAD segments.
- `scripts/validate-ps3-pkg.py`: PASS, 3,230,384-byte finalized NPDRM package, five items.
- Package extraction and SFO checks: AIRPLAY02, APP_VER `0.1.21`, content ID, EBOOT, icon, background asset, relative paths, and finalized envelope PASS.
- Earlier in this source pass: full Lima host suite with ASan/UBSan, RAOP TXT serializer check, and PPU component checks PASS.
- `git` ancestry confirms the image-only cleanup is already included in `main`.
- Computer Use check: Mac remains locked; RPCS3 install/launch could not be inspected.

## Immediate results

- ELF: 2,780,120 bytes; SHA-256 `401225f2c8b704103c5f65683a5b48f9c7b34633ae57145ca92c190485543e1e`.
- PKG: 3,230,384 bytes; SHA-256 `c8c77b78d34dc50a955383ddafe7c835a7848c62eb8a9673ede7ed0c2776ef73`.
- Artifacts are preserved in ignored `work/builds/0.1.21/`.
- App launch, display, clean exit, iPhone RTSP, RTP, decoding, and audible AirPlay remain unverified for this package.

## Known limitations / follow-ups

- Install and run this exact package in RPCS3 when desktop access is available; capture a fresh version-matched log.
- If the image-only renderer appears with overlays in the user's observed copy, first identify the installed package version and platform.
- Continue from the earliest failed receiver boundary; do not claim successful iPhone playback from host tests.

## Risks introduced

- No new runtime risk; package contents correspond to the existing source revision. Host validation does not establish emulator or PS3 behavior.

## Decision record (embedded)

### Preserve the existing image-only screen code

- **Decision:** Do not make another screen edit or duplicate package solely from the unversioned visual report.
- **Context:** The current renderer draws only the background; the cleanup is already in pushed history. RPCS3 desktop inspection is unavailable because the Mac is locked.
- **Options considered:** Make an additional speculative UI edit; rebuild an identical package; or retain source and resolve the mismatch against an identified runtime build.
- **Why chosen:** The requested elements are absent in current source, and package/platform identity is required to distinguish an old artifact from current behavior.
- **Tradeoffs accepted:** The user's exact observed screen remains unresolved until the desktop is accessible and the tested APP_VER is known.

### Preserve the validated v0.1.21 candidate

- **Decision:** Keep the versioned local PKG/ELF and record hashes; do not install it without runtime access.
- **Context:** Package validation is complete, but GUI launch evidence cannot be collected in this turn.
- **Options considered:** Treat build success as runtime success; discard the artifact; or preserve it pending emulator validation.
- **Why chosen:** Preserving exact bytes makes the next test repeatable and protects provenance.
- **Tradeoffs accepted:** Runtime confidence remains pending.

## Next recommended step

Unlock the Mac, install/run the exact v0.1.21 package in RPCS3, and capture its fresh capped log before beginning a controlled iPhone connection attempt.
