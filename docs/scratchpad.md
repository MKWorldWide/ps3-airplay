# Active work

- **Current task:** Install/test v0.1.17 display and exit in RPCS3 when its UI is accessible; then install/test v0.1.18 telemetry.
- **Latest package:** AIRPLAY02 v0.1.18 from `8769324`; 3,230,448 bytes, SHA-256 `5548fde98bc08949e4f3a6739a95d5b3dce7dea0458131ec693c01be97766ae0`. Full host suite, PPU app, ELF, and finalized package validation pass.
- **Display:** The v0.1.17 source removes all text overlays. Visual display, stable session, and controller exit remain untested.
- **RPCS3:** v0.1.15 earlier stopped at `NET_IP_TIMEOUT`/`EADDRNOTAVAIL` before listener/audio. GUI confirmed Internet `Disconnected`. v0.1.17 has not been installed or launched; RPCS3 UI access is unavailable.
- **Version:** v0.1.18 package generated; next is 0.1.19. Both canonical build/package scripts now default to 0.1.19.
- **v0.1.18:** Adds five-second PCM ring occupancy, frames queued/consumed, and GameOS block-submission counters. Runtime event behavior remains untested.
- **Next:** Install/launch exact v0.1.17 in RPCS3 when UI access returns; then test exact v0.1.18 package. A Connected route run requires separate action-time confirmation; do not claim volume/runtime behavior before it reaches playback. Physical PS3 remains untouched; no real AirPlay or Spotify stream is verified.
- **Boundary:** RPCS3 is not PS3 hardware. No real RTSP session, RTP audio, ALAC playback, or Spotify audio has been demonstrated. Preserve privacy-safe logs and do not commit local paths or raw RPCS3 output.
