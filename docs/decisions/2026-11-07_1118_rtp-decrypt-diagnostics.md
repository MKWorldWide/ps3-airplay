# Add bounded RTP decrypt success diagnostics

**Timestamp (EDT):** 2026-11-07 11:18

## Scope of the pass

Added a successful audio-payload decryption counter and low-rate `RTP_DECRYPT_OK` event, asserted the counter in the encrypted RTP-to-PCM integration test, and advanced the default package version to v0.1.22. Updated project memory, lessons, roadmap, test matrix, and versioning notes.

## Why this pass was needed

RTP packet reception and decrypt failure were logged, but successful decryption was implicit. A diagnostic log could not directly show whether an incoming packet passed the crypto stage before ALAC decode.

## Files changed

- `src/protocol/raop_receiver.h`
- `src/protocol/raop_receiver.c`
- `tests/protocol/raop_audio_pipeline_test.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/VERSIONING.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/decisions/2026-11-07_1118_rtp-decrypt-diagnostics.md`

## Architecture / logic changes

`raop_receiver` now maintains a saturating 64-bit count of audio payloads that successfully pass `raop_session_decrypt_audio`. It emits `RTP_DECRYPT_OK` on the first success and every 128 successes thereafter. The integer field is bounded at `INT_MAX` for the logger; payload length is recorded as the byte count. Existing `RTP_DECRYPT_FAILED` behavior remains unchanged. The counter is receiver-lifetime cumulative and contains no peer identity, key, or media data.

## Runtime behavior changes

When a sender streams packets, the capped diagnostic log now identifies the first successful decryption and then periodic progress checkpoints. Logging does not occur for every packet, limiting log growth and timing overhead.

## Validation performed

- Created a clean source snapshot from tracked and nonignored project files at `~/ps3-airplay-host/airplay-work-0.1.22` in the existing Lima VM.
- Ran `scripts/build-lima-crypto.sh` with PS3DK v0.27.0 and the existing crypto dependencies.
- PASS: crypto, session crypto/state, SDP, and mDNS tests.
- PASS: persistent RTSP + UDP RTP loopback.
- PASS: encrypted RTP-to-PCM pipeline; new assertion confirms exactly one successful decrypt for its fixture.
- PASS: ALAC negotiated sample-count bounds and malformed-input ASan/UBSan corpus (4,096 structured mutations + 256 random packets).
- PASS: RTSP listener/log, FFmpeg PCM reference, PCM ring/resampling, gain/mute/ramp, diagnostic tone checks.
- PASS: PPU object compilation, including the receiver with the added counter/log path.
- RPCS3 Computer Use still reports the Mac locked. The local app log is an older v0.1.15 record; this pass did not install/run v0.1.22. No physical PS3 changes.

## Immediate results

Host logs can now separate RTP receipt, successful decryption, and successful ALAC frame decode. The composed test exercises all three layers through PCM ring insertion. The full standalone ELF/PKG build is the next step.

## Known limitations / follow-ups

- A successful decrypt event does not prove the packet decodes or reaches audible output; retain separate ALAC and PCM counters.
- Run v0.1.22 in RPCS3 and capture a fresh package-matched log when desktop access returns, then attempt a controlled iPhone session.
- The user-reported app-visible result is not yet attributed to an exact package/platform; current renderer history already removes the blue diagnostic frame and display labels.

## Risks introduced

- Adds one 64-bit field to receiver state and a sparse log event. Counter overflow saturates instead of wrapping. No behavior change occurs in packet selection, decryption, or audio scheduling.

## Decision record (embedded)

### Count successes and checkpoint logs

- **Decision:** Count every successful decryption but emit only the first and each 128-packet milestone.
- **Context:** Per-packet logging would consume the capped diagnostic budget quickly; no success event left the crypto boundary ambiguous.
- **Options considered:** Log every packet; add no success observability; or add a cumulative count with sparse checkpoints.
- **Why chosen:** The checkpoint event proves progress with low log volume, and a counter assertion makes the success path testable.
- **Tradeoffs accepted:** The log does not contain a complete per-packet sequence history; the RTP queue already logs aggregate receipt, and packet-level capture is intentionally out of scope.

### Advance the package patch version to 0.1.22

- **Decision:** Use v0.1.22 for the next candidate.
- **Context:** The user requested unique patch versions for each package fix and reserved 0.2.0 for a stable standalone diagnostics application.
- **Options considered:** Reuse v0.1.21 or advance one patch step.
- **Why chosen:** v0.1.21 is already built and its bytes are recorded; v0.1.22 makes diagnostic behavior distinguishable.
- **Tradeoffs accepted:** The candidate still requires full app packaging and runtime validation.

## Next recommended step

Commit and push this source/host-test pass, build and validate the v0.1.22 ELF/PKG in Lima, then install/run only in RPCS3 when its desktop is accessible.
