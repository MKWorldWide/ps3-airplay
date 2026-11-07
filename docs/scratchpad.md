# Active work

- **Current task:** Diagnose the v0.1.8 RPCS3 RTSP bind failure and advance the AIRPLAY02 build pipeline on `main`.
- **Verified:** v0.1.8 installed and reached APP_START in RPCS3, then logged RTSP_BIND_FAIL (112), RECEIVER_OPEN_FAILED, APP_STOP. The RPCS3 network was observed disconnected; causality is unproven. No tone, visible pixels, AirPlay session, RTP, or music was verified.
- **Current change:** Move the existing bounded route-first/NetCtl IPv4 readiness check before receiver socket creation. This ensures a usable route is established before RTSP/RTP binds, while preserving existing address selection behavior. Advance the next unique package candidate to 0.1.9.
- **Next:** Commit/push implementation and decision record on `main`; build and validate v0.1.9 in Lima; install/launch in RPCS3 and collect only sanitized log events. Diagnose whether bind failure persists without changing emulator settings.
- **Boundary:** No physical PS3 writes or boot plugin changes. RPCS3 evidence is not hardware evidence; do not claim AirPlay playback until a real session and audible streamed audio are confirmed.
