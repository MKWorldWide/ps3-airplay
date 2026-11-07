# Active work

- **Current task:** Build and visually validate v0.1.17, then continue standalone RAOP toward end-to-end audio.
- **Current package:** AIRPLAY02 v0.1.16 from `0a204b0`; 3,231,088 bytes, SHA-256 `3c69bff7ab626c433728ca131bc04ab0adbc117b8845d88c748e1dad1968bd3c`. Host suite, PPU app, ELF, and finalized package validation pass.
- **Display:** Source removes the remaining text overlay; v0.1.16 remains the latest validated package. v0.1.17 has not been built or visually tested.
- **RPCS3:** v0.1.15 earlier stopped at `NET_IP_TIMEOUT`/`EADDRNOTAVAIL` before listener/audio. GUI confirmed Internet `Disconnected`. No v0.1.16 runtime evidence yet.
- **Version:** v0.1.16 package generated; next is 0.1.17. Both canonical build/package scripts now default to 0.1.17.
- **Next:** Commit the display cleanup, build/package v0.1.17 from that pushed revision, validate it, then install/launch in RPCS3 when the UI is accessible. A Connected route run requires separate action-time confirmation; do not claim volume/runtime behavior before it reaches playback. Physical PS3 remains untouched; no real AirPlay or Spotify stream is verified.
- **Boundary:** RPCS3 is not PS3 hardware. No real RTSP session, RTP audio, ALAC playback, or Spotify audio has been demonstrated. Preserve privacy-safe logs and do not commit local paths or raw RPCS3 output.
