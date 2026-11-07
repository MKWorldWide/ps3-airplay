# Active work

- **Current task:** Isolate RPCS3 host-port conflicts and continue AirPlay receiver testing from `main`.
- **Verified:** v0.1.9 passed host tests and package validation. With RPCS3 disconnected, the app logged a bounded `NET_IP_TIMEOUT`; with RPCS3 network set to Connected, it logged `NET_IP_ROUTE_READY` then `RTSP_BIND_FAIL` value 112. TCP port 5000 was already occupied by a wildcard host listener; UDP 6000–6002 were free. The port conflict is a testable hypothesis, not yet proven as the cause.
- **Current change:** Add a `RTSP_PORT` build input (default 5000) and compile v0.1.10 with port 5001 for RPCS3. mDNS uses the receiver's actual bound port. The physical-console default remains 5000.
- **Next:** Commit/push the change and record; rebuild/test v0.1.10 in RPCS3; inspect fresh sanitized listener/mDNS events and check actual host port binds. Restore RPCS3's previous Disconnected setting after tests.
- **Boundary:** No physical PS3 writes or boot plugin changes. RPCS3 evidence is not hardware evidence; do not claim AirPlay playback until a real session and audible streamed audio are confirmed.
