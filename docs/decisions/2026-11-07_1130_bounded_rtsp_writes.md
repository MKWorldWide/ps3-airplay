# Bound RTSP response writes

**Timestamp (EDT):** 2026-11-07 11:30

## Scope of the pass

Added a two-second send timeout to accepted RTSP clients, a two-second overall response deadline across partial writes, privacy-safe timeout diagnostics, and a socket-level integration assertion. Advanced the next package version to v0.1.23 and updated architecture, risks, roadmap, tests, and working memory.

## Why this pass was needed

The receiver bounded incomplete request reads, but `send_all` used blocking writes with no deadline. A client that stopped reading could stall the application polling loop and delay network, audio, or controller work.

## Files changed

- `src/protocol/raop_receiver.c`
- `tests/protocol/raop_receiver_test.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-crypto.sh`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/ARCHITECTURE.md`
- `docs/RISKS.md`
- `docs/VERSIONING.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_1130_bounded_rtsp_writes.md`

## Architecture / logic changes

On accept, the receiver applies `SO_SNDTIMEO` of two seconds to the client socket. Response writes also check elapsed monotonic time against a two-second total deadline between `send()` calls. Failed timeout configuration closes that client and logs `RTSP_SEND_TIMEOUT_SETUP_FAILED`; timeout errors are recorded as `RTSP_SEND_TIMEOUT` separately from other `RTSP_SEND_FAIL` errors. Listener and other receiver services remain active.

## Runtime behavior changes

A stalled RTSP response can no longer wait forever. A client whose reply cannot be sent within the bounded interval is disconnected, allowing the main receiver loop to continue. Normal small responses are unchanged.

## Validation performed

- Created a Lima source snapshot and ran `scripts/build-lima-crypto.sh` with `RAOP_VERBOSE_LOGGING=1` for the receiver socket test and the existing PS3DK v0.27.0 host/PPU toolchain.
- PASS: crypto, session, SDP, mDNS, persistent RTSP/UDP loopback, encrypted RTP-to-PCM path, ALAC bounds and ASan/UBSan mutation corpus, capped logging, FFmpeg reference decode, PCM ring/resampling, gain/mute/ramp, and diagnostic tone tests.
- PASS: `raop_receiver_test` observes `SO_SNDTIMEO` on the accepted client, shrinks socket buffers, withholds response reads, observes `RTSP_SEND_TIMEOUT` in the capped log, and confirms disconnect within four seconds.
- PASS: PPU object compilation of the updated receiver.
- The test runs in the host environment; it verifies the timeout/deadline behavior under simulated backpressure but does not establish PS3 or RPCS3 socket semantics.
- RPCS3 remains locked and the local app-data log is v0.1.15. No v0.1.23 package/runtime test or physical-console operation occurred.

## Immediate results

Host receive-path tests and PPU receiver compilation pass. The response-writing path has both a kernel-enforced per-send bound and an application-level total deadline. Full standalone ELF/PKG validation remains pending.

## Known limitations / follow-ups

- Complete v0.1.23 standalone build/package validation.
- Install and run the exact candidate in RPCS3 when the desktop is accessible, then inspect the fresh matching log.
- Do not claim iPhone interoperability or audible output from host tests.

## Risks introduced

- A platform-specific socket stack that rejects `SO_SNDTIMEO` will refuse RTSP clients, but the receiver stays running and records the setup failure. Confirm this option on RPCS3 and PS3 before relying on it for production playback.
- A sender with a pathologically slow but live response read may be disconnected at the two-second bound. This is preferable to indefinitely stalling the single receiver loop; the threshold can be adjusted only from measured behavior.

## Decision record (embedded)

### Use per-send timeout and overall response deadline

- **Decision:** Configure a two-second socket send timeout and enforce a two-second total deadline for one response.
- **Context:** A per-call timeout alone would still allow many partial writes to extend indefinitely; a total deadline alone could leave one blocked `send()` stuck until kernel TCP timeout.
- **Options considered:** Keep blocking writes; use only `SO_SNDTIMEO`; make sockets nonblocking and add a write-select loop; or use both the socket timeout and total elapsed-time check.
- **Why chosen:** The accepted socket gets a kernel bound and the response transaction gets a total bound with a small change to the existing send loop. PS3DK v0.27.0 declares `SO_SNDTIMEO`, PPU compilation succeeds, and a slow-reader socket test validates the bounded disconnect path.
- **Tradeoffs accepted:** Platform runtime support still needs RPCS3/PS3 confirmation, and valid extremely slow clients may be dropped.

### Advance the package patch version to 0.1.23

- **Decision:** Use v0.1.23 for the next AIRPLAY02 candidate.
- **Context:** This is a distinct runtime change and the user requires each package fix to receive its own patch version.
- **Options considered:** Reuse v0.1.22 or advance one patch number.
- **Why chosen:** The timeout behavior must be identifiable independently in future logs and artifacts.
- **Tradeoffs accepted:** Candidate package and runtime evidence remain pending.

## Next recommended step

Run the slow-reader integration test, then commit/push this pass, build and validate AIRPLAY02 v0.1.23 in Lima, and test it in RPCS3 when accessible.
