# Integrate the validated v0.2.7 work into main

Timestamp: 2026-11-08 00:23:13 EST (UTC-05:00; America/New_York).

## Scope of this pass

Fast-forwarded the tested fidelity branch into `main`. Preserved the forensic-audit commit as a second parent using a reviewed selective merge, restored its unique decision record, and refreshed project state/test documentation with current v0.2.7 hardware evidence.

## Why this pass was needed

The production branch remained at v0.2.6 while the v0.2.7 implementation, tests and validation were isolated in a linked worktree. The forensic branch contained a historical v0.2.5 audit but its current tree predated later fixes.

## Files changed

- `docs/decisions/2026-11-07_2306_sol61_forensic_audit.md`
- `docs/decisions/2026-11-08_0023_project_consolidation_integration.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/ROADMAP.md`
- `docs/RISKS.md`
- `docs/scratchpad.md`
- `docs/TEST_MATRIX.md`

The v0.2.7 source and regression files arrive from their existing commits; no release version or runtime behavior was changed in this pass.

## Architecture / logic changes

The 64-tap, 160-phase FIR and aggregate audio-service diagnostics are now part of main, preserving their tested implementation and exact 160:147 conversion ratio. The display presenter keeps the bounded 350ms resume guard. No synchronization or resend-policy rewrite was introduced.

The forensic commit adds no production code unique to its branch. Its forensic report, analyzer, fixtures and tests were already represented by newer main files. The current main versions were retained; the older branch's stale v0.2.5-era tree state was not imported. Its commit ancestry and unique audit decision record were preserved.

## Runtime behavior changes

This is an integration/documentation pass; the v0.2.7 behavior already present in the validated package is now the main-branch implementation. The dev log records a 30m03s application run, 29m23s of RECORD sessions, RTP/ALAC/PCM progress, one 26,814-frame PCM-underflow delta, and clean START cleanup. It does not establish audible quality or physical XMB-resume behavior.

## Validation performed

- `git diff main...lily/fidelity-v027 --stat` and file-level review identified 27 files/2 commits.
- `git diff main..lily/sol61-forensic-audit` and exact file comparisons showed no production source unique to the older branch.
- Audit RTP/PCM regression, 1,416-byte ALAC reference decode, and two log-analyzer tests passed against the newer main source.
- v0.2.7 reference package verified at 3,273,456 bytes and SHA-256 `48251b3d741328fa78a5cda9bccd76ab02a79ec664d49ce29edfd819eb9f9993`; SFO AIRPLAY02/00.47 and the debug-format PKG envelope pass their validators.
- Full Linux host/sanitizer/PPU/ELF/SELF/PKG gate previously passed for packaged source `39e918ce15d70e4dec1a25ec2369a166651057f0`; it will be rerun from the relocated checkout.
- Hardware evidence is from the private appended dev log. No console command or installation was performed in this pass.

## Immediate results

The validated v0.2.7 implementation is now on main at the fidelity branch tip. Both rollback tags remain untouched. A package-type check initially used the finalized format; the artifact is intentionally a type-1 debug PKG, and validation passed when run with the correct format.

## Known limitations / follow-ups

The full clean release gate and binary/package reproducibility comparison remain pending from the final directory. Physical visible XMB resume and audible-output confirmation remain unresolved.

## Risks introduced

No runtime risk was added. The migration itself remains incomplete; original worktrees and all private ignored artifacts remain preserved until post-move validation succeeds.

## Decision record

### Preserve the fidelity branch history with a fast-forward

- **Decision:** Fast-forward main to `adebb68`, preserving both existing commits.
- **Context:** The source revision used for the validated package is the ancestor `39e918c`; the later commit adds validation documentation/tests.
- **Options considered:** Cherry-pick files, squash, or fast-forward.
- **Why chosen:** Fast-forward preserves original authorship and commit provenance without rewriting history.
- **Tradeoffs accepted:** The branch's docs/tests become part of main alongside the validated source.

### Selectively retain forensic history without its stale tree

- **Decision:** Keep the audit commit as merge ancestry and restore its unique decision record, while retaining current main files for duplicated or older reports/tests.
- **Context:** The branch tip is based on v0.2.5 and its tree differs from newer v0.2.6/v0.2.7 source.
- **Options considered:** Normal merge, discard the branch, or selective ancestry/content integration.
- **Why chosen:** The audit's primary report and regressions already exist in newer main content; importing the old full tree would roll back source and docs.
- **Tradeoffs accepted:** A deliberate `ours` ancestry merge must be explained in the final migration report.

### Keep release identity unchanged

- **Decision:** Keep app v0.2.7 and SFO 00.47 for the relocation comparison.
- **Context:** No runtime source change is being made after the validated package.
- **Options considered:** Increment the package version or compare the exact known artifact.
- **Why chosen:** A new version would obscure whether relocation alone changed the build.
- **Tradeoffs accepted:** Any later runtime fix must use the next version step.

## Next recommended step

Finish this reviewed integration commit and push main. Then archive the ignored fidelity/audit work data inside main's private `work/`, relocate the repository, run the complete regression/release gate from the new path, compare the rebuilt package, and only then remove linked worktrees/local branches.
