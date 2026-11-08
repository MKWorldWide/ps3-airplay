# Restore the release-gate helper status contract

**Timestamp:** 2026-11-08 04:31 EST

## Scope of the pass

Corrected the release-gate heading to match the staging helper, clarified that package identity and hashes are authoritative in the release evidence assets, and labelled the earlier exact-main run as a checkpoint. Updated memory, lessons, and scratchpad.

## Why this pass was needed

The release staging helper rejected an otherwise passing gate because the heading text had been changed from its exact machine-readable marker.

## Files changed

- `docs/RELEASE_GATE_1.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0431_release-gate-helper-contract.md`

## Architecture / logic changes

No application or build logic changed. The gate heading again matches the exact staging-helper contract. The report distinguishes the prior checkpoint package from the artifact generated from the eventual release source SHA.

## Runtime behavior changes

None.

## Validation performed

- Ran `scripts/stage-github-release.sh` against the verified package; it failed closed because the gate heading did not match the required marker and created no tag or draft.
- Reviewed the helper's exact status and pending/blocked row checks.
- Confirmed source/status changes require a fresh exact-source build before staging.

## Immediate results

- Gate status heading restored to `**Release gate status: PASS**`.
- The previous candidate remains local and ignored; it will not be staged because the source SHA will change.
- No GitHub tag or draft was created by the failed attempt.

## Known limitations / follow-ups

Commit and push this docs correction, then repeat the entire pinned Lima build from the new exact `main` SHA. Recheck release assets and stage only after the new evidence matches that SHA.

## Risks introduced

None to runtime or console state. A documentation-only commit changes the release source SHA and invalidates the earlier candidate evidence for release staging.

## Decision record

### Decision: Preserve the helper contract rather than weakening the release check

- **Context:** The helper requires a specific heading to fail closed before creating a release tag/draft.
- **Options considered:** Loosen the helper regex or restore the documented exact heading.
- **Choice:** Restore the exact heading and keep explanatory text elsewhere.
- **Tradeoffs accepted:** Minor wording flexibility is reduced; release staging remains simple and deterministic.

## Next recommended step

Commit and push this correction, rebuild v1.0.0 from the resulting main SHA in Lima, verify exact assets, then stage and publish through the manual workflow.
