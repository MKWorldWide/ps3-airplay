# Active work

- **Current task:** Diagnose RPCS3 routing and advance from listener readiness into an end-to-end RAOP session.
- **Latest artifact:** AIRPLAY02 v0.1.25, built with emulator-only `RTSP_PORT=5001`; package SHA-256 `420971af3601d0a9813d4853f389f7e4ceb1c9f4a6077e27fcc3b81b292cbcd4`. Production default remains 5000.
- **Verified in RPCS3:** Network route acquired; RTSP 5001 and RTP 6000–6002 ready; diagnostic tone completion logged; local OPTIONS returned 200 and the receiver logged accept/status/disconnect. App log and rollback copies are kept under ignored `work/`.
- **Still unverified:** mDNS fails at interface setup (stage 6/error 125); no iPhone discovery/session, RTP audio, ALAC decode, audible tone/music, visible pixels, or controller START exit. Desktop remains locked. Physical PS3 and boot plugins unchanged.
- **Next:** Revisit RPCS3 with desktop unlocked for visual/START checks; separately diagnose its multicast-interface mismatch. Then attempt a controlled ANNOUNCE/SETUP/RECORD session and follow RTP/decrypt/ALAC/PCM events. Keep public docs sanitized and do not overstate host/emulator evidence.
- **Version:** v0.1.25 consumed; next package version v0.1.26. Game audio mixing remains a separate Cobra feasibility gate.
