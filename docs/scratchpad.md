# Active work

- **Current task:** Complete ALAC malformed-bitstream hardening, build AIRPLAY02 v0.1.20, then test in RPCS3 when the Mac desktop unlocks.
- **Source change:** In addition to negotiated sample-count limits, reject invalid predictor/quantization/interlace fields, constrain Rice k-modifier, and use bounded wider arithmetic for predictor and weighted-interlace math. The composed encrypted RTP-to-PCM golden path remains in `raop_audio_pipeline_test`.
- **Validation:** Full Lima host suite passed under ASan/UBSan halt-on-error, including 256 deterministic malformed packets, over-limit/`UINT32_MAX` rejection, reference PCM equality, and the loopback timing test. PPU component object compilation passed. Full standalone app and v0.1.20 package validation remain pending.
- **Artifact:** PKG 3,230,704 bytes, SHA-256 `41488b9683b901ecc4d71e113565fcdb10c1834c69e1642fbe774f5cdf4a31a5`; ELF SHA-256 `beb816b2aed2125a94584e4e820f18de66a98f21260ce47fe1654199322e7ec3`. Local ignored copy: `work/builds/0.1.19/`.
- **RPCS3:** Computer Use twice reported Mac locked, so v0.1.19 is not installed or run. Earlier RPCS3 log is v0.1.15 and not current evidence.
- **Version:** v0.1.19 package built and preserved; current source changes are intended for v0.1.20. Physical PS3 not touched.
- **Next:** Run full Lima host/PPU suite, build v0.1.20 from the exact commit, validate package hashes, then retry RPCS3 when desktop becomes available.
- **Boundary:** Host integration tests do not prove iPhone RTSP/RTP interoperability, GameOS audible output, console display/exit, Spotify playback, or game-audio coexistence.
