# Active work

- **Current task:** Reach AirPlay discovery and a real RAOP connection after the v0.1.12 RPCS3 diagnosis.
- **Verified:** UI cleanup commit `46fca40` is pushed. v0.1.11 displays the background plus START exit prompt in RPCS3. v0.1.12 passes Lima host tests, ELF/package validation, and launches in RPCS3. It binds TCP 5001 and RTP UDP 6000–6002, then fails mDNS UDP bind stage 4 with errno 112 (`EADDRINUSE`). Host UDP 5353 is occupied; wildcard bind probe confirms the collision.
- **Current change:** Privacy-safe mDNS stage/error reporting is implemented. v0.1.12 manifest records base revision `46fca40`; the report documents that the diagnostic sources were uncommitted at build time. The exact artifact hash is recorded.
- **Next:** Keep Lima host networking intact. Use a separately authorized native PS3 run to validate whether UDP 5353 binds, then test iPhone discovery, RTSP, RTP, decode, and output. RPCS3 is stopped and its prior Internet setting is restored.
- **Boundary:** No physical PS3 writes or boot plugin changes. RPCS3 evidence is not hardware evidence; do not claim AirPlay playback until a real session and audible streamed audio are confirmed.
