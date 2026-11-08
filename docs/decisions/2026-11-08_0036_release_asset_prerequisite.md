# Make the release gate generate its background asset

**Timestamp:** 2026-11-08 00:36 EST

## Scope of the pass

Added the missing JPEG-to-RGB image generation step to the combined Lima release gate and documented the Pillow dependency. Installed Ubuntu `python3-pil` in the existing Lima VM so the pinned build environment can execute the step.

## Why this pass was needed

The first clean-source build from the relocated checkout passed the host/sanitizer tests, PPU object compilation, and standalone ELF validation, then failed at packaging because `build/diagnostic-background.rgb` was absent. The old worktree had retained ignored generated assets, masking the missing orchestrator step.

## Files changed

- `scripts/release-lima-standalone-receiver.sh`
- `docs/BUILD_PIPELINE.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0036_release_asset_prerequisite.md`

## Architecture / logic changes

After the standalone receiver build succeeds, the release wrapper now invokes `prepare-diagnostic-background.py` with the checked-in JPEG and the build-directory RGB destination. The package stage therefore consumes a deterministic generated input from the same source tree. Pillow is an explicit host dependency (`python3-pil` on Ubuntu).

## Runtime behavior changes

None. Application code, package version, and SFO version are unchanged.

## Validation performed

- `bash scripts/release-lima-standalone-receiver.sh` against clean exported revision `002e6cf`: all host/sanitizer tests passed; PPU objects and ELF validation passed; the run stopped at the previously missing background prerequisite.
- Checked the generated-asset converter uses Pillow and confirmed the Lima guest lacked it.
- Installed Ubuntu `python3-pil` 10.2.0-1ubuntu1.4; guest reports 11 GiB free after installation.
- Full release rerun with this source fix is pending.

## Immediate results

The failure boundary is explained and the two concrete prerequisites are addressed in source and guest setup. The package has not yet been rebuilt from the new commit.

## Known limitations / follow-ups

The build still needs a complete rerun from the corrected committed revision, then package comparison and full migration acceptance. This does not establish PS3 launch, XMB display restoration, or audible playback.

## Risks introduced

Only a build-time image dependency was added to the release sequence. It depends on Pillow and can fail clearly if that dependency is absent. No console files, system settings, or runtime behavior were modified.

## Decision record

### Decision

Generate the RGB image within the combined release gate instead of relying on an ignored artifact from a prior build.

### Context

A clean relocated source export exposed the missing build input. A pre-existing ignored RGB asset made previous builds appear self-contained.

### Options considered

1. Keep reusing a copied, ignored RGB file.
2. Add the existing conversion script to the release gate and declare Pillow.
3. Check in the 2.7 MiB generated RGB file.

### Why this option was chosen

Option 2 creates the asset deterministically from the checked-in JPEG and keeps generated output out of Git. The required Python package is small and available from Ubuntu's package repository.

### Tradeoffs accepted

The build host must provide Pillow. This is preferable to retaining generated binaries in the repository or relying on stale local output.

## Next recommended step

Commit this focused build fix after the routine staged privacy scan, push `main`, export that exact commit into Lima, and rerun the complete release gate with package output isolated from the existing v0.2.7 reference.
