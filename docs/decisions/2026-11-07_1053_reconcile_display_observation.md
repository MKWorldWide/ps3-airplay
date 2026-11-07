# Reconcile visible display report with cleaned source

## Timestamp in EDT

2026-11-07 10:53 EDT

## Scope of the pass

Clarified the standalone README to state that the screen renders only the configured background and that controller START exits without an on-screen prompt. Updated the hardware display report, test matrix, roadmap, project memory, lessons learned, and scratchpad to preserve the new user observation and the source/artifact mismatch.

## Why this pass was needed

The user reports that the app opens and shows the image, but still sees blue framing and a `DISPLAY ACTIVE` label. Current source has no such renderer, so the observation cannot be resolved without identifying the actual package and platform.

## Files changed

- `prototype/standalone-receiver/README.md`
- `docs/reports/2026-11-07_display-followup.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_1053_reconcile_display_observation.md`

## Architecture / logic changes

No runtime changes. The renderer already copies the configured image to the framebuffers without drawing any overlay. Both display-cleanup commits (`46fca40` and `4a06c2a`) are ancestors of the v0.1.20 source commit. README now matches this behavior.

## Runtime behavior changes

None. No package was rebuilt or installed.

## Validation performed

- Inspected `diagnostic_screen.c` and confirmed it only copies the RGB image into the two framebuffers.
- Verified both cleanup commits are ancestors of the v0.1.20 source revision.
- Confirmed the v0.1.20 package path/hash against the existing version record.
- Checked Computer Use twice after the user offered to unlock the desktop; both checks still reported the Mac locked.
- No new tests or package build were needed for this documentation-only pass.

## Immediate results

The current source and v0.1.20 package lineage do not contain the reported blue frame or `DISPLAY ACTIVE` overlay. The user's visible-app observation is retained, but exact runtime package identity remains unknown.

## Known limitations / follow-ups

Install and launch a version-matched package in RPCS3 once UI access is available. Compare displayed APP_VER/startup log with the visible screen. If the cleaned package still shows overlays, inspect its extracted EBOOT and runtime image path before code changes.

## Risks introduced

No runtime risk. Documentation explicitly preserves uncertainty instead of attributing the observation to an unverified artifact.

## Decision record (embedded)

### Decision: Do not duplicate a display cleanup already present in the package lineage

- **Context:** The user-visible screen appears inconsistent with current source, but the exact package and platform are unknown.
- **Options considered:** Remove more renderer code; rebuild the same behavior under a new package version; first identify the displayed artifact.
- **Why this option was chosen:** Both requested overlays are already absent in current source and v0.1.20 ancestry. A fresh version-matched run is needed to locate the discrepancy.
- **Tradeoffs accepted:** Visual verification remains pending while the desktop UI reports locked.

## Next recommended step

Use the accessible RPCS3 UI to install and run the exact v0.1.20 package, collect its fresh version marker, and inspect the actual displayed image and START-to-exit behavior.
