# Active work

- **Current task:** Continue toward an iPhone RAOP session; validate and run v0.1.21.
- **Candidate:** v0.1.21 corrects stale mDNS RAOP `vs=0.1.2` to server version `105.1`, matching the RTSP `AirTunes/105.1` header. Full-app package has not yet been built.
- **Host validation:** Full Lima suite passes, including discovery TXT assertion; RTSP/TCP + encrypted RTP/UDP + decrypt + ALAC + PCM ring loopback; 4,096 seeded ALAC mutations plus 256 random inputs under ASan/UBSan; and PPU component compilation.
- **Emulator evidence:** Installed AIRPLAY02 was v0.1.15 at the last accessible snapshot. A direct `--no-gui` v0.1.20 EBOOT attempt did not update the capped app log; classify it as inconclusive. The user reports a visible app/background but also display overlays absent from current source and v0.1.20 ancestry. Exact observed package remains unknown. CUA still reports the desktop locked.
- **Next:** Build/validate v0.1.21 in Lima. RPCS3 UI still reports locked; when accessible, install the exact package, capture a fresh matching log, inspect display/START exit, then follow the earliest receiver failure. Host simulation is not iPhone, GameOS output, or audible-playback evidence.
- **Version:** v0.1.21 is reserved for this focused discovery metadata fix. Keep physical console and boot plugin configuration unchanged; game-audio coexistence is a separate feasibility gate.
