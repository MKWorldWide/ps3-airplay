# Consolidate and verify the canonical project checkout

**Timestamp:** 2026-11-08 00:53 EST

## Scope of the pass

Integrated validated v0.2.7 work into `main`, retained forensic commit ancestry, preserved ignored worktree data, relocated the single working checkout, repaired the combined release gate's missing image-generation step, rebuilt and validated v0.2.7 from a clean exported source, and recorded migration evidence.

## Why this pass was needed

The project had split across temporary worktrees, while ignored build outputs and old release artifacts could hide missing setup steps. The requested permanent checkout needed a verified backup, recoverable branch history, a clean single-tree layout, and a full build from the new location.

## Files changed

- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-08_0053_project_consolidation_verified.md`
- `docs/reports/project_consolidation_and_relocation.md`

Ignored local archive/build outputs were preserved under `work/`; they are not tracked.

## Architecture / logic changes

No application architecture changed. The release wrapper now generates the runtime RGB image from the checked-in JPEG before packaging. `main` contains the two fidelity commits, plus an `ours` merge retaining the forensic audit commit as ancestry while preserving the newer source tree. Tag `baseline-v0.2.7` points to the validated runtime source revision.

## Runtime behavior changes

None. Application version remains v0.2.7 and SFO remains 00.47.

## Validation performed

- Verified a full three-worktree backup and source/backup manifest before changes.
- Removed the two clean linked worktrees without force after archiving and hash-verifying ignored build/work contents.
- Copied the primary checkout to the requested Projects destination; all 4,051 source entries matched at cutover.
- `git fsck --full`: exit 0, zero non-dangling diagnostics in both final and retained repositories; dangling blobs only, with no pruning.
- Full `bash scripts/release-lima-standalone-receiver.sh` passed from clean exported commit `a4ff0f2c0c6f30d306a91b61122c5612807f6291` in Lima. Host/sanitizer protocol, RTP, ALAC, PCM, resampler, logging, and display-state tests passed; PPU ELF, SFO, extraction, asset, content-ID, and PKG validators passed.
- Independent NumPy/SciPy resampler fidelity test passed with exact chunk/restart continuity.
- Rebuilt package hash `90ea62e641259098b45515c1c96e95acc38412e555faaae92d3b72456e172c11`, 3,273,456 bytes; ELF hash `8da9e671981d8f0354558d03b6953e9c79cd6be5108ec683cd56530c58650fe5`.
- Checked reachable Git blobs for old local paths/private network identifiers. No old workspace path was found. The only email-shaped fixture is a fabricated RAOP `.local` address using a standard placeholder device ID. Scanned 217 local text/log/JSON files for user-specific absolute paths; none were found.
- PS3 left untouched; no GitHub Release created.

## Immediate results

One canonical `main` worktree remains at `PS3 Airplay`; tracked status is clean and synced. The original validated v0.2.7 PKG and rollback tags remain. A fresh local v0.2.7 package was produced from the relocated source and validated. SFO and non-executable assets match the reference package; the NPDRM EBOOT bytes differ, while repeated wrapping of an identical ELF also yielded different EBOOT bytes. No source change exists between the validated v0.2.7 runtime revision and relocated release source under `src/` or the standalone receiver.

## Known limitations / follow-ups

The exact source of NPDRM SELF byte variation was not isolated, and a SELF-normalized comparison was not performed. No emulator or hardware launch was part of the relocation test. XMB resume/display stability and end-to-end audio require separate testing. Remote development branches were intentionally retained.

## Risks introduced

The release wrapper now requires Pillow on the build host. No runtime or console risk was introduced. Git reports unreferenced dangling blobs; they were not pruned.

## Decision record

### Decision

Keep the validated fidelity implementation as `main`, preserve audit ancestry without overlaying its older tree, retain remote branches, and use the relocated source as the sole local working tree.

### Context

The fidelity branch held the v0.2.7 runtime changes; the audit branch originated from older source and contained forensic evidence but no production changes worth replacing newer code with.

### Options considered

1. Merge both branch trees wholesale.
2. Fast-forward fidelity, retain audit ancestry selectively, restore useful audit documentation/tests, and preserve remote refs.
3. Leave multiple linked worktrees as active development locations.

### Why this option was chosen

Option 2 preserves the tested runtime, audit history, rollbackability, and one predictable local checkout without reverting newer files.

### Tradeoffs accepted

Two remote branches remain until separately reviewed. The new package is validated structurally but not launched, because the assignment forbids console deployment and treats build validation separately from hardware behavior.

## Next recommended step

Continue application development from the canonical `main` tree. Review audio service/underflow evidence and display resume on hardware independently; consider remote branch cleanup only after a separate reviewed decision.
