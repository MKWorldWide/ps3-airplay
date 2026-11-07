# Preserve recovered launch baseline and correct release identity

## Timestamp in EDT
2026-11-07 1404 source-local America/New_York

## Scope of the pass
Change filename/log label from 0.30 to 0.1.30; retain SFO 00.30 and recovered runtime code. Record exact hardware evidence and the OPTIONS stall.

## Why this pass was needed
Maintainer corrected release naming and reported first startup/tone/idle exit success plus silent sender attempt and controller freeze.

## Files changed
prototype/standalone-receiver/Makefile; scripts/build-lima-standalone-receiver.sh; scripts/package-lima-standalone-receiver.sh; docs/memory.md; docs/scratchpad.md; docs/lessons-learned.md; docs/TEST_MATRIX.md; docs/BUILD_PIPELINE.md; docs/VERSIONING.md; docs/ROADMAP.md; docs/RISKS.md; docs/reports/2026-11-07_v030_hardware_launch_and_options_stall.md; this decision log.

## Architecture / logic changes
Keep release identifier separate from SFO platform version. No receiver/runtime changes.

## Runtime behavior changes
Startup version marker becomes 0.1.30. Historical 0.30 log/artifact remains preserved.

## Validation performed
Retrieved original hardware log read-only; allowlisted events and SHA-256 preserved. Installed payload previously matched source package. Shell syntax and corrected package extraction/version validation follow this commit's clean build.

## Immediate results
Hardware startup/tone/idle START success confirmed by user. Logged TCP and basic OPTIONS succeed; final OPTIONS stalls. No complete AirPlay session or audio playback proven.

## Known limitations / follow-ups
Synchronous challenge/RSA is a hypothesis; last request headers were deliberately not logged. Need targeted phase diagnostics and responsive exit investigation in the next version.

## Risks introduced
No runtime change. Reinstalling/relaunching can replace current title/truncate its diagnostic log; original evidence is preserved locally first.

## Decision record (embedded)
- Decision: preserve working runtime while correcting label. Context: explicit requested 0.1.30 / 00.30 pair. Options: relabel or bundle unproven protocol changes. Chosen: relabel. Tradeoff: known OPTIONS stall remains.
- Decision: document SFO launch defect and recovered startup with co-change caveat. Context: SFO format and container changed together. Options: claim isolated cause or preserve evidence limits. Chosen: record defect and observed recovery accurately. Tradeoff: causal isolation remains incomplete.
- Decision: use last reliable log stage. Context: no USER_EXIT_START despite user controller attempts. Options: assume cleanup hang or inspect unanswered OPTIONS. Chosen: unanswered request. Tradeoff: exact blocked operation still unknown.

## Next recommended step
Build and verify requested release identity; then instrument OPTIONS challenge and RNG phases in the next runtime version before repeating risky sender tests.
