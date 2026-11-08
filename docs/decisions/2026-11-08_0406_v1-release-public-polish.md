# Polish public v1.0.0 release path and record latest hardware report

**Timestamp:** 2026-11-08 04:06 EST

## Scope of the pass

Updated the repository landing page, installation/recovery guide, documentation index, v1.0.0 notes, roadmap context, test matrix, release gate, and project memory to make the existing AIR PS3 v1.0.0 easier to download, install, understand, and exit. Recorded the maintainer's new qualitative hardware report. No new application feature was started.

## Why this pass was needed

The public repository still read as experimental and unpublished, lacked an obvious direct release path, and did not reflect the latest user-reported v1.0.0 observation. Installation instructions needed to make the normal START exit control unmistakable.

## Files changed

- `README.md`
- `docs/INSTALLATION.md`
- `docs/PUBLIC_RELEASE_PLAN.md`
- `docs/ROADMAP.md`
- `docs/README.md`
- `docs/RELEASE_GATE_1.md`
- `docs/RELEASE_NOTES-v1.0.0.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/air_ps3_v1_0_0_user_report.md`
- `docs/decisions/2026-11-08_0406_v1-release-public-polish.md`

## Architecture / logic changes

No application or build logic changed. Public copy now clearly describes an iPhone-sent AirPlay 1 receiver, the foreground/debug-PKG scope, the release download/checksum links, install/remove/recovery path, and separation from Spotify Connect and resident/game audio.

## Runtime behavior changes

None. The user reports v1.0.0 is running beautifully on the development PS3. The report supplied no run duration, fresh log, or specific playback/control checks, so none are inferred.

## Validation performed

- Inspected current Git branches, tags, and feature/main relationship; the reviewed release branch is a fast-forward descendant of current main.
- Inspected live GitHub visibility, releases, workflow file state, and all six open issues.
- Confirmed source handles controller START as the application exit and prior hardware evidence records START returning to XMB while idle.
- Reviewed the live release workflow's required asset names and source/package/evidence checks.
- Documentation and public issue changes are pending final privacy scan, host/build validation, and release gate completion.

## Immediate results

- README now links directly to v1.0.0, the PKG, and checksum asset.
- Installation guidance prominently states: “Press START to exit AIR PS3 and return to the XMB.”
- Current v1.0.0 user feedback is recorded as a qualitative observation with its evidence boundary.
- No console was installed, launched, or otherwise changed in this pass.

## Known limitations / follow-ups

The release is not published yet. The exact-main-tip Lima build, final package/assets privacy review, and workflow publication remain pending. The current user report does not establish a soak duration, actual AirPlay stream stages, or controls for this run.

## Risks introduced

Direct release links will be live only after the v1.0.0 release is published. The status will be rechecked before publishing.

## Decision record

### Decision: Record the current hardware statement without extrapolation

- **Context:** The maintainer says v1.0.0 is running beautifully but provided no run duration, log, or specific test list.
- **Options considered:** Treat the statement as a full acceptance pass or record it as a qualitative user observation.
- **Choice:** Record the wording as a qualitative observation; retain the v0.2.8 45-minute summary as separate historical evidence.
- **Tradeoffs accepted:** The release materials do not claim an unreported v1.0.0 soak, log result, or controller test.

### Decision: Make START exit the primary user instruction

- **Context:** Source handles controller START as an application exit; historical hardware evidence records return to XMB when idle.
- **Options considered:** Mention START only in troubleshooting or make it prominent in quick-start and installation guidance.
- **Choice:** State the START-to-XMB instruction explicitly in both places and keep recovery guidance next to it.
- **Tradeoffs accepted:** The wording is supported by source and prior hardware evidence; it does not claim a fresh v1.0.0 START retest.

## Next recommended step

Complete the remaining README/roadmap and issue reconciliation, fast-forward the reviewed work into main, then build and verify the exact main tip in Lima.
