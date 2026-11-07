# v0.1.14 mDNS port-sharing build

**Date:** 2026-11-07
**Source revision:** 6d9f28d0f079c445b96203695ea1912e8deed887
**Package:** AIRPLAY02 v0.1.14, RTSP TCP 5000
**PKG size:** 3,225,008 bytes
**PKG SHA-256:** 5994cf4828976245f530b1c90d180fd4f0e80abddcfd0d09d3780c351ad006c1
**ELF SHA-256:** 856ff5a85bd01f8b128027de225221729bc8dcb19c184ef0c6e1b392cfbe3d5b

## Scope

Added a best-effort SO_REUSEPORT request to the RAOP mDNS socket and built the next AIRPLAY02 candidate from the pushed source revision. The package remains on standard UDP 5353 and TCP 5000.

## Validation

- Host socket probe: without SO_REUSEPORT, wildcard UDP 5353 bind failed while the system mDNS responder was active. With SO_REUSEPORT, bind succeeded and the socket received multicast responses to a standard RAOP service query.
- Native host C compile with warnings-as-errors and the RAOP mDNS packet test passed.
- PS3DK headers define SO_REUSEPORT as 0x0200. The PPU build included the guarded call.
- RPCS3 source maps the PS3 SO_REUSEPORT option to the host SO_REUSEPORT on non-Windows builds: https://github.com/RPCS3/rpcs3/blob/master/rpcs3/Emu/Cell/lv2/sys_net/lv2_socket_native.cpp. This is source-level support evidence, not a runtime result for this package.
- The host suite command passed crypto/challenge, session crypto/state, SDP, and mDNS tests, then stalled at the RTSP/RTP loopback. The untraced test exceeded a 90-second bound. Running it under bounded strace with LeakSanitizer disabled passed. RTSP listener/log, ALAC decode, PCM ring/resampling, and diagnostic tone tests also passed individually. The timing-sensitive loopback cause remains unknown.
- scripts/build-lima-standalone-receiver.sh passed CellOS PPC64 ELF validation.
- scripts/package-lima-standalone-receiver.sh passed NPDRM generation, extraction, APP_VER/title ID, content ID, asset equality, relative-path, and finalized-envelope checks.
- The package was copied from the Lima guest to the ignored local build area and the copied SHA-256 matched the guest output.

## Runtime and hardware status

RPCS3 install/launch was not completed. The PS3 did not answer the read-only reachability check, so no console transfer or setting change was made.

| Acceptance area | Result |
|---|---|
| mDNS host socket sharing probe | PASS |
| Host mDNS packet serialization | PASS |
| PPU ELF and NPDRM package validation | PASS |
| RPCS3 launch and mDNS advertisement | NOT TESTED |
| Native PS3 launch and mDNS advertisement | NOT TESTED |
| iPhone discovery / RTSP / RTP | NOT TESTED |
| ALAC output / audible streaming | NOT TESTED |

## Next action

Install and run this exact artifact in RPCS3 when the interactive UI is available; check whether mDNS bind/advertisement now succeeds and capture a fresh log. Then test the same package on the native PS3 when reachable. Do not infer AirPlay discovery from the host socket probe.
