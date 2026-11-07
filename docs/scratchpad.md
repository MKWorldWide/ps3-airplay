# Active work

- **Current task:** Advance standalone RAOP toward end-to-end audio; v0.1.16 implements RAOP volume/mute and PCM gain ramp.
- **Validated:** Full Lima crypto/protocol/codec/audio suite passes with volume parser/session and PCM gain tests. PPU objects compile. The standalone v0.1.16 working-tree ELF links and passes CellOS PPC64 validation (2,780,096 bytes; SHA-256 `968f9c3238e7c8d4479c0191452f378f4705d7d6585c68364cfcd7d792682f2a`). No canonical package has been generated yet.
- **RPCS3:** v0.1.15 installed/booted but stops at `NET_IP_TIMEOUT`/`EADDRNOTAVAIL` before listener/audio startup. RPCS3 GUI confirms Internet `Disconnected`; no setting was changed. Await action-time confirmation for one temporary Connected launch followed by restoration.
- **Version:** v0.1.15 is the latest validated package; v0.1.16 is source-only and next to build.
- **Next:** Commit this source/test pass after privacy review; run the canonical build/package scripts from that commit and validate v0.1.16. Then use an authorized RPCS3 route test and inspect the first reached stage. Physical PS3 remains untouched; no real AirPlay or Spotify stream is verified.
- **Boundary:** RPCS3 is not PS3 hardware. No real RTSP session, RTP audio, ALAC playback, or Spotify audio has been demonstrated. Preserve privacy-safe logs and do not commit local paths or raw RPCS3 output.
