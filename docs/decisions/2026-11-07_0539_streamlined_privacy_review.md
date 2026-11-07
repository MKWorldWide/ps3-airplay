# Streamlined GitHub privacy review

## Timestamp in EDT

2026-11-07 05:39 EST (America/New_York)

## Scope of the pass

Add a concise routine privacy-review procedure to the private global agent instructions and the public project's contributor instructions. Update project memory, lessons, and active context.

## Why this pass was needed

The maintainer asked to reduce time and token use spent on pre-commit sanitation while preserving the complete privacy review before each commit or push.

## Files changed

- `CONTRIBUTING.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_0539_streamlined_privacy_review.md`

## Architecture / logic changes

The global workflow and repo contribution guide now share a three-step review: inspect the full staged diff and exact changed files; run whitespace, already-available secret, and targeted private-data checks; manually inspect matches. Routine findings are sanitized or redacted automatically and rescanned. Only content whose safe remediation requires guessing or could alter intended meaning is excluded as a narrow blocker; independent work continues. Unavailable scanners are recorded without installing extra tools for routine commits. The global instruction file remains private and outside the public repository.

## Runtime behavior changes

No product runtime behavior changed. Future GitHub commits and pushes use a compact repeatable sanitation pass without weakening the publication gate.

## Validation performed

- Read the existing global privacy gate and repo contribution checklist before editing.
- Confirmed both instruction locations contain the new workflow.
- Confirmed the global instruction file is not tracked by the project repository.
- Before push: inspected the complete staged diff and exact staged file list; `git diff --cached --check` passed. Targeted scans found no local paths, private IP/MAC addresses, emails, credentials, or key material. Three raw-log pattern hits were mentions in the new review instructions and were manually confirmed not to contain logs or device data. `gitleaks`, `trufflehog`, and `detect-secrets` are unavailable; no scanner was installed solely for this pass.

## Immediate results

- Global and project instructions now prescribe the requested brief but mandatory review.
- No scanner is installed solely for this workflow; this is disclosed in the instruction.
- No product runtime behavior or console state was changed.

## Known limitations / follow-ups

Targeted searches depend on repository-specific patterns and cannot prove that content is safe; manual review remains required. The separate full audit before changing a private repository to public is unchanged.

## Risks introduced

The procedure could be misread as allowing less review. Both copies explicitly retain the complete staged-diff/file review, manual match inspection, remediation, rescanning, and a narrow stop condition for unsafe-to-resolve content. The global instruction file must remain private.

## Decision record (embedded)

### Decision: Use a compact scan-and-remediate checklist

- **Context:** Routine privacy reviews were taking too long, and the user wants low human intervention after setting clear goals.
- **Options considered:** Keep the verbose repeated procedure; remove checks; use one automated pass followed by focused manual review and automatic remediation.
- **Why this option was chosen:** It cuts redundant review work while preserving whitespace checks, available secret scanning, targeted searches, exact file review, automatic sanitation, and a narrow safety hold for ambiguity.
- **Tradeoffs accepted:** Automated searches may miss unfamiliar identifiers, so manual inspection remains mandatory. Some ambiguous content may still need to be withheld from publication.

### Decision: Keep global instructions private and publish only project guidance

- **Context:** The same practical workflow is useful to contributors, while the global agent file contains system-wide instructions and must not be copied into the public repository.
- **Options considered:** Publish a project `AGENTS.md`; add the safe workflow to contributor guidance; update only the global file.
- **Why this option was chosen:** `CONTRIBUTING.md` exposes only the project-relevant procedure and keeps private global instructions outside GitHub.
- **Tradeoffs accepted:** The public contributor guidance and private global file are maintained separately and can drift; review both when updating this policy.

## Next recommended step

Run the compact review on the staged project files, commit and push the project changes, then resume PS3-AirPlay loader diagnosis from the recorded v0.1.6 boundary.
