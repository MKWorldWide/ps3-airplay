# Active work

- **Current task:** Build and runtime-check v0.1.22; continue toward an iPhone RAOP session.
- **Candidate:** v0.1.22 logs successful RTP payload decryption at bounded checkpoints. Lima ELF/PKG validation passes; PKG SHA-256 `c7242d47669a4a22090663c969e2b4f5efd4c0cc9a0eb6ef65ad18d730080a2b`.
- **Host validation:** Full Lima suite passes, including discovery TXT assertion; RTSP/TCP + encrypted RTP/UDP + decrypt + ALAC + PCM ring loopback; 4,096 seeded ALAC mutations plus 256 random inputs under ASan/UBSan; and PPU component compilation.
- **Emulator evidence:** Installed AIRPLAY02 was v0.1.15 at the last accessible snapshot. A direct `--no-gui` v0.1.20 EBOOT attempt did not update the capped app log; classify it as inconclusive. The user reports a visible app/background but also display overlays absent from current source and v0.1.20 ancestry. Exact observed package remains unknown. CUA still reports the desktop locked.
- **Next:** RPCS3 UI still reports locked and the current on-disk app log is v0.1.15; when accessible, install the exact package, capture a fresh matching log, inspect display/START exit, then follow the earliest receiver failure. Host simulation is not iPhone, GameOS output, or audible-playback evidence.
- **Version:** v0.1.22 is reserved for successful-decrypt diagnostics; next package is v0.1.23. Keep physical console and boot plugin configuration unchanged; game-audio coexistence is a separate feasibility gate.
