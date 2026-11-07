# Active work

- Current target: AIRPLAY02 v0.1.34 / SFO 00.34. v0.1.33 is the first build with user-confirmed Spotify music; quality crackles and the screen flickers on XMB resume.
- Candidate change: retain throttled RTP resend gaps; accept delayed recovery against up to 16 outstanding ranges; reduce packet/decode log rate and emit five-second recovery totals.
- Lima ASan/UBSan suite and PPU component compilation pass. Full PPU app/package validation and upload/install are next.
- Hardware retest: short AirPlay stream and retrieve counters, then PS-menu resume twice and clean START exit. Do not run long soak until quality/resume improve.
- Private log and all generated artifacts remain in ignored `work/`; boot plugins and resident module remain untouched.
