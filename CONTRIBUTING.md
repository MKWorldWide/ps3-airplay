# Contributing

Thanks for helping improve PS3 AirPlay. Contributions and testing reports are welcome. The project is experimental, and clear evidence is more useful than a broad compatibility claim.

## Before starting

- Check the [roadmap](docs/ROADMAP.md), [open issues](https://github.com/MKWorldWide/ps3-airplay/issues), and [architecture](docs/ARCHITECTURE.md).
- Follow the project's [versioning policy](docs/VERSIONING.md); every new package consumes a unique version, including failed candidates.
- For substantial changes, open or comment on an issue to agree on scope.
- Keep protocol parsing, memory use, sessions, queues, and diagnostic output bounded.
- Keep standalone application work separate from resident PRX work. A PRX build or structural check does not establish Cobra compatibility.
- Preserve third-party attribution and license terms; update `THIRD_PARTY_NOTICES.md` when adding or changing vendored material.

## Changes and checks

- Add or update focused host tests for portable protocol, codec, queue, and logging behavior.
- Run the relevant test executable(s) and cross-build target(s) when available; include exact commands and results in the pull request.
- Run `git diff --check` before submitting.
- Do not claim PS3, iPhone, audible playback, or game compatibility without reporting the exact test that demonstrates it.

## Before committing or pushing

Keep the privacy review brief and repeatable:

1. Inspect the full staged diff, exact staged file list, and all new or changed files. Check that private working files and local instructions remain ignored and unstaged.
2. Run `git diff --cached --check`, available secret scanners, and targeted searches for personal paths/usernames, machine names, private URLs, email addresses, private IP/MAC addresses, Wi-Fi names, device/account IDs, credentials, and raw logs.
3. Manually inspect every match and the exact files being committed. Keep normal project details and fabricated examples when useful; sanitize or redact private identifiers, secrets, and unredacted device data, then rerun affected checks. Do not pause for routine sanitation. If safe remediation requires guessing or could change intended meaning, leave only that content out of the commit, report the narrow blocker, and continue independent work.

This is a compact workflow, not a skipped check. If no secret scanner is already available, note that and complete the targeted searches and manual review without installing one just for the commit.

## Hardware test reports

Use the relevant rows in [the test matrix](docs/TEST_MATRIX.md). Include the artifact revision or checksum, broad hardware/software versions, test steps, result, and concise observations. Redact console serials and account IDs, MAC/IP addresses, Wi-Fi names, credentials, and raw logs that may contain them. Do not upload screenshots or logs before reviewing them for identifiers.

Hardware tests must use equipment you control or are authorized to test. Do not alter boot-time plugin configuration as part of an unreviewed test. Follow the [risk and safety gates](docs/RISKS.md).

## Pull request checklist

- [ ] The change has a clear scope and links to relevant discussion.
- [ ] Tests or build checks are listed with their actual results.
- [ ] Documentation and third-party notices are current.
- [ ] No credentials, personal paths, machine names, device identifiers, or unredacted logs are included.
- [ ] Claims match the validation performed.
