# Allow the Full Diagnostic Event Vocabulary

**Timestamp:** 2026-11-07 13:10 EDT (America/New_York)

## Scope of the pass

Recorded the v0.1.28 RPCS3 replay, corrected prior interpretation of missing long diagnostic events, raised the receiver log event-name limit, enlarged its line buffer, added a persistence regression test, and advanced package defaults to v0.1.29.

## Why this pass was needed

The v0.1.28 log stopped after ANNOUNCE offer fields even though source added Content-Type, RSA, crypto-input, and response events. Inspection found the logger silently rejected any event name longer than 24 characters.

## Files changed

- `src/observability/receiver_log.h`
- `src/observability/receiver_log.c`
- `tests/protocol/rtsp_listener_test.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/ARCHITECTURE.md`
- `docs/BUILD_PIPELINE.md`
- `docs/ROADMAP.md`
- `docs/RISKS.md`
- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_rpcs3_announce_v028_logger_limit.md`
- `docs/decisions/2026-11-07_1310_log_event_name_limit.md`

## Architecture / logic changes

Receiver diagnostics now accept event names up to 48 characters rather than 24. The file logger's formatting buffer grows from 128 to 160 bytes to accommodate longer names and the bounded timestamp/counter fields. The 16 KiB total file cap and character allowlist remain unchanged. The host receiver test verifies a 33-character Content-Type event is persisted and can be read back.

## Runtime behavior changes

The wire protocol and audio behavior are unchanged. v0.1.28 logs startup, OPTIONS 200, SDP parse success, validation start, and offer fields, but does not return an ANNOUNCE response. Its longer new phase labels were not persisted. v0.1.29 has not yet been built or replayed.

## Validation performed

- Replayed v0.1.28 in RPCS3: route/listeners ready, OPTIONS 200, ANNOUNCE SDP parsed, offer fields logged, sender timed out waiting for response.
- Full Lima host suite under ASan/UBSan passed on this candidate diff, including the new long-event persistence assertion.
- PPU protocol/receiver/codec/audio object compilation and type checks passed.
- v0.1.28 package validation passed. v0.1.29 standalone package build/runtime replay is pending.

## Immediate results

- Diagnosed why new phase events were silently absent: the shared logger imposed a 24-character event-name bound.
- Prior statements that RSA unwrap did not start were not supported by logs and have been corrected in current docs. The absence of `ANNOUNCE_VALIDATE_DONE` remains informative because its name is under the limit.
- No RTP packets, iPhone session, audible audio, display/controller result, or physical PS3 change occurred.

## Known limitations / follow-ups

- Commit/push this logger fix and documentation correction.
- Build v0.1.29 from the exact pushed revision and verify package metadata/hash.
- Replay the synthetic sender and confirm `ANNOUNCE_CONTENT_TYPE_CHECK_START/DONE`, `ANNOUNCE_KEY_UNWRAP_START/DONE` or invalid-input event, validation completion, and response construction are actually stored.
- Continue to SETUP/RECORD/RTP only if ANNOUNCE returns successfully.

## Risks introduced

The event-name field admits longer strings, increasing maximum log-line length; the line buffer is enlarged to 160 bytes, while total output remains capped at 16 KiB and the same safe-character allowlist applies. No network or system configuration changes are included. RPCS3 remains an emulator-only package on RTSP 5001.

## Decision record (embedded)

### Decision: Increase the logger event-name bound

- **Decision:** Permit event names up to 48 characters and grow the line buffer to 160 bytes.
- **Context:** v0.1.28 phase labels up to 33 characters were silently discarded under a 24-character cap.
- **Options considered:** Shorten each event name, change only the new events, or raise the shared event bound and test it.
- **Why this option was chosen:** It fixes the shared logger for current and future diagnostics while retaining existing safe-character validation and total byte cap.
- **Tradeoffs accepted:** Longer names use more of the bounded log; no unbounded allocation is introduced.

### Decision: Advance package version to 0.1.29

- **Decision:** Increment the patch version for the logger behavior fix.
- **Context:** v0.1.28 logs cannot support the intended stage-level analysis.
- **Options considered:** Reuse v0.1.28 or generate a distinguishable next package.
- **Why this option was chosen:** Exact source/log attribution is required for the replay.
- **Tradeoffs accepted:** v0.1.29 is consumed when packaged, even if the original ANNOUNCE stall remains.

## Next recommended step

Commit and push, build the exact v0.1.29 package from that revision, then replay the existing synthetic sender and inspect the now-persisted phases.
