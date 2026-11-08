# Public stable release roadmap

**Timestamp (Eastern Time):** 2026-11-08 02:40 EST

## Scope of the pass

Replaced the stale mixed-history roadmap with a current release sequence, added a detailed stable-release and feature plan, and reconciled v0.2.8 hardware evidence and version records.

## Why this pass was needed

The old roadmap still presented v0.2.0 as current, called v0.2.8 uninstalled, and mixed historical implementation notes with future milestones. The supplied release plan includes a later v0.2.8 installation and summarized active-stream capture, plus distinct primary/development-unit v0.2.7 results. The docs needed one current status with clear evidence provenance and release blockers.

## Files changed

- `docs/ROADMAP.md`
- `docs/PUBLIC_RELEASE_PLAN.md`
- `README.md`
- `docs/README.md`
- `docs/decisions/README.md`
- `docs/VERSIONING.md`
- `docs/TEST_MATRIX.md`
- `docs/reports/air_ps3_v028_reliability_branding.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0240_public_stable_roadmap.md`

No source, package, console, repository visibility, or GitHub release content changed.

## Architecture / logic changes

None. The roadmap now separates candidate acceptance, a focused stable receiver release, later metadata/controller/display phases, and the final package/publication checklist. It preserves receiver identity and keeps game audio mixing outside the promised product scope.

## Runtime behavior changes

None. This is documentation-only. No PS3 or RPCS3 actions were performed.

## Validation performed

- Read the supplied roadmap material, current memory/scratchpad, roadmap, versioning/release documentation, test matrix, v0.2.8 report, license, branch state, and remote refs.
- Read-only GitHub check confirmed the repository is public, `main` is the default branch, and the release list is empty.
- Used the supplied sanitized v0.2.8 telemetry summary as attributed evidence; no raw log was copied into public documentation.
- Checked the staged document diff, whitespace, ignored build/log/instruction paths, and targeted private-path/network/identifier patterns before commit. No secret scanner is installed; none was added.
- Per the supplied task boundary, changes will be committed locally but not pushed.

## Immediate results

- Roadmap status now reflects the installed v0.2.8 candidate and 10m41s active-stream telemetry without labeling it stable or claiming audible quality.
- The hardware summary separates current development-unit telemetry from earlier v0.2.7 development and primary test-unit observations.
- The stable release gate, version target, feature phases, evidence requirements, and remaining distribution/privacy work are explicit.

## Known limitations / follow-ups

- The 10m41s summary is user-supplied; raw log provenance and exact packet counter reconciliation were not independently re-audited in this documentation pass.
- No v0.2.8 XMB menu/resume data is present in the supplied summary. Human display and audio confirmation, repeat sessions, and a 30-minute run remain pending.
- Artifact/image and dependency distribution rights, plus debug PKG suitability/documentation, remain to be reviewed before a downloadable release.

## Risks introduced

- Aggregated hardware metrics are public and may reveal operational behavior, though no raw logs, local paths, device addresses, unique identifiers, or personal computer details were added.
- Readers could overinterpret counters; each table and report distinguishes receiver telemetry from human-observed audio/display results.

## Decision record

### Decision

Use v0.2.8 as the current installed hardware candidate, not as a stable release; keep the stable milestone focused on receiver reliability and defer feature phases.

### Context

The candidate has a validated package and a summarized 10m41s stream, but no captured XMB resume, 30-minute duration, or explicit listening assessment in that supplied evidence. Earlier unit histories differ.

### Options considered

- Keep the old roadmap and append new release notes.
- Replace stale roadmap claims and add one canonical public release plan with source-attributed evidence.
- Treat the current counters as sufficient for stable release readiness.

### Why this option was chosen

Replacing stale claims avoids contradictory milestone status; a detailed plan records acceptance and future features without implying that logs prove perceived quality or display behavior.

### Tradeoffs accepted

The current plan retains only concise telemetry summaries in public docs. Exact raw traces and private deployment evidence remain local and ignored.

## Next recommended step

When hardware validation is in scope, run the Gate 0 display/XMB, clean-exit, iPhone session, listening, reconnect, and 30-minute tests on the development unit; keep primary-unit testing separate.
