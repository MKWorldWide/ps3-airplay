# Active work

- **Current task:** Advance the standalone RAOP receiver toward end-to-end audio. v0.1.15 packages the bounded RTSP FLUSH fix.
- **Validated host work:** Full Lima crypto/protocol/codec/audio suite and standalone PPU build passed. Package v0.1.15 validated; 3,225,648 bytes, SHA-256 `230a2df0db8a7e561f3b048872c1a714ed0eb58f4b741587ee961f71834ffa59`.
- **RPCS3:** v0.1.15 installed and booted. Fresh versioned app log reached SCREEN_READY and CONTROLLER_READY, then NET_IP_TIMEOUT and APP_STOP result 1. RPCS3 logged EADDRNOTAVAIL; no receiver sockets, mDNS, audio, or FLUSH event was reached. Saved emulator Internet setting is Disconnected.
- **Current version:** v0.1.15 is built and tested at the package level; v0.1.16 is next. Both build/package scripts now default to 0.1.16.
- **Next:** Enable RPCS3 guest connectivity for one authorized test, restore it afterward, then use the first newly reached startup stage to guide further development. Physical PS3 remains unchanged; require fresh logs before a controlled iPhone attempt.
- **Boundary:** RPCS3 is not PS3 hardware. No real RTSP session, RTP audio, ALAC playback, or Spotify audio has been demonstrated. Preserve privacy-safe logs and do not commit local paths or raw RPCS3 output.
