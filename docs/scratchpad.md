# Current state

- AIRPLAY02 v0.2.6 / SFO 00.46 is installed and running; log confirms RECORD, encrypted 44.1 kHz AirPlay, ALAC decode and PCM submission.
- User reports major stuttering has stopped; slight audio unsteadiness remains. Latest ~3-minute stream log: 3,174/3,282 sequence gaps recovered, 108 concealed (~0.86 s), zero ALAC failures, PCM underflows or drops; PCM occupancy stable at ~74-77%.
- `SCREEN_RESUME_SETTLED` followed MENU/DRAW callbacks, but physical display stability is not verified.
- Next: keep current test conservative; collect a longer same-source window if user continues playback. Compare residual concealed gaps/retry behavior and preserve the fresh log before changing code.
- Preserve safe-v0.2.4, prior packages, and private hardware logs.
