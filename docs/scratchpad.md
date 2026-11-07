# Active work

- **Current task:** Continue toward an iPhone RAOP session using fresh emulator evidence and end-to-end host coverage.
- **Candidate:** AIRPLAY02 v0.1.20 from `8a4e641d114e1a613421b9ffb51f8615ad851827`; PKG SHA-256 `bfaa3f960de39daf95c2ab405bd361a031ed92bd35c76a99e667d4ffe8d19e87`, preserved in ignored `work/builds/0.1.20/`.
- **Host validation:** Lima suite passes, including a new live TCP/UDP socket loopback from fragmented RTSP ANNOUNCE through encrypted RTP, decrypt, ALAC reference PCM, and PCM ring enqueue. 4,096 seeded ALAC mutations plus 256 random inputs pass ASan/UBSan; PPU components compile.
- **Emulator evidence:** Installed AIRPLAY02 remains v0.1.15. A direct `--no-gui` v0.1.20 EBOOT attempt did not update the capped app log or emit startup evidence; classify it as inconclusive. The visible RPCS3 desktop is still locked.
- **Next:** Unlock the Mac, install and launch the exact v0.1.20 PKG in RPCS3, save a fresh matching log, then follow the earliest failing receiver layer. Do not promote host simulation to iPhone, GameOS output, or audible-playback evidence.
- **Version:** Next package is 0.1.21 after an application-level fix. Keep physical console and boot plugin configuration unchanged; game-audio coexistence is a separate feasibility gate.
