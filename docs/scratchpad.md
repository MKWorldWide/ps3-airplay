# Active project consolidation

- `main` is integrated and pushed at the v0.2.7 + forensic-history merge; only one primary worktree remains.
- The project is byte-verified at the requested `PS3 Airplay` destination; prior checkout and all three worktree states have a verified private backup. Linked worktree ignored `build/` and `work/` contents are archived under ignored `work/archives/`.
- Relocated release gate exposed that the RGB background generation step was missing. The script fix is staged for commit; Pillow is installed in Lima. Re-export the new commit and rerun the full release gate.
- Then compare rebuilt package internals/hash with v0.2.7 reference; finish migration report and integrity checks. Do not install on either PS3.
