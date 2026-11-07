# Active work

- **Current task:** Continue toward an iPhone RAOP session; reconcile the observed display with the installed package, then resume emulator diagnosis.
- **Candidate:** AIRPLAY02 v0.1.20 from `8a4e641d114e1a613421b9ffb51f8615ad851827`; PKG SHA-256 `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`, preserved in ignored `work/builds/0.1.20/`.
- **Host validation:** Lima suite passes, including a new live TCP/UDP socket loopback from fragmented RTSP ANNOUNCE through encrypted RTP, decrypt, ALAC reference PCM, and PCM ring enqueue. 4,096 seeded ALAC mutations plus 256 random inputs pass ASan/UBSan; PPU components compile.
- **Emulator evidence:** Installed AIRPLAY02 was v0.1.15 at the last accessible snapshot. A direct `--no-gui` v0.1.20 EBOOT attempt did not update the capped app log; classify it as inconclusive. The user reports a visible app/background but also display overlays absent from current source and v0.1.20 ancestry. Exact observed package remains unknown. CUA still reports the desktop locked.
- **Next:** When desktop access is restored, install and launch v0.1.20 in RPCS3 and save a fresh version-matched log; inspect displayed pixels and START exit. Then follow the earliest failing receiver layer. Host simulation is not iPhone, GameOS output, or audible-playback evidence.
- **Version:** Next package is 0.1.21 after an application-level fix. Keep physical console and boot plugin configuration unchanged; game-audio coexistence is a separate feasibility gate.
