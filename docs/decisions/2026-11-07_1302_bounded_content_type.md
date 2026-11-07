# Bound the ANNOUNCE Content-Type Scan

**Timestamp:** 2026-11-07 13:02 EDT (America/New_York)

## Scope of the pass

Recorded v0.1.27 RPCS3 validation evidence and changed ANNOUNCE Content-Type length handling to a bounded scan with explicit start/result events. Added callback assertions and advanced package defaults to v0.1.28.

## Why this pass was needed

v0.1.27 RPCS3 logs showed ANNOUNCE SDP parsing succeeded and the offer contained encryption type 1, an RSA key, and an AES IV. No validation-completion, RSA-unwrapping, or response event followed. The exact stopped expression was unknown.

## Files changed

- `src/protocol/raop_session.c`
- `tests/protocol/raop_session_crypto_test.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/ARCHITECTURE.md`
- `docs/BUILD_PIPELINE.md`
- `docs/RISKS.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_rpcs3_announce_validation_v027.md`
- `docs/decisions/2026-11-07_1302_bounded_content_type.md`

## Architecture / logic changes

ANNOUNCE now computes Content-Type length with a capacity-bounded helper and compares the exact bounded slice to `application/sdp`. It emits privacy-safe check-start/check-result events. The parsed field is fixed-capacity and parser-initialized; the change avoids unbounded scanning and makes this condition visible in the next runtime log. Package defaults advance to 0.1.28; production RTSP port remains 5000.

## Runtime behavior changes

No wire response or successful session behavior is intentionally changed. v0.1.27 currently starts under RPCS3, answers OPTIONS, parses SDP and encrypted offer fields, then does not complete ANNOUNCE. v0.1.28 has not yet been built or run.

## Validation performed

- Replayed v0.1.27 from the exact pushed source revision. RPCS3 reached startup, network/listener readiness, OPTIONS 200, SDP parse success, and offer-field diagnostics. The synthetic client timed out on ANNOUNCE.
- Built and ran the candidate v0.1.28 full Lima host suite under ASan/UBSan; all tests passed, including the new Content-Type event assertion.
- PPU component compilation/type checks passed for receiver/protocol and codec/audio objects.
- v0.1.27 package validation passed; v0.1.28 package build and runtime replay are pending.

## Immediate results

- ANNOUNCE failure is after successful offer parsing and before validation-completion logging.
- The old code used `strlen` on a parser-owned fixed-size field. The new code bounds that scan, but no causal claim is made until RPCS3 replay.
- No RTP/audio packets reached the receiver. No physical PS3 or boot-plugin state changed.

## Known limitations / follow-ups

- Commit/push this pass, build v0.1.28 from that exact revision with emulator RTSP port 5001, and replay the synthetic sender.
- Inspect Content-Type check start/result, validation completion, RSA unwrap, and response build events.
- If the client proceeds, inspect SETUP, RECORD, RTP/decrypt, ALAC, and PCM telemetry.
- Retry visual/controller checks only when RPCS3 desktop is accessible.

## Risks introduced

The bounded helper and extra log events add a small amount of code and capped-log volume. This is an emulator diagnostic package using RTSP 5001; it is not a physical deployment candidate. The protocol response behavior should remain unchanged for correctly parsed headers.

## Decision record (embedded)

### Decision: Bound the fixed-capacity Content-Type scan

- **Decision:** Replace the `strlen` use with an explicit capacity-bounded scan and log its result.
- **Context:** v0.1.27 logs stopped after offer fields and before validation completion; the exact expression was not established.
- **Options considered:** Change protocol semantics, alter RSA behavior, or add a bounded check with diagnostics.
- **Why this option was chosen:** It improves memory-safety defensively and directly tests one expression on the observed path without speculating about RAOP interoperability or crypto.
- **Tradeoffs accepted:** Another patch build/replay is required; the suspected relationship to the emulator stall may prove coincidental.

### Decision: Advance to package version 0.1.28

- **Decision:** Increment the patch version for this runtime diagnostic and code change.
- **Context:** Distinct artifacts are needed to map fresh logs to exact source behavior.
- **Options considered:** Reuse v0.1.27 or consume a new patch number.
- **Why this option was chosen:** A new number keeps app data, logs, and rollback artifacts attributable.
- **Tradeoffs accepted:** The version is consumed when packaged, regardless of whether the hypothesis is confirmed.

## Next recommended step

Commit and push this bounded validation pass, build v0.1.28 from the exact commit, and replay the synthetic OPTIONS/ANNOUNCE probe in RPCS3.
