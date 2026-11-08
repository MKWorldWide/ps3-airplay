# Remove the failing draft CLI lookup

**Timestamp:** 2026-11-08 04:46 EST (EDT)

## Scope of the pass

Removed a redundant `gh release view` check from the v1.0.0 publication workflow and corrected the recorded diagnosis of two failed runs. The draft will be resolved through the Releases API, followed by exact asset inventory, state, size, checksum, and evidence checks. The draft has not been published.

## Why this pass was needed

Both previous Actions attempts stopped at the first command in the draft-asset step with `release not found`. Reading the grouped run logs showed this was `gh release view`, not the later `gh release download` or REST asset-ID code. Accurate failure attribution matters because neither prior attempt tested the REST retrieval path.

## Files changed

- `.github/workflows/publish-release.yml`
- `docs/decisions/2026-11-08_0441_draft-release-asset-api.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0446_remove_draft_cli_lookup.md`

## Architecture / logic changes

The workflow no longer performs the tag-based `gh release view` assertion. Its Python step locates exactly one matching draft in the Releases API listing, checks the expected 10 assets and upload state, downloads each by API asset ID with the octet-stream header, and validates byte counts. Existing package SHA, manifest, evidence, and exact-source checks remain. The publish job still runs only after validation succeeds.

## Runtime behavior changes

No application/runtime changes. The manual publishing workflow can now advance past the specific command that stopped both earlier runs.

## Validation performed

- Inspected command-grouped failure logs for Actions runs `42468111811` and `42468417377`.
- Confirmed both logs show tag/source validation succeeded and `gh release view` returned `release not found`; neither reached asset retrieval.
- Confirmed the GitHub release remains a draft and the immutable `v1.0.0` tag still points to the release source.
- Local owner-authenticated API retrieval previously returned all 10 expected assets and a PKG matching the recorded hash; this is not equivalent to Actions-token validation.
- Parsed the workflow with PyYAML's `BaseLoader`; asserted the stale CLI precheck is absent.
- `git diff --cached --check` passed; all six staged files and the complete staged diff were reviewed.
- Targeted staged-file scans found no local home paths/usernames, private IPv4/MAC addresses, emails, or raw diagnostic log files. No secret scanner is installed; none was added for this routine check.
- Confirmed local `AGENTS.md`/`agents.md` and private `work/`/`build/` directories are ignored and not staged.

## Immediate results

The exact failing command is now reflected in the workflow documentation. The corrected workflow is uncommitted and untested in Actions. No release publication or console/RPCS3 operation occurred.

## Known limitations / follow-ups

Actions must still demonstrate access to the draft through the Releases API. If that token cannot enumerate or download draft assets, keep the release draft and diagnose the exact API stage. Do not weaken source, artifact, or hash validation.

## Risks introduced

The Releases API query still depends on the GitHub Actions token exposing draft release records and assets. The next run could fail at that API boundary. The release source tag is immutable; follow-up workflow-only commits must remain descendants on `main` without moving the tag.

## Decision record

### Decision: Remove only the proven failing CLI precheck

- **Context:** The same `gh release view` check failed twice before any API-based asset code ran.
- **Options considered:** Keep retrying that command, publish without validation, or remove the redundant check and rely on explicit API inventory plus the existing artifact verification chain.
- **Choice:** Remove the CLI check and make the Releases API lookup the first draft existence check.
- **Tradeoffs accepted:** Draft visibility under the Actions token remains unknown until the next run; publication stays blocked until that run passes.

## Next recommended step

Parse and review the workflow, stage the six exact files, run the compact privacy gate, commit/push to `main` as EsKaye, then dispatch the workflow with tag `v1.0.0`, source SHA `e1fdb44b03e2beb048954aded0eda5ae11248215`, and package SHA-256 `b815536168c5c1994ff6a5bb79f4aa16ffed6720691239bd9066e0a9e2075c43`.
