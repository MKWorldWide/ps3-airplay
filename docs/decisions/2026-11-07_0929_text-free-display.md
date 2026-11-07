# Remove Diagnostic Text from the Standalone Display

Timestamp: 2026-11-07 09:29 America/New_York (EST)

## Scope of the pass

Removed the standalone screen's remaining text overlay and updated project state, test tracking, lessons learned, and the v0.1.16 package decision log.

## Why this pass was needed

The user confirmed that seeing the background image is sufficient and requested removal of the blue framing and `DISPLAY ACTIVE` text. The source renderer did not contain those two elements; it did contain a `PRESS START TO EXIT` prompt that was also unnecessary for the requested clean image.

## Files changed

- `prototype/standalone-receiver/source/diagnostic_screen.c`
- `docs/BUILD_PIPELINE.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_0925_v016-package-build.md`
- `docs/decisions/2026-11-07_0929_text-free-display.md`

## Architecture / logic changes

Deleted the glyph bitmap, text rasterizer, overlay drawing function, and its call. Framebuffer initialization, image loading/scaling, scanout, and controller exit handling are otherwise unchanged.

## Runtime behavior changes

When a future build launches, the application should display the configured background image without diagnostic text. This source change has not yet been packaged or run.

## Validation performed

- `rg` inspection confirmed there was no `DISPLAY ACTIVE` string or blue-border drawing code in the renderer before the change; the remaining overlay was `PRESS START TO EXIT`.
- Source/diff checks are pending before commit.
- v0.1.16 package validation remains as recorded in the preceding decision log; it does not validate this follow-up.

## Immediate results

Source now contains no display text-drawing functions. Visible output on v0.1.17 remains unverified.

## Known limitations / follow-ups

- Build/package as v0.1.17, run package validation, then visually verify in RPCS3.
- Confirm controller START still exits cleanly because controller handling is independent of the removed prompt.
- Do not infer physical PS3 display, clean exit, AirPlay, or audible playback success.

## Risks introduced

No lifecycle or framebuffer behavior was changed. The screen no longer displays its own exit instruction; controller START remains the exit mechanism and should be tested on the next package.

## Decision record

### Decision 1: Remove all diagnostic text from scanout

- **Context:** The user reported visible extra framing and `DISPLAY ACTIVE` labeling and stated that the image alone suffices.
- **Options considered:** Search for the named visuals and leave the other prompt; remove only the named overlays; remove the complete diagnostic text overlay while retaining controller behavior.
- **Why chosen:** The named elements were absent from the current renderer, while the remaining prompt was part of the same diagnostic overlay and no longer needed once the image is visible.
- **Tradeoffs accepted:** The user will not see an on-screen START prompt; the controller exit path is unchanged and requires retest.

## Next recommended step

Commit and push this source/doc pass, then build and validate v0.1.17 from the pushed revision and run it in RPCS3 when UI access is available.
