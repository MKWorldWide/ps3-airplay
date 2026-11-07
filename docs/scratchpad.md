# Active work

- **Current task:** Test committed mDNS diagnostics v0.1.13 in RPCS3 and on the native PS3, then follow the earliest failed AirPlay stage.
- **Verified:** UI cleanup commit `46fca40` is pushed. v0.1.11 displays the background plus START exit prompt in RPCS3. v0.1.12 passes Lima host tests, ELF/package validation, and launches in RPCS3. It binds TCP 5001 and RTP UDP 6000–6002, then fails mDNS UDP bind stage 4 with errno 112 (`EADDRINUSE`). Repeated with Lima stopped; the host system mDNS responder still owns UDP 5353 and the bind fails.
- **Current change:** v0.1.13 was built from pushed source revision `70fa6654bd2d092a5f674dd526c05a2e5ccd4703` and passed the host/PPU/package checks. PKG hash: `f7d076e32520a552349ebb973894409d66fd429ff255d0919c66401fabe9f1e9`.
- **Next:** Install/run v0.1.13 in RPCS3 when the desktop is available, then stage/install on the native PS3 when it is reachable. Capture the fresh log and test UDP 5353, iPhone discovery, RTSP, RTP, decode, and output in order.
- **Boundary:** No physical PS3 writes or boot plugin changes. RPCS3 evidence is not hardware evidence; do not claim AirPlay playback until a real session and audible streamed audio are confirmed.
- **Blockers:** Interactive RPCS3 testing is pending; the PS3 did not answer the read-only reachability check at its last known address. Lima remains Running.
