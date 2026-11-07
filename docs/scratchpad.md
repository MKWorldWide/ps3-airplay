# Active work

- **Current task:** Install and visually validate v0.1.17 in RPCS3 when its UI is accessible, then continue standalone RAOP toward end-to-end audio.
- **Current package:** AIRPLAY02 v0.1.17 from `4a06c2a`; 3,230,256 bytes, SHA-256 `2095046e20716d9f2768c847691515f1d1e922e5fbd96e15829f92c45816cf23`. Full host suite, PPU app, ELF, and finalized package validation pass.
- **Display:** The v0.1.17 source removes all text overlays. Visual display, stable session, and controller exit remain untested.
- **RPCS3:** v0.1.15 earlier stopped at `NET_IP_TIMEOUT`/`EADDRNOTAVAIL` before listener/audio. GUI confirmed Internet `Disconnected`. v0.1.17 has not been installed or launched; RPCS3 UI access is unavailable.
- **Version:** v0.1.17 package generated; next is 0.1.18. Both canonical build/package scripts now default to 0.1.18.
- **Next:** Install/launch the exact v0.1.17 package in RPCS3 and inspect the fresh versioned log. A Connected route run requires separate action-time confirmation; do not claim volume/runtime behavior before it reaches playback. Physical PS3 remains untouched; no real AirPlay or Spotify stream is verified.
- **Boundary:** RPCS3 is not PS3 hardware. No real RTSP session, RTP audio, ALAC playback, or Spotify audio has been demonstrated. Preserve privacy-safe logs and do not commit local paths or raw RPCS3 output.
