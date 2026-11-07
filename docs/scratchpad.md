# Active work

- **Current task:** Build AIRPLAY02 v0.1.19 with the ALAC negotiated-frame bound, validate its ELF and NPDRM package, then test in RPCS3 when the Mac desktop is unlocked.
- **Source change:** ALAC decode rejects in-band sample counts outside the negotiated maximum before internal array writes. `raop_audio_pipeline_test` composes RTSP RSA-OAEP key unwrap, encrypted RTP payload decryption, ALAC decode, and golden PCM comparison; it also verifies over-limit frame rejection.
- **Validation:** Full Lima host suite and PPU objects passed after the change under ASan/UBSan. Integration output matched the reference PCM. A full standalone app and package build from the committed source remains pending.
- **RPCS3:** Last UI-access attempt reported Mac locked. Earlier local RPCS3 run remains v0.1.15 and is not current evidence. Do not claim v0.1.19 was installed or run until a fresh log is captured.
- **Version:** Next package 0.1.19; package script defaults already align. No physical PS3 action is authorized in this turn.
- **Next:** Commit source and docs after the brief GitHub privacy gate; build v0.1.19 from that exact commit in Lima; validate and record exact artifact hashes; retry RPCS3 only if UI access becomes available.
- **Boundary:** Host integration tests do not prove iPhone RTSP/RTP interoperability, GameOS audible output, console display/exit, Spotify playback, or game-audio coexistence.
