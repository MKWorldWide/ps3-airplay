# Public release

**Timestamp:** 2026-11-07 00:40 EDT

## Scope of the pass

Prepared a clean public repository snapshot, separated it from the private development history, carried over privacy-reviewed documentation, and recreated the project issues without private workstation details.

## Why this pass was needed

The project is ready to invite community contributions, but its private development history was not suitable for public release. The demo console's non-unique firmware/software configuration is approved as testing context; personal computer details remain private.

## Files changed

- `README.md`, `CONTRIBUTING.md`, and `LICENSE`
- `docs/README.md`, `docs/ARCHITECTURE.md`, `docs/DEV_SETUP.md`, `docs/ROADMAP.md`, `docs/TEST_MATRIX.md`, and `docs/RISKS.md`
- `docs/memory.md`, `docs/lessons-learned.md`, and `docs/scratchpad.md`
- `docs/research/README.md` and `docs/research/PS3_PLUGIN_AIRPLAY_RESEARCH.md`
- This decision record and `docs/decisions/README.md`
- Repository issues #1–#6, recreated with privacy-reviewed descriptions

## Architecture / logic changes

No runtime code changed. The public repository contains the current complete source tree as one clean initial commit. Earlier development commits remain in a separate private archive and are not ancestors of the public repository.

## Runtime behavior changes

None.

## Validation performed

- Reviewed every tracked file in the clean snapshot and scanned staged content for personal paths, workstation identifiers, private addresses, MAC addresses, email addresses, credentials, and agent instruction files.
- Verified local Markdown links, `git diff --cached --check`, single-commit history, commit author metadata, and the final GitHub repository visibility.
- Reviewed branch/tag/ref counts and available GitHub surfaces before release. There were no prior pull requests, issue comments, forks, releases, deployments, wikis, discussions, or workflow runs.
- Retained the prior private repository history in a local owner-only backup and a separate private GitHub archive.

## Immediate results

- The canonical `MKWorldWide/ps3-airplay` repository is public and contains the clean source snapshot, MIT license, contributor guide, and six privacy-reviewed milestone issues.
- The public source contains the approved demo PS3 software configuration but no personal workstation paths or hostnames.
- The prior development history remains private.

## Known limitations / follow-ups

- No PS3 runtime, iPhone session, audible playback, or game-audio coexistence has been demonstrated.
- GitHub package listing and Projects listing were unavailable during the preflight; the clean public repository has no migrated packages or project boards.
- The public release contains one root commit rather than the earlier private development history.

## Risks introduced

- The public repository has a new GitHub repository identity and clean initial history; old private clones must not push their previous history to the public repository.
- Anyone can view and fork the new public repository.

## Decision record

### Decision: Publish a clean repository at the canonical project URL and retain the prior repository privately

- **Context:** The old private repository history contained local development-machine details. Force-pushing a rewritten history can leave cached GitHub views and does not guarantee a full purge.
- **Options considered:** Make the existing repository public after a force-push; replace it with a fresh clean-history repository and retain the old one privately.
- **Why this option was chosen:** The public URL contains no inherited commits or prior GitHub activity, while the original history remains recoverable in the private archive.
- **Tradeoffs accepted:** Public commit history starts at the cleaned snapshot, issue numbers restart in the fresh repository, and the old repository's GitHub identity is retained privately.

### Decision: Include only the approved demo console software context

- **Context:** Hardware configuration helps community testers reproduce target conditions. The maintainer approved the non-unique firmware/software version for publication.
- **Options considered:** Remove all test-device context; publish only the approved version details without unique identifiers.
- **Why this option was chosen:** It supports testing without revealing personal computer details or unique device data.
- **Tradeoffs accepted:** The console model remains unspecified and compatibility is not implied.

## Next recommended step

Invite community host-test and hardware-test reports, then continue timed RTP buffering and sender-clock playback work before claiming audible AirPlay support.
