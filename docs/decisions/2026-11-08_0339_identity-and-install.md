# Correct GitHub commit attribution and install v1.0.0 on the development unit

**Timestamp:** 2026-11-08 03:39 EST

## Scope of the pass

Corrected global and repository-local Git author email to the authenticated EsKaye account's no-reply address. Closed AIRPLAY02 to XMB, FTP-transferred and installed the v1.0.0 candidate on the development PS3, and verified the installed SFO and EBOOT against the package. No launch was performed.

## Why this pass was needed

GitHub attributed recent commits to a different account because the previous no-reply email identified the wrong GitHub user. The user also requested deployment of the verified v1.0.0 package to the development unit.

## Files changed

- `docs/RELEASE_GATE_1.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/air_ps3_v1_0_0_dev_install.md`
- `docs/decisions/2026-11-08_0339_identity-and-install.md`

Git configuration changes are local user/repository settings outside the project tree.

## Architecture / logic changes

No application code changed. Global and repo-local `user.name` are `EsKaye`; `user.email` now uses the authenticated profile's GitHub no-reply identity so new commits map to the intended account.

## Runtime behavior changes

The installed development-unit title is now AIRPLAY02 v1.0.0 / SFO `01.00`. The package was not launched, so runtime behavior is unknown.

## Validation performed

- GitHub profile tool confirmed the authenticated login as `EsKaye`.
- GitHub REST API inspection of the previous latest commit exposed the mismatched account ID in the old no-reply email.
- webMAN exit-to-XMB returned HTTP 200.
- FTP upload followed by download-back: byte-for-byte comparison passed; PKG SHA-256 `ef8f0afdacef560e9f5825fa9539247f3eb17e7791282aa6790f086fa8b1288e`.
- Installed SFO validation and exact hash comparison passed; installed EBOOT size/hash matched package evidence.

## Immediate results

- Development unit has v1.0.0 installed; it was not launched.
- At this point in the timeline, only future commit identity was corrected. The subsequent historical migration is documented in [the attribution rewrite decision](2026-11-08_0345_git-attribution-rewrite.md).

## Known limitations / follow-ups

Verify the next pushed commit's author login through GitHub API. Reattributing existing public history would rewrite commit hashes across descendants and requires a separate reviewed migration plan; it was not performed here.

## Risks introduced

The install replaced the AIRPLAY02 application on the development unit. The v0.2.8 rollback package remains locally available; no system plugin or boot configuration was touched. The primary PS3 was untouched.

## Decision record

### Decision: Correct future Git attribution, preserve existing history

- **Context:** Current commits were associated with a different GitHub account due to the configured no-reply account ID.
- **Options considered:** Fix identity for new commits; rewrite all public branch/tag history immediately; or leave identity unchanged.
- **Choice:** Correct global and repository settings, then verify the next GitHub attribution. Keep existing commit objects unchanged until a dedicated history migration is reviewed.
- **Tradeoffs accepted at this stage:** Older commits still resolved to the former account. A separate, subsequently completed migration corrected the historical metadata while preserving project content.

### Decision: Install without launching

- **Context:** User asked for upload and installation of the validated v1.0.0 package.
- **Options considered:** Install only or immediately launch and begin runtime testing.
- **Choice:** Install and verify package identity; leave launch as a separate hardware test.
- **Tradeoffs accepted:** Deployment is confirmed, but no runtime result is claimed.

## Next recommended step

Verify the next commit is attributed to the EsKaye account, then conduct one user-observable v1.0.0 launch test on the development PS3.
