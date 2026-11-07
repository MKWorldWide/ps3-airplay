# Active work

- **Current task:** Install/run the validated AIRPLAY02 v0.1.19 in RPCS3 once the Mac desktop is unlocked; collect a fresh log and test display/exit/readiness boundaries.
- **Source change:** ALAC decode rejects in-band sample counts outside the negotiated maximum before internal array writes. `raop_audio_pipeline_test` composes RTSP RSA-OAEP key unwrap, encrypted RTP payload decryption, ALAC decode, and golden PCM comparison; it also verifies over-limit frame rejection.
- **Validation:** Full Lima host suite and PPU objects passed under ASan/UBSan, including RSA-OAEP unwrap → encrypted RTP decrypt → ALAC decode → reference PCM equality and over-limit rejection. v0.1.19 ELF and finalized PKG built from `7b45103` and passed structural/package checks.
- **Artifact:** PKG 3,230,704 bytes, SHA-256 `41488b9683b901ecc4d71e113565fcdb10c1834c69e1642fbe774f5cdf4a31a5`; ELF SHA-256 `beb816b2aed2125a94584e4e820f18de66a98f21260ce47fe1654199322e7ec3`. Local ignored copy: `work/builds/0.1.19/`.
- **RPCS3:** Computer Use twice reported Mac locked, so v0.1.19 is not installed or run. Earlier RPCS3 log is v0.1.15 and not current evidence.
- **Version:** v0.1.19 package built; next is 0.1.20. Physical PS3 not touched.
- **Next:** Retry RPCS3 only when desktop becomes available, then inspect the fresh log. Continue host protocol hardening meanwhile if still locked.
- **Boundary:** Host integration tests do not prove iPhone RTSP/RTP interoperability, GameOS audible output, console display/exit, Spotify playback, or game-audio coexistence.
