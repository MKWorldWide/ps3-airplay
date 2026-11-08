# Project consolidation and relocation

**Completed:** 2026-11-08 00:53 EST

**Canonical branch:** `main`

**Final source revision for relocation validation:** `a4ff0f2c0c6f30d306a91b61122c5612807f6291`

## 1. Original repository and worktree inventory

The primary checkout was inside a temporary Codex workspace. Its original `main` revision was `2cfcfe4573d7ab444e973e663663ca5a2ca294d3`. The registered worktrees were:

| Worktree | Branch | Revision before integration | Disposition |
| --- | --- | --- | --- |
| Primary PS3-AirPlay checkout | `main` | `2cfcfe4` | Copied and verified at the canonical destination |
| Fidelity development checkout | `lily/fidelity-v027` | `adebb6837d586effe3cb64f6687a46f8a26ac07d` | Fast-forwarded into `main`; ignored artifacts archived; linked worktree and local branch removed |
| Forensic audit checkout | `lily/sol61-forensic-audit` | `731ae6cadb3746164813832bab96e5eaabf8ed33` | Commit retained as merge ancestry; ignored artifacts archived; linked worktree and local branch removed |

Before relocation, tracked status was clean in all three trees. The two linked trees had ignored `build/` and `work/` directories; these were copied and hash-verified before removing the worktrees. The primary tree also contained ignored local build, log, reference, and nested-repository data, all preserved in the primary tree copy. No worktree was being used by an active agent or build at removal time.

## 2. Destination verification

The sole registered worktree is now `PS3 Airplay` under the existing Projects directory. `git rev-parse --show-toplevel` confirms that location, and `main` is checked out there. The final tree has no broken symlinks. The three preserved symlinks are inside the archived fidelity Python environment and resolve on this Mac.

## 3. Backup and recovery

A full local backup remains outside the repository at `$HOME/PS3-Airplay-Migration-Backup-20261108`. It contains copies of all three original worktree roots, private Git/toolchain baselines, and file manifests. The initial backup manifest matched 4,558 source entries across the three trees with no missing, extra, or differing entries.

After the destination passed build and integrity checks, the obsolete nested checkout was moved into this backup area as an additional recoverable copy. It was not permanently deleted.

Before cutover, a complete manifest of the merged primary tree (including Git metadata and ignored data) matched the destination copy exactly: 4,051 entries. Later differences are expected: the release-gate fix and its documentation, and local verification artifacts under ignored `build/` and `work/`. No original project file was missing or unexpectedly changed. Git metadata was verified independently because its index, reflogs, and object directory naturally change after commits and builds.

## 4–6. Integration decisions and branch history

- `lily/fidelity-v027` was fast-forwarded into `main`, preserving commits `39e918ce15d70e4dec1a25ec2369a166651057f0` (v0.2.7 implementation) and `adebb6837d586effe3cb64f6687a46f8a26ac07d` (validation and documentation).
- Forensic commit `731ae6cadb3746164813832bab96e5eaabf8ed33` was retained as the second parent of merge commit `002e6cf415918f856914ee03a0fb050554c860ee`. Its older tree was not overlaid on the newer v0.2.7 code. The forensic report, tests, analyzer, fixtures, and historical decision are represented in the resulting tree; the decision record was restored explicitly.
- The audit branch did not contain a unique production implementation that should replace v0.2.7. Three audit reproductions passed against the newer source before integration.
- The new tag `baseline-v0.2.7` points to the validated application source revision `39e918c`. Existing rollback tags `safe-v0.2.4` and `baseline-v0.2.6` remain intact.
- The local fidelity and audit branches were removed with safe `git branch -d`. Remote branches `origin/lily/fidelity-v027` and `origin/lily/sol61-forensic-audit` were not deleted. A later remote cleanup may remove them after review; no remote branch cleanup was performed here.

## 7–8. File manifest and Git integrity

The relocation copy passed a byte-for-byte file and symlink manifest comparison. After subsequent commits, the non-Git comparison shows no missing original files; the expected deltas are the new decision/report documentation, build-gate correction, and ignored validation outputs. The fidelity archive preserves 15 ignored build entries and 1,940 ignored work entries; the audit archive preserves 37 build entries and 43 work entries. Each copied entry, including symlink targets, was verified.

`git fsck --full` exits successfully with zero non-dangling diagnostics in both the final and retained repositories. Output contains dangling blobs only; counts vary as prior staged documentation versions become unreferenced. These are not integrity errors, and no garbage collection or pruning was run. `git worktree list --porcelain` reports exactly one worktree. `main` is clean and synced with `origin/main`.

## 9. Build-system path and dependency corrections

Build scripts use their own location to derive the project root and accept external toolchain paths through environment variables. No former workspace path was found in the tracked source tree or reachable Git history. The combined release wrapper was missing the image-conversion step, which had been masked by ignored output from earlier builds. It now creates the RGB background from the checked-in JPEG before packaging. Pillow is documented as a build dependency; Ubuntu `python3-pil` 10.2.0 was installed in the existing Lima guest. The guest retained about 11 GiB free afterward.

The existing x86_64 Lima guest and pinned PS3DK v0.27.0 / PSL1GHT toolchain were reused without resizing the VM. No additional VM was created. No PS3 files, settings, packages, or resident modules were changed.

## 10–11. Regression results and rebuilt package

The complete command `bash scripts/release-lima-standalone-receiver.sh` passed from a fresh tracked-source export of `a4ff0f2c0c6f30d306a91b61122c5612807f6291` in Lima. It ran sanitizer-backed host tests, protocol/crypto/codec/audio tests, timing and display-state tests, PPU component compilation, the standalone build, image generation, and package validation. The independent NumPy/SciPy resampler comparison also passed, including exact chunk/restart continuity and the measured 44.1-to-48 kHz sample count.

| Artifact | Result |
| --- | --- |
| Application / SFO | v0.2.7 / `00.47` |
| Title / content ID | `AIRPLAY02` / `UP0001-AIRPLAY02_00-0000000000000000` |
| PPU ELF | 2,754,928 bytes; SHA-256 `8da9e671981d8f0354558d03b6953e9c79cd6be5108ec683cd56530c58650fe5` |
| Relocated PKG | 3,273,456 bytes; SHA-256 `90ea62e641259098b45515c1c96e95acc38412e555faaae92d3b72456e172c11` |
| Existing validated PKG | 3,273,456 bytes; SHA-256 `48251b3d741328fa78a5cda9bccd76ab02a79ec664d49ce29edfd819eb9f9993` |
| PPU toolchain | PS3DK v0.27.0; PPU GCC 16.1.0; GNU ld 2.42; PSL1GHT revision `dd4e8c9be744e3273d665c393fa6d97a87402e2e` |

The relocated PKG passes the debug/type-1 package validator. Extracted payload comparison:

| Payload | Existing candidate SHA-256 | Relocated build SHA-256 | Result |
| --- | --- | --- | --- |
| `PARAM.SFO` | `ec721c1cb954e2a9ea7afba114e20eab82663995562d932d3fc733d91f170694` | same | exact |
| `ICON0.PNG` | `59c50aad56a8f454aa4d7bbec0717a26d8e9edff906b556efacbfd05bd23738f` | same | exact |
| `background.rgb` | `f5be8f7bce6b56952451b18a1120d89a9d26e1c239fdd13feab16d0a0537105b` | same | exact |
| `EBOOT.BIN` | `9bbc0877b372d64d7e2e46ab3ace253a85a9915139e3d6c0b70f6765398ce201` | `84fe34c255d114ea1b55b014915b8d89e99c4303fcdcb0fb464ffad5b8835615` | differs; both 492,976 bytes |

The fresh PPU ELF was reproducible across two clean exports of the same runtime source (`8da9e671...` both times). Repeated NPDRM wrapping of that same ELF yielded three different EBOOT hashes (`19de9c8731be50cfe4747fcbc95bbf2cb0ce4cb6a91ead3e953c13c511bef750` and `84fe34c...` from the two relocated attempts, versus the existing candidate's `9bbc0877...`). This supports nondeterministic SELF wrapping as the likely cause of the package hash difference; the exact SELF-internal source of variation was not isolated. The source diff from the v0.2.7 implementation commit to the validated main revision contains no changes under `src/` or `prototype/standalone-receiver/`.

The rebuilt PKG is retained locally at `work/builds/consolidation/ps3-airplay-receiver-0.2.7-relocated.pkg` and is not installed. Package validation is host evidence only; it does not prove RPCS3 or PS3 launch, display restoration, AirPlay playback, or audible output.

## 12–13. Worktrees and branches

Only the canonical `main` worktree remains registered. Both linked worktrees were removed without force after checking clean tracked state, preserving ignored data, and confirming no active agent or build process. Local development branches were safely deleted. Both corresponding remote development branches remain available for review and rollback; no force-push or history rewrite occurred.

## 14–16. Remaining risks and next actions

- The repository history and current tracked tree contain no old local workspace paths. A history scan found one email-shaped value in the RAOP discovery test fixture: it is a fabricated twelve-hex placeholder with a `.local` service name, not an account address. A private local scan found zero user-specific absolute paths in 217 text/log/JSON files; those logs and archives remain ignored and private.
- This was not the separate full audit of every GitHub surface (issues, discussions, workflows, packages, or all content before any visibility change). No visibility change or GitHub Release was made as part of this migration.
- The v0.2.7 hardware observations remain as previously recorded. XMB resume/display behavior and audible audio quality were not retested during relocation. No console installation was performed.
- `origin/lily/fidelity-v027` and `origin/lily/sol61-forensic-audit` are safe remote cleanup candidates now that their useful work is represented in `main`; remote deletion is deferred for a separate reviewed operation.
- Future development should start from `PS3 Airplay` on `main`, run the single release gate, and keep device tests separate from build validation.

## Final Git state

- Branch: `main`
- Revision before this report commit: `a4ff0f2c0c6f30d306a91b61122c5612807f6291`
- Tracked working tree: clean
- Registered worktrees: one
- Preserved tags: `safe-v0.2.4`, `baseline-v0.2.6`, `baseline-v0.2.7`
- Remote development branches: retained
- PS3: untouched
