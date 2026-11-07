# Move network readiness before listener setup

**Timestamp:** 2026-11-07 06:37 EDT

## Scope of the pass

Move the existing bounded local IPv4 readiness check ahead of receiver socket creation. Advance build/package defaults to v0.1.9 and update project state and v0.1.8 RPCS3 evidence.

## Why this pass was needed

v0.1.8 installed and reached application entry in RPCS3, but the RTSP socket bind failed with error 112 before the receiver became ready. RPCS3 networking was observed disconnected, but that observation does not establish cause.

## Files changed

- `prototype/standalone-receiver/source/main.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/BUILD_PIPELINE.md`
- `docs/ROADMAP.md`
- `docs/RISKS.md`
- `docs/TEST_MATRIX.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v018_rpcs3_retest.md`
- `docs/decisions/2026-11-07_0637_network_readiness_order.md`

## Architecture / logic changes

`resolve_local_ipv4()` already tries a UDP route lookup first, polls NetCtl for up to five seconds if necessary, and performs one route fallback. It now runs after NetCtl initialization and before crypto and receiver socket setup. This preserves address selection while establishing readiness earlier.

The build, package, and Makefile defaults move to v0.1.9 because v0.1.8 has already been generated and tested. The version is consumed even though its runtime receiver failed.

## Runtime behavior changes

The app now records route readiness or a bounded NetCtl wait/timeout before opening RTSP/RTP sockets. In a networkless environment it can stop at the explicit readiness boundary instead of first attempting a listener bind. The existing route-first fast path remains unchanged.

## Validation performed

- Verified v0.1.8 package validation/build records and sanitized RPCS3 log event sequence from the previous test pass.
- Confirmed `main` now contains the previously pushed 14-commit development history; removed the obsolete side branch after push.
- Pending for this source change: Lima build/package validation and RPCS3 launch.

## Immediate results

- v0.1.8 package and application startup passed in RPCS3.
- v0.1.8 failed at RTSP bind with value 112 and then stopped.
- No AirPlay session, visible display, tone, RTP, decoded audio, or music was established.

## Known limitations / follow-ups

- Validate v0.1.9 in Lima and RPCS3.
- If no route is available, confirm bounded wait and clean exit from the new sanitized log.
- If a route is available and bind still fails, inspect socket setup and emulator network state without changing emulator settings.
- Physical PS3 display, clean exit, and streamed audio remain unverified.

## Risks introduced

- Moving a bounded wait earlier may delay startup by up to five seconds when route lookup fails. The timeout is finite and the controller is initialized earlier in startup than network initialization.
- A successful address lookup does not prove that TCP/UDP ports can bind or that the iPhone can reach them.

## Decision record (embedded)

### Decision: Resolve network readiness before opening receiver sockets

- **Context:** The app opened the listener before calling an existing route-first, bounded NetCtl/IP resolver.
- **Options considered:** Keep the current order; remove NetCtl waiting; or move the existing readiness function before socket setup.
- **Chosen:** Move the existing function with no behavior expansion.
- **Tradeoffs:** Startup can wait up to five seconds without a route, but the failure boundary becomes explicit and sockets are not attempted before readiness.

### Decision: Advance to package version 0.1.9

- **Context:** v0.1.8 has already been generated and tested.
- **Options considered:** Reuse v0.1.8 or use a distinct version for this fix.
- **Chosen:** Use 0.1.9 to preserve artifact-to-source attribution.
- **Tradeoffs:** Version increments for failed candidates; this avoids overwriting test evidence.

## Next recommended step

Commit this pass on `main`, build and validate AIRPLAY02 v0.1.9 in Lima, then install/launch it in RPCS3 and compare sanitized startup events. Do not modify emulator network settings or physical console files.
