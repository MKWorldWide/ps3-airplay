# Trace the ANNOUNCE Path After SDP Parsing

**Timestamp:** 2026-11-07 12:50 EDT (America/New_York)

## Scope of the pass

Replayed AIRPLAY02 v0.1.26 in RPCS3 using the synthetic cross-host RAOP sender, recorded the package/runtime evidence, and added v0.1.27 phase events for ANNOUNCE field classification, validation completion, and response construction.

## Why this pass was needed

The previous phase events established that SDP parsing returned success, but the client still received no ANNOUNCE response. The RSA key-unwrapping start event was absent, so evidence did not support attributing the stall to RSA.

## Files changed

- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `src/protocol/raop_session.c`
- `tests/protocol/raop_session_crypto_test.c`
- `docs/ARCHITECTURE.md`
- `docs/BUILD_PIPELINE.md`
- `docs/RISKS.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/lessons-learned.md`
- `docs/memory.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_rpcs3_announce_phase.md`
- `docs/decisions/2026-11-07_1250_announce_validation_trace.md`

## Architecture / logic changes

The optional session diagnostic callback now marks ANNOUNCE field classification, validation completion, and response formatting start/success/failure. The event payload contains only bounded numeric status/length information; SDP, keys, peer addresses, and media remain excluded. Package defaults advance to 0.1.27. The production RTSP port remains 5000.

## Runtime behavior changes

v0.1.26 RPCS3 evidence now separates the path into successful app/network/listener startup, OPTIONS 200, successful SDP parse, and a stall before response. RSA unwrap did not start. No SETUP/RECORD/RTP occurred. v0.1.27 has not yet been built or run.

## Validation performed

- Rebuilt the exact v0.1.26 source snapshot in Lima with PS3DK v0.27.0 and PSL1GHT; ELF and finalized PKG checks passed.
- Re-ran the full Lima host/ASan/UBSan suite and PPU component compilation; all passed.
- Replayed a synthetic OPTIONS/ANNOUNCE exchange against RPCS3. OPTIONS returned 200; ANNOUNCE SDP parse returned success; client timed out waiting for ANNOUNCE response.
- Preserved prior RPCS3 AIRPLAY02 title data before locally staging the validated v0.1.26 extraction. Desktop CUA remained locked, so no visual/controller observation was available.

## Immediate results

- Confirms parsing is not the last successful internal ANNOUNCE phase.
- RSA is not shown to be the blocked operation.
- Sender probe remains unable to reach SETUP/RECORD/RTP.
- No physical console files, settings, or boot plugins were changed.

## Known limitations / follow-ups

- Build v0.1.27 from the committed source, package with RTSP port 5001 for RPCS3, and replay the same sender.
- Inspect whether ANNOUNCE validation and response construction start/complete.
- Keep visual/controller testing pending until desktop access is available.
- Do not infer iPhone compatibility or audible AirPlay from synthetic host traffic.

## Risks introduced

Additional bounded log events consume a small portion of the existing 16 KiB cap per ANNOUNCE. No wire protocol behavior changes. The emulator-only package remains unsuitable for physical deployment due to its RTSP port override.

## Decision record (embedded)

### Decision: Add checkpoints rather than alter protocol behavior

- **Decision:** Instrument ANNOUNCE validation and response construction before attempting a behavior fix.
- **Context:** v0.1.26 confirms successful SDP parsing but has no response; RSA unwrap did not start.
- **Options considered:** Change RAOP compatibility responses; alter crypto; add more stage logging.
- **Why this option was chosen:** The first two are speculative without locating the last completed operation. More phase evidence can distinguish the validation branch from response formatting.
- **Tradeoffs accepted:** One additional package/replay cycle is required before a corrective behavior change.

### Decision: Use v0.1.27 for the diagnostic pass

- **Decision:** Increment the patch version for the new package candidate.
- **Context:** This is a source/runtime diagnostic change and should remain distinct from the v0.1.26 artifact.
- **Options considered:** Reuse v0.1.26 or advance the patch version.
- **Why this option was chosen:** Distinct package versioning keeps emulator logs and rollback artifacts attributable.
- **Tradeoffs accepted:** v0.1.27 is consumed when packaged, even if the diagnostic pass fails.

## Next recommended step

Commit and push this instrumentation pass, build the exact v0.1.27 package, then replay OPTIONS/ANNOUNCE and inspect all new phase events.
