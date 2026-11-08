# Current scratchpad

- Task: isolated overnight development of diagnostic resident AirPlay PRX on `experimental/resident-airplay`; never alter `main`, published v1.0.0, or physical PS3s.
- Worktree is separate; stable checkout was rechecked clean on `main` at the original base commit.
- Current changes: bounded worker exit wait; network finalize/unload failures retain retryable ownership state; build-configurable log/RTSP/RTP ports; dedicated `RPRX90001` lifecycle harness; fresh package build plus extracted-PRX byte comparison.
- Validation: full Lima suite/build/PRX structure/PKG exact-match passed; forensic RTP/reorder/resend/flush/jitter/SRC/display-resume/cost tests passed. Earlier one-off audio observation assertion has not recurred in isolated/full/repeat runs.
- RPCS3 0.0.43-20570 process is running, but desktop automation could not access the locked Mac. User was asked to unlock it; no install/run/settings change has happened.
- PCM: host synthetic encrypted RTP→ALAC→PCM tests pass. Resident PRX clears/discards PCM; no output device or coexistence experiment.
- Next: if unlocked, run three emulator cycles and retain exact logs. Then finalize report with artifact hashes, status table, and NOT HARDWARE READY decision if lifecycle remains untested.
