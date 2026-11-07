# Active work

- **Current task:** Reconcile the new visible-app report with the cleaned screen implementation, then test the validated v0.1.14 SO_REUSEPORT candidate in RPCS3 and follow the earliest AirPlay failure.
- **Verified:** UI cleanup commit `46fca40` is pushed. v0.1.11 displays the background plus START exit prompt in RPCS3. The local RPCS3 installation remains v0.1.12; its available log shows RTSP 5001/RTP 6000–6002 readiness, mDNS bind stage 4/error 112, tone completion, PCM underflows without RTP, and APP_STOP. A current bounded RAOP browse found no expected project label. Prior physical-console FTP/webMAN endpoints did not respond.
- **Current change:** v0.1.14 was built from pushed source 6d9f28d0f079c445b96203695ea1912e8deed887; PPU/ELF/PKG validation passed. PKG hash: 5994cf4828976245f530b1c90d180fd4f0e80abddcfd0d09d3780c351ad006c1. The RTSP/RTP loopback remains untraced-flaky.
- **New user evidence:** App opens and background is visible; user requests removing blue borders and `DISPLAY ACTIVE`. Current source already removes both. Observed package/platform are unknown and must not be conflated with the local RPCS3 v0.1.12 snapshot.
- **Next:** Install/run the exact v0.1.14 PKG in RPCS3, inspect fresh versioned logs, and check whether SO_REUSEPORT clears the bind collision. Then identify the user's observed build before producing a display package. Physical console remains read-only and unreachable.
- **Boundary:** No physical PS3 writes or boot plugin changes. RPCS3 evidence is not hardware evidence; do not claim AirPlay playback until a real session and audible streamed audio are confirmed.
- **Blockers:** Interactive RPCS3 testing is pending; the PS3 did not answer the read-only reachability check at its last known address. Lima remains Running.
