# Active work

- **Current task:** Use v0.1.26 phase events to identify why RPCS3 does not answer the synthetic ANNOUNCE, then reach SETUP/RECORD/RTP.
- **Latest deployed emulator artifact:** AIRPLAY02 v0.1.25, emulator-only `RTSP_PORT=5001`; package SHA-256 `420971af3601d0a9813d4853f389f7e4ceb1c9f4a6077e27fcc3b81b292cbcd4`. Production default remains 5000.
- **Verified:** v0.1.25 obtains a route, opens RTSP/RTP sockets, completes the tone code path, answers OPTIONS 200, and logs `RTSP_ANNOUNCE` from a synthetic sender. The sender gets no ANNOUNCE response within three seconds. mDNS still fails at stage 6/error 125.
- **Source changes:** Diagnostic callback now marks SDP parse and RSA key unwrap boundaries. `raop_audio_pipeline_test --remote` will send OPTIONS/ANNOUNCE/SETUP/RECORD and 128 encrypted RTP fixture packets. Host/sanitizer suite and PPU components pass; v0.1.26 package/runtime replay pending.
- **Still unverified:** No iPhone session, RTP audio received by the running app, ALAC decode, audible output, visible pixels, controller START exit, or physical PS3 result. Desktop remains locked. Physical console and boot plugins unchanged.
- **Version:** v0.1.26 is next and package defaults now use it. Keep game audio mixing as a separate Cobra feasibility gate.
