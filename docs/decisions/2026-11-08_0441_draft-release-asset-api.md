# Fetch draft release assets by authenticated asset ID

**Timestamp:** 2026-11-08 04:41 EST

## Scope of the pass

Changed the manual release workflow to locate the exact draft release by tag, verify the expected asset names and upload states, and retrieve each asset through the authenticated Releases API by asset ID. The existing package hash, evidence, source, and content validation remain in place. Updated memory, lessons, and scratchpad.

## Why this pass was needed

Both observed workflow runs failed at the initial `gh release view "$RELEASE_TAG" --json isDraft` check with “release not found”. Neither run reached `gh release download` or the API asset-ID retrieval. The owner CLI and direct authenticated asset-ID API download work locally, but the exact Actions token behavior remains unproven.

## Files changed

- `.github/workflows/publish-release.yml`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0441_draft-release-asset-api.md`

## Architecture / logic changes

The validation job lists draft releases for the supplied tag through the Releases API, requires exactly one, checks its exact 10-file asset set, checks every asset state and basename, downloads by release asset ID using `Accept: application/octet-stream`, and verifies downloaded sizes before continuing. The publish job also locates the draft by tag through the release list API instead of the tag-specific lookup endpoint. Tag/source SHA checks remain strict. The release source SHA must be an ancestor of main so a follow-up workflow-only fix can be tested without moving the already verified release tag.

## Runtime behavior changes

No application change. The publishing workflow performs an authenticated API-based download of draft assets before the original hash/evidence validation.

## Validation performed

- First Actions run 42468111811 and second run 42468417377: tag/source validation passed; the initial `gh release view` command failed with “release not found”; neither run reached asset retrieval and neither publish job ran.
- Owner CLI `gh release download` retrieved the draft package.
- Authenticated REST download by asset ID retrieved a 3,906,640-byte package matching SHA-256 `b815536168c5c1994ff6a5bb79f4aa16ffed6720691239bd9066e0a9e2075c43`.
- The current uncommitted workflow removes the failing CLI lookup and will be re-run against the same unchanged v1.0.0 tag and package evidence.

## Immediate results

Draft is retained and remains unpublished. Tag v1.0.0 still points to the exact built release source commit. No PS3/RPCS3 action occurred.

## Known limitations / follow-ups

The current workflow has not yet run on GitHub. It must pass API draft enumeration/download, exact set/size checks, SHA256SUMS, release-evidence verification, then publish. If the Actions token cannot read draft assets by API ID, report the observed stage and keep the release as a draft.

## Risks introduced

The workflow now allows the exact tagged release source to be an ancestor of the current main tip rather than requiring current main to equal the tag. This is limited to post-tag follow-up commits; the tag itself must still equal the supplied source SHA, and package evidence must match the tagged source. No release-source rewrite is permitted.

## Decision record

### Decision: Use authenticated release asset IDs for draft downloads

- **Context:** Both failed runs show `gh release view` returned “release not found”; API download by asset ID was verified locally, not in Actions.
- **Options considered:** Publish directly, weaken package checks, or adapt only the download mechanism while preserving the original validation steps.
- **Choice:** Adapt retrieval to authenticated release/asset IDs and keep all checksums, expected files, source identity, and package evidence validation.
- **Tradeoffs accepted:** The workflow has a small Python retrieval block and must permit an unchanged tagged source to remain an ancestor of main after workflow-only fixes. The Actions token's draft visibility remains to be established by the next run.

## Next recommended step

Commit/push this workflow fix, dispatch the manual publisher again with v1.0.0, source SHA `e1fdb44b03e2beb048954aded0eda5ae11248215`, and package hash `b815536168c5c1994ff6a5bb79f4aa16ffed6720691239bd9066e0a9e2075c43`; verify publication and assets.
