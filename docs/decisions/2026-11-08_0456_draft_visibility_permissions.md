# Grant validation access to draft release metadata

**Timestamp:** 2026-11-08 04:56 EDT

## Scope of the pass

Raised the release-validation job's `contents` permission from read to write so the Actions token can enumerate the unpublished v1.0.0 draft. Updated project memory, lessons, and active scratchpad with the third run's exact result and the remaining validation boundary.

## Why this pass was needed

After removal of the failing CLI lookup, run `42468710312` completed tag/source checks but the Releases API returned zero matching drafts. GitHub documents that only actors with repository push access receive draft-release listings ([REST API: List releases](https://docs.github.com/en/rest/releases/releases#list-releases)). The validation job had only `contents: read`.

## Files changed

- `.github/workflows/publish-release.yml`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0456_draft_visibility_permissions.md`

## Architecture / logic changes

The validation job receives `contents: write` and uses its token only in GET requests that enumerate the matching draft, list assets, and download by asset ID. The `publish` job remains a distinct job that patches the draft to published after validation succeeds. Tag, source, asset inventory, byte counts, checksums, evidence, and package SHA checks are unchanged.

## Runtime behavior changes

The validator should be able to see the draft record and proceed to asset checks. The job token now has repository write capability even though the current validation script performs read-only requests.

## Validation performed

- Run `42468710312` confirmed tag/source validation passed, then failed because the Releases API query found no matching draft with `contents: read`.
- Reviewed the full diff from the preceding pass and the failed-run stage. This permission change has not yet been pushed or exercised in Actions.
- Workflow YAML parse, whitespace checks, staged diff review, and privacy scan will be repeated before committing.

## Immediate results

The permission change is local and uncommitted. The GitHub release remains a draft; no release or console state changed.

## Known limitations / follow-ups

The next run must confirm the draft becomes visible, then list and download all ten expected assets. If asset access still fails, stop and report the exact stage; do not remove artifact checks or publish by bypassing validation.

## Risks introduced

The validation job's token can write repository contents. The current job uses it only for read-only API calls, but a compromised or modified workflow step could exercise the broader permission. The permission is limited to this job and is needed to access draft release records; review whether a narrower GitHub permission becomes available.

## Decision record

### Decision: Grant write scope to the validator

- **Context:** GitHub draft listings are limited to actors with push access, while the read-scoped Actions token returned an empty list.
- **Options considered:** Leave publication blocked, introduce a long-lived personal token secret, publish before validation, or grant the short-lived workflow token the permission needed for draft inspection.
- **Choice:** Grant `contents: write` to the isolated validation job; keep all checks and the separate publish job.
- **Tradeoffs accepted:** Validation's token has more capability than the API calls require. No long-lived user credential is stored in repository secrets, and publication remains gated on the complete validator succeeding.

## Next recommended step

Validate and privacy-scan this exact change, commit/push as EsKaye, rerun the manual v1.0.0 publication workflow, and verify that draft enumeration, asset retrieval, package checks, and final publication all pass.
