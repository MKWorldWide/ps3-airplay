# GitHub author attribution migration

**Date:** 2026-11-08
**Repository:** `MKWorldWide/ps3-airplay`

## Cause and scope

GitHub attributed user-authored commits to `sunnykaye` because the configured no-reply email identified a different GitHub account from the authenticated `EsKaye` profile. Global and repository-local Git settings now use the authenticated profile's no-reply identity.

At the user's request, history was corrected across every published branch and tag. The migration changed author/committer identity fields for the user's prior commits and preserved the separate `MKWorldWide`-authored project commit.

## Recovery and impact review

- Before mutation, saved and verified a complete local Git bundle and a manifest of every remote branch/tag SHA.
- GitHub reported zero forks and zero open pull requests. The default branch was `main`; no branch protection was enabled.
- Four branches and three tags were updated atomically using exact old-SHA leases. The existing tag names `safe-v0.2.4`, `baseline-v0.2.6`, and `baseline-v0.2.7` remain.
- A local recovery bundle remains in ignored work data. It contains the original refs if rollback is needed.
- Existing clones, downloaded archives, and cached GitHub commit pages cannot be updated by this operation; collaborators with clones may need to fetch the rewritten refs or reclone. No forks were listed at migration time.

## Integrity checks

- All 113 reachable commit trees across branch and tag refs match the pre-rewrite tree inventory.
- No prior user-account email remains reachable from active local branches, remote-tracking branches, or tags.
- `git fsck --full --no-reflogs` reported no errors.
- The remote branch and tag tips match local refs after push.
- GitHub REST API reports `EsKaye` for the tips of `main` and all three development branches.
- Git working tree remained clean before writing this report.

## Updated remote tips

These are the branch/tag targets immediately after the metadata rewrite; the current feature branch advances when this report is committed.

| Ref | New tip |
|---|---|
| `main` | `853a582c73820ea8ff291dae9cfa2510ae25b54f` |
| `lily/air-ps3-v028` | `277676dfed201903517c02b5b7999b4558f64f8b` |
| `lily/fidelity-v027` | `0a62714508abdaad00d18b9e9e933a4b037a9990` |
| `lily/sol61-forensic-audit` | `ab504b763ee8efe5110991fe8799b7740beba2da` |

## Release artifact note

The installed v1.0.0 package was built before the identity rewrite from commit `f3da352809f81daac5e3b61a35d7fc18362bf1cc`. Its source tree is identical to rewritten commit `a1edbba176f31076eccd10d2c627916963cb5605`, but its embedded release evidence still cites the former SHA. Rebuild from the exact current `main` tip before staging any release.
