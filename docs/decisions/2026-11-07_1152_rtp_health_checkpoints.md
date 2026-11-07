# Add bounded RTP health checkpoints

**Timestamp (America/New_York):** 2026-11-07 11:52 EST

## Scope of the pass

Added periodic privacy-safe RTP queue-health diagnostics, reset the diagnostic counter baseline on FLUSH and advanced the package version to v0.1.24. Extended the real socket loopback test to cross a 128-packet checkpoint and introduce a new sequence gap after FLUSH.

## Why this pass was needed

The receiver tracked packet loss and queue conditions in memory, but its capped log did not report most of those counts. A future iPhone log could show RTP receipt without showing whether the stream had packet loss or queue pressure.

## Files changed

- `src/protocol/raop_receiver.c`
- `src/protocol/raop_receiver.h`
- `tests/protocol/raop_receiver_test.c`
- `prototype/standalone-receiver/Makefile`
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
- `docs/decisions/2026-11-07_1152_rtp_health_checkpoints.md`

## Architecture / logic changes

Every 128 successfully parsed RTP audio packets since the last checkpoint or FLUSH, the receiver logs packet/payload deltas and current queue depth. Nonzero deltas are reported separately for missing, reordered, duplicate, late, and queue-dropped packets. Per-session checkpoint baselines reset on a new client; FLUSH snapshots current cumulative counters so prior-stream errors are not attributed to the post-seek stream. Internal RTP counters remain cumulative across FLUSH.

## Runtime behavior changes

No audio handling or packet acceptance semantics changed. Capped logs gain low-rate numeric health events; raw packet data, peer addresses, and audio content are not logged.

## Validation performed

- `bash scripts/build-lima-crypto.sh` in Lima, including the complete ASan/UBSan host suite and PPU component compilation.
- PASS: RTSP/RTP receiver loopback crosses the 128-packet checkpoint, verifies queue depth, introduces one sequence gap after FLUSH, and asserts one new missing packet is logged.
- PASS: full host suite includes crypto/session/SDP/mDNS, persistent RTSP and bounded slow-reader handling, encrypted RTP-to-PCM, ALAC mutation corpus, FFmpeg reference decode, PCM ring/resampling, gain/mute/ramp, and diagnostic tone.
- One initial test assertion failed because the fixture had not popped the baseline sequence-1 packet before checking subsequent frames. The fixture was corrected to drain that queued packet; the rerun passed. No product packet-handling defect was indicated.
- RPCS3 remained inaccessible: CUA reported the Mac locked even after the user attempted to unlock it. Package build is pending; no physical PS3 operation occurred.

## Immediate results

The host receiver now exposes loss and queue-health evidence at a bounded rate. The integration test verifies a discontinuity across FLUSH is counted relative to the new logging baseline.

## Known limitations / follow-ups

- Build and validate the standalone v0.1.24 ELF/PKG from the committed source.
- Test the exact package in RPCS3 when the desktop is accessible; the last accessible installed version/log was v0.1.15.
- Verify on an iPhone that RTP counters and decode/PCM events correspond to real sender traffic; host simulation does not prove interoperability or audible output.

## Risks introduced

- New periodic events consume part of the 16 KiB capped log. Error deltas are emitted only when nonzero; the cap still limits retention time during long streams.
- The emulator and PS3 runtime behavior remains unverified.

## Decision record (embedded)

### Emit aggregate health data rather than per-packet logs

- **Decision:** Log receiver packet and payload deltas, queue depth, and nonzero RTP error counter deltas every 128 successfully parsed audio packets.
- **Context:** Internal statistics existed but were not visible in hardware logs; per-packet logs would fill the capped file quickly.
- **Options considered:** Keep counters internal; log every RTP packet; or emit bounded aggregate checkpoints.
- **Why chosen:** Checkpoints reveal the first meaningful loss/queue boundary while limiting log volume and avoiding media data or peer identity.
- **Tradeoffs accepted:** A brief session ending before the next checkpoint may not show aggregate error counts.

### Reset only the logging baseline on FLUSH

- **Decision:** Snapshot cumulative counters at FLUSH without clearing the RTP statistics themselves.
- **Context:** A seek or track transition should not make old loss appear as a new-stream delta, while cumulative statistics remain useful to the running app.
- **Options considered:** Clear all statistics; leave the log baseline untouched; or advance only the diagnostic baseline.
- **Why chosen:** This keeps both per-stream diagnostic deltas and lifetime internal counters.
- **Tradeoffs accepted:** The log starts a fresh interval at FLUSH and does not itself report the pre-FLUSH total again.

### Advance AIRPLAY02 patch version to v0.1.24

- **Decision:** Use v0.1.24 for this observable behavior change.
- **Context:** Each runtime-relevant change must map to a distinguishable package.
- **Options considered:** Reuse v0.1.23 or increment the patch version.
- **Why chosen:** Future logs and artifacts must identify whether the new counters are present.
- **Tradeoffs accepted:** Package artifacts remain pending until the build pass.

## Next recommended step

Commit and push the tested logging pass, build and validate its standalone package, then run the exact candidate in RPCS3 when desktop access is available.
