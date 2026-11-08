# Prepare the AIR PS3 v1.0.0 release candidate pipeline

**Timestamp:** 2026-11-08 03:17 EST

## Scope of the pass

Centralized package version and identity, updated current-facing Gate 0 and release documentation, added package evidence generation and verification, prepared a fail-closed local draft-staging helper and manual publish workflow, and included upstream license texts for release distribution.

## Why this pass was needed

The v0.2.8 Gate 0 evidence was accepted by the user, but current docs retained an older 10m41s test boundary and the build scripts had stale/inconsistent app/SFO defaults. A public release needed exact artifact provenance, notices, install guidance, and an explicit publishing path.

## Files changed

- `.github/workflows/publish-release.yml`
- `README.md`, `release.json`, `THIRD_PARTY_NOTICES.md`
- `docs/BUILD_PIPELINE.md`, `docs/INSTALLATION.md`, `docs/PUBLIC_RELEASE_PLAN.md`, `docs/README.md`, `docs/RELEASE_GATE_1.md`, `docs/RELEASE_NOTES-v1.0.0.md`, `docs/RISKS.md`, `docs/ROADMAP.md`, `docs/TEST_MATRIX.md`, `docs/VERSIONING.md`
- `docs/memory.md`, `docs/lessons-learned.md`, `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0317_v1-release-candidate-pipeline.md`, `docs/decisions/README.md`
- `licenses/MBED-TLS-3.6.7-LICENSE.txt`, `licenses/PS3DK-LICENSE.txt`, `licenses/PSL1GHT-LICENSE.txt`, `licenses/PSL1GHT-RSX-LICENSE.txt`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`, `scripts/package-lima-standalone-receiver.sh`, `scripts/release-lima-standalone-receiver.sh`, `scripts/read-release-config.py`, `scripts/test-release-config.py`, `scripts/test-release-evidence.py`, `scripts/write-release-evidence.py`, `scripts/verify-release-evidence.py`, `scripts/stage-github-release.sh`

## Architecture / logic changes

`release.json` is the shared source for app `1.0.0`, fixed-width PS3 SFO `01.00`, title ID AIRPLAY02, XMB title AIR PS3, AirPS3 RAOP label, and debug PKG format. Build/package scripts and the standalone Makefile read that configuration; the combined release gate rejects conflicting version overrides. The version regression accepts the next mapping `1.0.1` / `01.01` and rejects malformed semantic versions.

On a complete Lima release-gate pass, the evidence writer records exact source SHA, toolchain identifiers, package/ELF/payload sizes and hashes, SFO identity and completed validation statuses without local filesystem paths. The verifier rejects source/version/hash/envelope/status mismatches. Draft staging also requires a clean tree, the gate report to say PASS, and the selected source to be integrated into `origin/main`. A deliberate manual workflow dispatch performs validation with `contents: read`; only the final publish job has `contents: write`.

The release path consumes a locally built PKG. The pinned PS3DK/PSL1GHT/Mbed TLS environment exists in the existing Lima guest and is not provisioned as a GitHub-hosted or self-hosted Actions compiler. The package includes accompanying installation and third-party license texts. The user explicitly authorized the supplied artwork for public project/PKG use; no broader artwork license is claimed.

## Runtime behavior changes

None. The standalone receiver and both PS3s are unchanged. App startup identity will read version `1.0.0` in a future package produced by this pipeline.

## Validation performed

- `python3 scripts/test-release-config.py`: PASS (`1.0.0`/`01.00` to `1.0.1`/`01.01`).
- `python3 scripts/test-release-evidence.py`: PASS; valid synthetic evidence accepted, package-hash and pending-validation mismatches rejected.
- `python3 -m py_compile` for release config/evidence helpers: PASS.
- `bash -n` for build, package, release, and staging shell scripts: PASS.
- `git diff --check`: PASS before final staged review.
- Staged privacy scan: no targeted local paths/usernames, private IPv4/MAC, email, or populated secret assignments found; no existing secret scanner was available, so none was installed. Complete changed-file review found no private material. Agent instructions and ignored work artifacts are not tracked.
- Existing Lima guest: 11 GiB free; PS3DK source, package tools, PSL1GHT checkout, and Mbed TLS 3.6.7 license available. No build process was running at inspection. Full release build remains to run after commit.
- Read-only GitHub check: repository is public, default branch main, no releases listed.
- User supplied Gate 0 summary: accepted ~45-minute v0.2.8 run, four XMB cycles, no specified error counters; ~45 ms max service interval during resume accepted as a known hitch. Not independently re-audited here.

## Immediate results

Version defaults are no longer split across scripts and docs. Release Gate 1 remains BLOCKED: full v1.0.0 package build is pending, source is on a feature branch rather than integrated into origin/main, and final privacy/notices review must pass. No tag or GitHub Release was created. No PS3 was installed, restarted, or changed.

## Known limitations / follow-ups

- Build the exact committed revision in Lima and inspect the actual SFO, ELF/SELF, package contents, asset hashes, and generated evidence.
- Source/doc privacy review passed with compact automated scans and manual changed-file review; repeat it for exact generated release assets before staging.
- Integrate the validated source into the intended release branch and rebuild from the final selected SHA before staging.
- A fresh v1.0.0 hardware test remains useful but is outside this non-console-changing preparation pass.

## Risks introduced

- An inaccurate `release.json` edit could still choose an unintended but syntactically valid version; the version regression checks the intended current and next mapping.
- The GitHub workflow assumes a draft release/tag with verified assets exists and that manual dispatch is made from main. It does not build or sign a package.
- Release publication is blocked until review gates pass. Automated hash/evidence checks do not replace third-party rights review or hardware acceptance.

## Decision record

### Decision 1 — Use `release.json` as the sole current version source

- **Context:** Scripts and docs had stale defaults. The requested app version is 1.0.0 and PS3 SFO fields require fixed-width XX.YY; the explicit value is 01.00 (displayed numerically as 1.00).
- **Options considered:** Keep explicit environment overrides at each call site; hard-code 1.0.0 in every script; or read one validated config.
- **Why chosen:** One source reduces mismatches and prevents accidental stale output.
- **Tradeoffs accepted:** Future version increments require editing two paired values in one manifest and running its mapping test.

### Decision 2 — Build in Lima and publish only the exact local artifact

- **Context:** The working PS3 toolchain and package tools are in the existing x86_64 Lima guest; Actions has not been provisioned with this pinned environment.
- **Options considered:** Assume hosted Actions can reproduce the local setup; use a self-hosted runner; or validate/publish a locally built candidate.
- **Why chosen:** A local artifact path preserves the toolchain already used and avoids exposing a persistent self-hosted runner to PR code.
- **Tradeoffs accepted:** A release operator transfers the exact package/evidence bundle; automation validates it before publication but does not build it.

### Decision 3 — Keep tag/release creation gated

- **Context:** The release brief prohibits tag/publication when any build, package, privacy, licensing, or release check fails.
- **Options considered:** Create a tag now and add a draft; or require a passing gate document, source integration, clean tree, candidate evidence and explicit manual publication.
- **Why chosen:** A blocked candidate remains reviewable without leaving a misleading public version tag.
- **Tradeoffs accepted:** Release staging remains unavailable until Gate 1 status is explicitly changed to PASS after documented review.

## Next recommended step

Commit and push this sanitized implementation on the existing feature branch, then build the exact source archive in Lima and record the resulting v1.0.0 / SFO 01.00 package evidence.
