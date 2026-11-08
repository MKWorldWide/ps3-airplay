# Current scratchpad

- Task: isolated overnight development of the diagnostic resident AirPlay PRX on `experimental/resident-airplay`; do not alter stable `main`, published v1.0.0, or physical consoles.
- Worktree: separate worktree at `../ps3-airplay-resident-airplay`.
- Completed this pass: added configurable resident logs/ports, bounded worker-exit wait, RPCS3 lifecycle harness using unused `RPRX90001`, and fixed explicit libm link plus package overlay inclusion.
- Harness failure paths now attempt to unload a module after a failed or nonresident start; successful starts still require checked stop before unload.
- Validation: full Linux host suite passed after one earlier intermittent RTP observation timeout (isolated rerun and full suite passed); PPU PRX build and structural check passed (6094 relocations, 4 imports); PKG inspection confirms embedded PRX.
- Emulator: RPCS3 0.0.43-20570 is installed/running, but computer-use reports the Mac is locked. User has been asked to unlock it for package install and run. No settings changed. Continue host-level verification meanwhile.
- PCM: host encrypted RTP→ALAC→PCM tests pass; resident PRX still zeroes/discards decoded PCM. No device audio or background coexistence.
- Next: obtain RPCS3 execution evidence if UI becomes accessible; test three load/start/stop cycles, record exact failures; produce overnight report, hashes, hardware readiness, final branch push.
