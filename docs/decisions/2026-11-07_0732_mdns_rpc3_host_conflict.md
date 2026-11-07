# RPCS3 mDNS bind failure is host-specific

**Timestamp:** 2026-11-07 07:32 EDT

## Scope of the pass

Documented a controlled RPCS3 retry with the Lima VM stopped and the emulator network connected. Updated the mDNS risk, test matrix, build notes, project memory, scratchpad, and hardware/emulator report. No source or package was changed.

## Why this pass was needed

v0.1.12 exposed an mDNS bind-stage failure, but the initial run had both RPCS3 and Lima sharing the host. A second run was needed to distinguish the Lima proxy from the operating system's standard mDNS listener.

## Files changed

- `docs/BUILD_PIPELINE.md`
- `docs/RISKS.md`
- `docs/TEST_MATRIX.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/reports/2026-11-07_v0111_v0112_rpcs3.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_0732_mdns_rpc3_host_conflict.md`

## Architecture / logic changes

None. The documentation now attributes the RPCS3 mDNS bind failure to the host's system mDNS responder, which still held wildcard UDP 5353 after Lima was stopped. Lima's host-side proxy remains an additional conflict when the VM is running.

## Runtime behavior changes

None. No operating system service or application configuration was changed as part of the diagnostic. RPCS3 was stopped through its UI, its prior disconnected Internet setting was restored, and Lima was restarted.

## Validation performed

- Repeated the v0.1.12 launch in RPCS3 with Internet Connected while Lima was stopped.
- Confirmed RTSP 5001 and RTP UDP 6000–6002 readiness followed by mDNS stage 4 / errno 112.
- Confirmed a host wildcard UDP 5353 bind probe failed with `EADDRINUSE`.
- Inspected the UDP table and identified the system mDNS responder on wildcard IPv4 and IPv6 UDP 5353.
- Verified Lima returned to Running and has at least 8 GiB free in the guest.
- `git diff --check` passed before this decision record was added.

## Immediate results

The RPCS3 mDNS failure is attributable to host UDP 5353 ownership. This is not evidence that the PS3's UDP 5353 is unavailable. No iPhone discovery or RAOP audio session was established.

## Known limitations / follow-ups

The next decisive test is a native PS3 run with the receiver bound to UDP 5353, followed by iPhone discovery and one connection attempt. The app's screen is visibly simplified in RPCS3; physical HDMI output and clean START exit remain separate hardware checks.

## Risks introduced

No runtime risk introduced. Do not stop or alter the host's system mDNS responder to accommodate an emulator test.

## Decision record

### Decision

Treat the RPCS3 UDP 5353 failure as a host limitation and use a native PS3 run to validate target behavior.

### Context

The exact bind-stage failure persisted with Lima stopped, and the host UDP table identified the system mDNS responder as the wildcard listener.

### Options considered

- Change receiver mDNS behavior to bypass the host's standard port.
- Stop or modify the system mDNS responder.
- Keep RPCS3's failure classified as host-specific and test on the intended PS3.

### Why this option was chosen

AirPlay DNS-SD conventionally uses UDP 5353, while stopping the host resolver is disruptive and would not validate the target console. The native PS3 has a different network stack and port ownership.

### Tradeoffs accepted

RPCS3 cannot currently validate mDNS on this host. Discovery must be established with the actual console or a separate controlled host configuration.

## Next recommended step

Build a committed-source v0.1.13 package, validate its exact contents and checksum, then use the already-authorized installation path for one native PS3 launch. Capture the fresh log and confirm visible screen and clean START exit before attempting iPhone discovery.
