# Report the mDNS startup failure stage

**Timestamp:** 2026-11-07 07:11 America/New_York

## Scope of the pass

Add privacy-safe mDNS startup diagnostics covering argument validation, socket creation, multicast TTL, UDP bind, group join, outgoing interface selection, and initial announcement. Build and validate v0.1.12, then advance package defaults to v0.1.13.

## Why this pass was needed

v0.1.10 and v0.1.11 reached RTSP and RTP listener readiness in RPCS3, then reported only `MDNS_FAILED`. The logs could not distinguish a bind conflict from multicast or send failure.

## Files changed

- `src/discovery/raop_mdns_service.h`
- `src/discovery/raop_mdns_service.c`
- `prototype/standalone-receiver/source/main.c`
- `tests/discovery/raop_mdns_test.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/BUILD_PIPELINE.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v0111_v0112_rpcs3.md`
- `docs/decisions/2026-11-07_0711_mdns_failure_diagnostics.md`

## Architecture / logic changes

The mDNS open operation preserves an enum failure stage and numeric errno after closing partial resources. The standalone log stores the stage in `value` and errno in `bytes`; no address, MAC, hostname, or service label is emitted. If the group join already succeeded, error cleanup drops membership before closing the socket. The normal success path is unchanged.

## Runtime behavior changes

`MDNS_FAILED` now identifies the stage and numeric platform error. Stage values are defined in the public header: 1 argument, 2 socket, 3 TTL, 4 bind, 5 join, 6 interface, and 7 announcement. Socket reuse remains best-effort as in the prior implementation. v0.1.12 is the tested diagnostic build; package defaults now advance to v0.1.13.

## Validation performed

- Full Lima host test suite passed, including the mDNS argument-stage assertion.
- PPU build, ELF validation, package extraction/SFO/content ID/assets, and finalized envelope checks passed for v0.1.12.
- RPCS3 v0.1.12 reached RTSP 5001 and RTP 6000–6002, then logged `MDNS_FAILED value=4 bytes=112`. Stage 4 is UDP bind; PS3DK errno 112 is `EADDRINUSE`.
- Host UDP 5353 listener observed; a temporary wildcard bind probe failed with `EADDRINUSE` even with `SO_REUSEADDR`.
- RPCS3 was stopped cleanly and its prior Internet setting was restored.

## Immediate results

v0.1.12 isolated the RPCS3 failure to UDP 5353 bind with `EADDRINUSE`, consistent with the occupied host port. No claim is made about native PS3 discovery or an AirPlay session.

## Known limitations / follow-ups

- The host negative test covers deterministic argument validation; it does not simulate each platform socket failure.
- Validate UDP 5353 bind behavior on a separately authorized native PS3 run; RPCS3 is blocked by the local host port.
- A successful mDNS socket setup still does not prove multicast advertisement reaches the iPhone.

## Risks introduced

- The mDNS stage is now measured in RPCS3, but a successful socket setup still does not prove the multicast announcement reaches an iPhone.
- The value/bytes fields carry enumerated diagnostics rather than existing success counters for the `MDNS_FAILED` event only.

## Decision record (embedded)

### Decision: Preserve setup stage and errno through failure cleanup

- **Context:** A generic boolean result hid the point where discovery initialization stopped.
- **Options considered:** Emit stage-specific event names; expose errno in an unbounded string; or keep one allowlisted event with bounded numeric fields.
- **Chosen:** Keep `MDNS_FAILED` and encode stage/errno as numeric fields, preserving the existing capped log format and privacy rules. Preserve best-effort `SO_REUSEADDR` behavior.
- **Tradeoffs:** Readers must map numeric stage values through the header or report, but the format remains small and does not risk logging network identifiers.

### Decision: Drop multicast membership during partial failure

- **Context:** Failures can occur after group join but before interface selection or announcement.
- **Options considered:** Close the socket only; or explicitly drop membership before close.
- **Chosen:** Drop membership when joined, then close the socket.
- **Tradeoffs:** Cleanup issues one best-effort socket option call; the primary failure code remains preserved.

## Next recommended step

Keep Lima networking intact. Establish mDNS bind behavior on a separately authorized native PS3 run; then test iPhone discovery and follow the session through RTP and audio output.
