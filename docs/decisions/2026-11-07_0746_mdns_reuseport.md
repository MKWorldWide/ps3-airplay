# Request best-effort mDNS port sharing

**Timestamp:** 2026-11-07 07:46 EDT

## Scope of the pass

The RAOP mDNS service now attempts SO_REUSEPORT after SO_REUSEADDR when the target headers define the option. The request is best-effort, so unsupported socket stacks continue to the normal bind path. Build and package defaults advance to v0.1.14.

## Why this pass was needed

v0.1.12 isolated RPCS3's mDNS bind failure to host ownership of UDP 5353. A temporary host socket probe showed SO_REUSEADDR failed but SO_REUSEPORT allowed a second wildcard bind and receipt of multicast responses while the system resolver stayed active.

## Files changed

- src/discovery/raop_mdns_service.c
- prototype/standalone-receiver/Makefile
- scripts/build-lima-standalone-receiver.sh
- scripts/package-lima-standalone-receiver.sh
- docs/BUILD_PIPELINE.md
- docs/TEST_MATRIX.md
- docs/lessons-learned.md
- docs/memory.md
- docs/scratchpad.md
- docs/decisions/2026-11-07_0746_mdns_reuseport.md

## Architecture / logic changes

No architectural change. The listener requests port sharing before binding UDP 5353. It does not replace discovery, change the mDNS port, or disable the host responder.

## Runtime behavior changes

On platforms defining SO_REUSEPORT, the receiver now asks the socket stack to share the bound UDP port. Failure to set that option is ignored and the normal bind attempt proceeds, preserving compatibility with stacks that do not support it.

## Validation performed

- A temporary host UDP socket with SO_REUSEADDR failed to bind UDP 5353 while the host system mDNS service was active.
- The same probe with SO_REUSEPORT succeeded and received two multicast responses to a standard RAOP service query.
- The updated mDNS sources compiled with warnings-as-errors on the host; the existing DNS-SD packet test passed.
- git diff --check passed.
- No PPU build, PKG generation, RPCS3 run, PS3 transfer, or hardware test was performed in this pass.

## Immediate results

Host socket sharing is feasible without stopping the system responder. The receiver's real advertisement and RPCS3 socket behavior remain unverified.

## Known limitations / follow-ups

Verify whether the PS3 SDK defines SO_REUSEPORT and whether RPCS3 maps it to the host socket option. Then build and run v0.1.14 in RPCS3. Native PS3 discovery remains the decisive target test.

## Risks introduced

Two local responders can receive the same multicast packets and may produce duplicate responses. The option is best-effort and no resident/system configuration is changed. Validate event logs and iPhone discovery before treating it as a fix.

## Decision record

### Decision

Add SO_REUSEPORT as a best-effort socket hint, leaving SO_REUSEADDR and standard UDP 5353 behavior intact.

### Context

The RPCS3 app reached listener setup but its mDNS bind failed with EADDRINUSE. The host has a system mDNS responder on the same wildcard port. The no-reuseport bind probe failed; the reuseport probe succeeded and multicast reception worked.

### Options considered

- Stop or modify the system resolver.
- Change mDNS to a nonstandard port.
- Leave RPCS3 unable to exercise mDNS.
- Attempt best-effort port sharing where supported.

### Why this option was chosen

It preserves standard mDNS behavior, does not change system services, and is a small reversible socket option. Unsupported target stacks continue through the existing path.

### Tradeoffs accepted

The host probe is not an app-level or PS3 test. Port sharing could duplicate multicast responses; runtime validation remains required.

## Next recommended step

Commit the change, rebuild v0.1.14 from that exact source revision in Lima, verify the package, then test in RPCS3 and on the reachable native PS3.
