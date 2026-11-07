# Active work

- **Current task:** Build v0.1.23 and continue toward an iPhone RAOP session.
- **Candidate:** v0.1.23 bounds each accepted RTSP client's socket send timeout and overall response duration to two seconds. Host integration checks the socket timeout; full Lima host/sanitizer and PPU component checks pass.
- **Host validation:** Full Lima suite passes, including discovery TXT assertion; RTSP/TCP + encrypted RTP/UDP + decrypt + ALAC + PCM ring loopback; 4,096 seeded ALAC mutations plus 256 random inputs under ASan/UBSan; and PPU component compilation.
- **Emulator evidence:** Installed AIRPLAY02 was v0.1.15 at the last accessible snapshot. A direct `--no-gui` v0.1.20 EBOOT attempt did not update the capped app log; classify it as inconclusive. The user reports a visible app/background but also display overlays absent from current source and v0.1.20 ancestry. Exact observed package remains unknown. CUA still reports the desktop locked.
- **Next:** Finish v0.1.23 ELF/PKG validation. RPCS3 UI remains locked and its current log is v0.1.15; when accessible, install the exact package, capture a fresh matching log, inspect display/START exit, then follow the earliest receiver failure. Host simulation is not iPhone, GameOS output, or audible-playback evidence.
- **Version:** v0.1.23 is reserved for bounded RTSP writes. Keep physical console and boot plugin configuration unchanged; game-audio coexistence is a separate feasibility gate.
