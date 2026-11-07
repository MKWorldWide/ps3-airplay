# Active work

- **Current task:** Validate a best-effort SO_REUSEPORT mDNS bind fix in v0.1.14, then test RPCS3 and native discovery.
- **Verified:** UI cleanup commit `46fca40` is pushed. v0.1.11 displays the background plus START exit prompt in RPCS3. v0.1.12 passes Lima host tests, ELF/package validation, and launches in RPCS3. It binds TCP 5001 and RTP UDP 6000–6002, then fails mDNS UDP bind stage 4 with errno 112 (`EADDRINUSE`). Repeated with Lima stopped; the host system mDNS responder still owns UDP 5353 and the bind fails.
- **Current change:** Source and package defaults advanced to v0.1.14. A non-mutating host socket probe confirms SO_REUSEPORT can share UDP 5353 and receive multicast; the RAOP mDNS packet host test passes. Full PPU/package and RPCS3 tests remain pending.
- **Next:** Commit the focused fix, build v0.1.14 from the pushed source in Lima, then install/run in RPCS3 when the interactive desktop is available. Retest native PS3 when reachable.
- **Boundary:** No physical PS3 writes or boot plugin changes. RPCS3 evidence is not hardware evidence; do not claim AirPlay playback until a real session and audible streamed audio are confirmed.
- **Blockers:** Interactive RPCS3 testing is pending; the PS3 did not answer the read-only reachability check at its last known address. Lima remains Running.
