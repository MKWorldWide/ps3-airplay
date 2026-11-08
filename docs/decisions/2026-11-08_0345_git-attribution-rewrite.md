# Correct published commit attribution to EsKaye

**Timestamp:** 2026-11-08 03:45 EST

## Scope of the pass

Corrected Git author/committer configuration and rewrote every remote branch and tag so prior user-authored commits resolve to the authenticated EsKaye GitHub account. Verified tree equivalence and GitHub attribution. The v1.0.0 package remains installed on the development unit and was not launched.

## Why this pass was needed

Recent commits were still displayed under `sunnykaye` because their no-reply email mapped to a different GitHub account. The user explicitly requested that all commits appear under EsKaye.

## Files changed

- `docs/RELEASE_GATE_1.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/git_author_attribution_migration.md`
- `docs/decisions/2026-11-08_0345_git-attribution-rewrite.md`

Git author settings changed globally and in this repository. The pre-rewrite recovery bundle and reference manifest are local ignored files.

## Architecture / logic changes

No application code or source trees changed. Commit metadata for the prior user-authored history was mapped to the authenticated GitHub account's no-reply address. All branch and tag refs were moved to the corresponding rewritten commits.

## Runtime behavior changes

None. The development PS3 retains the installed AIRPLAY02 v1.0.0 package; it was not launched. The primary PS3 was untouched.

## Validation performed

- Confirmed authenticated GitHub account login and corrected global/repository Git identities.
- Reviewed public repository state: zero forks, zero open PRs, no main branch protection; recorded four branches and three tags.
- Saved and verified a complete pre-rewrite bundle and exact remote-ref manifest.
- Compared all 113 commit trees before and after; inventory matched exactly.
- Dry-ran an atomic force-with-lease update for all seven remote branch/tag refs against saved old SHAs; then pushed the same exact ref updates atomically.
- Verified every pushed branch/tag matches local refs; `git fsck --full --no-reflogs` had no errors.
- Checked GitHub REST API author login for all four branch tips; each reports `EsKaye`.
- Confirmed no prior-account email remains reachable from active local branch/tag refs.

## Immediate results

All active GitHub branch and tag histories now attribute the user's commits to `EsKaye`; rollback tag names were preserved. One project-owner commit remains authored by `MKWorldWide` as intended. No source tree changed.

## Known limitations / follow-ups

Existing clones, remote caches, or previously copied commit links may still show old hashes and must fetch/reclone to adopt new refs. This cannot be verified for machines outside the repository. The v1.0.0 build evidence cites its pre-rewrite source SHA; rebuild from the current main tip before release.

## Risks introduced

All rewritten commit hashes differ. Downstream clones that push old refs may require repair. A verified local bundle and original-ref manifest are retained locally for recovery. No open PRs or forks were found before updating the remote.

## Decision record

### Decision: Rewrite published branch and tag history

- **Context:** The user asked that all project commits show as EsKaye; repository inspection confirmed that user-authored commits across main and development branches were linked to the former account identity.
- **Options considered:** Correct future Git settings only; rewrite only the active branch; or rewrite all published branches and tags.
- **Choice:** Rewrite all branches and tags after a bundle backup, ref inventory, clone/fork/PR assessment, exact-lease dry run, and tree-equivalence check.
- **Tradeoffs accepted:** Commit hashes and old commit URLs changed. Existing clones/caches may remain stale; backup recovery is available locally.

## Next recommended step

Commit this report using the corrected identity, verify its GitHub author login, then rebuild the release candidate from exact current `main` before any release staging.
