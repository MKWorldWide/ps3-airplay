# Active work

- **Current task:** Test the validated v0.1.14 SO_REUSEPORT candidate in RPCS3 and on the native PS3, then follow the earliest failed AirPlay stage.
- **Verified:** UI cleanup commit `46fca40` is pushed. v0.1.11 displays the background plus START exit prompt in RPCS3. v0.1.12 passes Lima host tests, ELF/package validation, and launches in RPCS3. It binds TCP 5001 and RTP UDP 6000–6002, then fails mDNS UDP bind stage 4 with errno 112 (`EADDRINUSE`). Repeated with Lima stopped; the host system mDNS responder still owns UDP 5353 and the bind fails.
- **Current change:** v0.1.14 was built from pushed source 6d9f28d0f079c445b96203695ea1912e8deed887; PPU/ELF/PKG validation passed. PKG hash: 5994cf4828976245f530b1c90d180fd4f0e80abddcfd0d09d3780c351ad006c1. The RTSP/RTP loopback remains untraced-flaky.
- **Next:** Install/run this exact package in RPCS3 when the interactive desktop is available. Retest native PS3 discovery when reachable; inspect fresh logs before attempting further AirPlay stages.
- **Boundary:** No physical PS3 writes or boot plugin changes. RPCS3 evidence is not hardware evidence; do not claim AirPlay playback until a real session and audible streamed audio are confirmed.
- **Blockers:** Interactive RPCS3 testing is pending; the PS3 did not answer the read-only reachability check at its last known address. Lima remains Running.
