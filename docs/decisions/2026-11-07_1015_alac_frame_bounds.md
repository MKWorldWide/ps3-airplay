# Bound ALAC frame sample counts

## Timestamp

2026-11-07 10:15 America/New_York (host reports EST)

## Scope of the pass

Added an ALAC decoder guard against invalid in-band sample counts and a composed encrypted RTP-to-reference-PCM host integration test. Updated project memory, lessons, scratchpad, roadmap, risks, and test matrix.

## Why this pass was needed

AddressSanitizer found that a 353-sample ALAC frame paired with a negotiated maximum of 352 samples could write past predictor arrays sized from that maximum. Existing separate crypto, RTP, and ALAC tests did not join those stages or exercise this mismatch.

## Files changed

- `src/codec/alac.c`
- `tests/protocol/raop_audio_pipeline_test.c`
- `scripts/build-lima-crypto.sh`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/TEST_MATRIX.md`
- `docs/ROADMAP.md`
- `docs/RISKS.md`
- `docs/decisions/2026-11-07_1015_alac_frame_bounds.md`

## Architecture / logic changes

Both mono and stereo ALAC branches now read the in-band sample count as unsigned and reject zero or above-negotiation counts before narrowing, size arithmetic, or count-indexed array operations. The host test constructs a synthetic ANNOUNCE with an RSA-OAEP-wrapped AES key, creates an AES-CBC encrypted reference ALAC payload in an RTP packet, sends it through the receiver's queue/decrypt API, and compares decoded PCM byte-for-byte with the checked-in reference. It also verifies safe rejection under a 352-sample limit and for `UINT32_MAX`.

## Runtime behavior changes

Malformed or incompatible ALAC frames with invalid sample counts are discarded by the decoder rather than being allowed to drive writes beyond negotiated sample buffers. Valid supported frames retain their existing decode behavior.

## Validation performed

- `scripts/build-lima-crypto.sh` in the existing x86_64 Lima guest, using the project archive plus the exact changed source/test/script files.
- Full host suite passed under AddressSanitizer and UndefinedBehaviorSanitizer, including RTSP/RTP loopback, crypto/session, ALAC golden decode, PCM, tone, and the new composed pipeline test.
- PPU object compilation passed for crypto, RTSP session/receiver, ALAC, and PCM components.
- RPCS3 UI access was attempted after the user reported unlocking the Mac; CUA still reported the Mac locked. No v0.1.19 runtime test occurred in this pass.

## Immediate results

- The first composed test run exposed the bounds defect under ASan.
- After the guard, the new test reports both 352/353 and `UINT32_MAX` rejection plus encrypted RTP-to-reference-PCM success.
- The final full host suite and PPU object checks pass.
- The standalone v0.1.19 application/package has not yet been built from the committed revision.

## Known limitations / follow-ups

- The sender is synthetic and the payload is a checked-in fixture; this is not iPhone interoperability evidence.
- Build v0.1.19 from the exact committed source, validate the CellOS ELF and finalized NPDRM package, and capture its size/hash.
- Try RPCS3 install/launch only when desktop UI access is available; inspect a fresh log for the exact tested package.
- Physical PS3 playback, receiver display/exit, AirPlay streaming, Spotify output, and game coexistence remain unverified.
- Add broader malformed ALAC corpus/fuzz coverage in a later bounded pass.

## Risks introduced

No new external interfaces or persistent console changes. The validation only covers the sample-count guard; malformed bitstream parsing has broader untested surfaces. Package/runtime compatibility is unchanged until the new build is tested.

## Decision record (embedded)

### Decision 1: Reject frame counts above negotiated maximum

- **Context:** The decoder sizes internal per-frame arrays from the SDP maximum but previously trusted a later in-band sample count.
- **Options considered:** Increase all arrays to a global maximum; clamp the count; reject invalid frames before decode.
- **Why chosen:** Rejection honors negotiated codec bounds, keeps allocation bounded, and prevents silently decoding a sender-invalid frame.
- **Tradeoffs accepted:** A sender that emits a frame beyond its own negotiated maximum will lose that frame and may experience a gap; the receiver does not attempt recovery by reallocating.

### Decision 2: Compose existing stages in a host integration test

- **Context:** Crypto, RTP, and ALAC each had standalone tests, but no single test proved that the receiver's decrypt handoff produces bytes accepted by the decoder.
- **Options considered:** Add only a decoder boundary unit test; add a synthetic sender path using the current APIs and fixture; make protocol changes based on assumed iPhone behavior.
- **Why chosen:** The composed fixture detects cross-component mismatches without changing wire behavior or requiring the locked emulator/physical console.
- **Tradeoffs accepted:** It proves deterministic component composition only and does not replace UDP sender, iPhone, GameOS, or hardware tests.

## Next recommended step

Commit and push this source/docs pass after the brief privacy gate, then build AIRPLAY02 v0.1.19 from that exact commit in Lima and validate its package. Retry RPCS3 after desktop unlock.
