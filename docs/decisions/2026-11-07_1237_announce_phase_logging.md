# Isolate the ANNOUNCE Stall

**Timestamp:** 2026-11-07 12:37 EST (America/New_York)

## Scope of the pass

Added bounded session-phase diagnostics for SDP parsing and RSA-wrapped key recovery, extended the encrypted RTP-to-PCM host fixture with a remote RPCS3 sender mode, fixed the receiver-test log read size, and advanced package defaults to v0.1.26. Updated project state and test documentation.

## Why this pass was needed

v0.1.25 RPCS3 answered OPTIONS but did not answer a complete synthetic ANNOUNCE within three seconds. Existing method/status logging showed where the request entered but not which session-handling stage stopped.

## Files changed

- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `src/protocol/raop_session.h`
- `src/protocol/raop_session.c`
- `src/protocol/raop_receiver.c`
- `tests/protocol/raop_audio_pipeline_test.c`
- `tests/protocol/raop_session_crypto_test.c`
- `tests/protocol/raop_receiver_test.c`
- `docs/BUILD_PIPELINE.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/RISKS.md`
- `docs/VERSIONING.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_rpcs3_announce_stall.md`
- `docs/decisions/2026-11-07_1237_announce_phase_logging.md`

## Architecture / logic changes

`raop_session` now accepts an optional privacy-safe diagnostic callback. The standalone receiver connects it to the capped log. ANNOUNCE handling emits SDP-parse and RSA-key-unwrapping entry/completion events with only status and byte counts; request bodies, keys, and peer details remain excluded. The host audio-pipeline fixture gains an opt-in remote mode for a synthetic end-to-end RTSP/RTP sequence. Version defaults advance to 0.1.26 while the production RTSP port remains 5000.

## Runtime behavior changes

No installed app was changed in this pass. The v0.1.25 RPCS3 test demonstrated route and listener readiness, OPTIONS 200, then an ANNOUNCE response timeout. The v0.1.26 source should expose whether SDP parsing or RSA unwrap returns before the response boundary.

## Validation performed

- Built and ran `scripts/build-lima-crypto.sh` in the existing Lima guest.
- PASS: crypto vectors, RSA/OAEP session-key recovery, diagnostic event order/result assertion, RTSP session transitions, SDP/mDNS serialization, receiver loopback and slow-reader disconnect, encrypted RTP-to-decrypt/ALAC/PCM fixture, 4,096 seeded ALAC mutations plus 256 random packets under ASan/UBSan, PCM ring/resampling/gain, and five-second tone.
- PASS: PPU component compilation and ELF type checks for receiver/codec/audio objects.
- Repeated the receiver suite after one failure. The failure was due to the test reading only 4 KiB of a capped 16 KiB log; increasing the buffer to the full configured cap made the full suite pass.
- v0.1.26 package and RPCS3 replay are pending; source package defaults are set to 0.1.26.
- No raw logs, private host identifiers, or physical-console state were committed.

## Immediate results

- The RPCS3 ANNOUNCE stall is not yet fixed; v0.1.26 instrumentation is intended to localize it.
- The full host/sanitizer and PPU component suite passes with the added callback and sender mode.
- The corrected host test now reads the entire capped diagnostic log.
- Physical PS3, display, controller exit, iPhone discovery, and audible streaming remain untested.

## Known limitations / follow-ups

- Build exact v0.1.26 package from the committed source with `RTSP_PORT=5001` for RPCS3.
- Retry the synthetic remote sequence and inspect SDP parse/key unwrap start/completion, RTSP status, RTP receive/decrypt, ALAC decode, and PCM events.
- Retry visible-screen/START checks only when RPCS3 desktop access is unlocked.
- Do not send the port-5001 emulator artifact to the physical PS3; production uses 5000.

## Risks introduced

The v0.1.26 emulator-only package will use TCP 5001 and is not a physical deployment candidate. Phase diagnostics add a few bounded entries per ANNOUNCE to the existing 16 KiB cap. The optional callback adds fields to the in-memory session structure but does not alter wire format.

## Decision record (embedded)

### Decision: Instrument within session handling

- **Decision:** Expose optional callbacks for SDP and RSA key-recovery phases rather than adding raw request logging or speculative protocol changes.
- **Context:** v0.1.25 showed `RTSP_ANNOUNCE` but no response or failure event.
- **Options considered:** Log full SDP/keys for diagnosis; rewrite ANNOUNCE handling; or add bounded stage events.
- **Why this option was chosen:** Stage events can localize the stall while preserving the existing parser/crypto logic and privacy boundaries.
- **Tradeoffs accepted:** A new package and RPCS3 replay are needed before the stall's exact cause is known.

### Decision: Use the existing synthetic encrypted fixture as a remote sender

- **Decision:** Extend the current integration test with an opt-in remote mode and 128 fixture RTP packets.
- **Context:** The app requires 128 packets before it decrypts/decodes; an OPTIONS-only probe does not test the media path.
- **Options considered:** Build a separate client with duplicated crypto fixtures or reuse the existing encrypted end-to-end fixture.
- **Why this option was chosen:** Reusing the tested fixture keeps crypto/ALAC data consistent and avoids a second source of test vectors.
- **Tradeoffs accepted:** Synthetic packets exercise protocol plumbing, not iPhone sender compatibility or audible hardware output.

## Next recommended step

Commit this pass, build v0.1.26 from that exact revision with emulator port 5001, and rerun the synthetic sender while collecting a fresh capped app log.
