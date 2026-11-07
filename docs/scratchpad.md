# Active work

- **Current task:** Correct silent diagnostic-event loss, then replay synthetic ANNOUNCE in RPCS3 with v0.1.29.
- **Latest replay:** AIRPLAY02 v0.1.28. Startup, route, listener readiness, OPTIONS 200, SDP parse, validation start, and offer fields logged. Client receives no ANNOUNCE response. `ANNOUNCE_VALIDATE_DONE` is absent. Content-Type/RSA/response events were rejected because event names exceeded 24 characters; their absence is inconclusive.
- **Latest package:** v0.1.28, 3,231,984 bytes, SHA-256 `0e3201647bb83b85c0df4e08dacc780c9daf1468c9d261e321c10be0f8fa9cef`, source `4308bb120d0b86e065481a1175990de47acb4a24`.
- **In progress:** v0.1.29 increases log event-name limit to 48, expands line buffer, and tests persistent logging of a 33-character event. Full Lima ASan/UBSan and PPU tests pass on candidate source.
- **Unverified:** ANNOUNCE response internals, RTP, ALAC in RPCS3, audible output, iPhone compatibility, visible pixels, controller exit, physical PS3. RPCS3 mDNS still fails; desktop locked.
- **Constraints:** Production RTSP 5000; emulator 5001. Preserve boot plugin configuration and game-audio separation.
