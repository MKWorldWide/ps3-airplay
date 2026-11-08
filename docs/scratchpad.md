# Current scratchpad

- Task: isolated overnight development of diagnostic resident AirPlay PRX on `experimental/resident-airplay`; never alter `main`, published v1.0.0, or physical PS3s.
- Separate worktree; stable checkout remains clean on the original `main` base.
- Changes include bounded worker wait, retryable network teardown, configurable resident ports/log, dedicated `RPRX90001` harness, exact packaged-PRX verification, and per-cycle PRX module-count baseline checks.
- Full Lima host suite, forensic suite, PPU PRX build, structural check, and package extraction/comparison passed. Earlier one-off 500 ms receiver observation failure has not recurred in isolated, full, or repeated runs.
- Harness module-count check is compiled but not runtime-tested because computer-use cannot access the locked Mac. The matching RPCS3 source registers PRX lifecycle and module-list APIs; runtime results remain unknown.
- Host synthetic encrypted RTP→ALAC→PCM tests pass; resident PRX clears/discards PCM. No audio output or background coexistence.
- Candidate report is drafted; current readiness is NOT HARDWARE READY. Next: run three RPCS3 cycles if the Mac is unlocked, refresh hashes/report, then final branch handoff.
