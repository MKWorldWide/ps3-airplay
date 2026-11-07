# Active work

- **Current task:** Run v0.1.24 in RPCS3 when desktop access returns, then continue toward an iPhone RAOP session.
- **Candidate:** v0.1.24 adds 128-packet RTP health checkpoints. Full Lima host/ASan/UBSan suite, PPU compile, ELF, and finalized PKG validation pass. PKG SHA-256 `7638825e0c2f6eb26a19410ba56ffd52ed96a70dc274f691b6bf2d12cf615757`.
- **Host validation:** Full Lima suite passes, including discovery TXT assertion; RTSP/TCP + encrypted RTP/UDP + decrypt + ALAC + PCM ring loopback; 4,096 seeded ALAC mutations plus 256 random inputs under ASan/UBSan; and PPU component compilation.
- **Emulator evidence:** Installed AIRPLAY02 was v0.1.15 at the last accessible snapshot. A direct `--no-gui` v0.1.20 EBOOT attempt did not update the capped app log; classify it as inconclusive. The user reports a visible app/background but also display overlays absent from current source and v0.1.20 ancestry. Exact observed package remains unknown. CUA still reports the desktop locked.
- **Next:** RPCS3 UI remains locked and its current log is v0.1.15; when accessible, install v0.1.23, capture a fresh matching log, inspect display/exit, then test v0.1.24. Host simulation is not iPhone, GameOS output, or audible-playback evidence.
- **Version:** v0.1.23 is bounded RTSP writes; v0.1.24 is RTP health logging; next package is v0.1.25. Keep physical console and boot plugin configuration unchanged; game-audio coexistence is a separate feasibility gate.
