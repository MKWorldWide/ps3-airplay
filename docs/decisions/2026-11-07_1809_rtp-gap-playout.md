# Bounded RTP gap recovery and playout

## Timestamp (EDT)

2026-11-07 18:09 EDT

## Scope of this pass

Compared our RAOP/RTP path with Shairport Sync and OpenAirPlay1, fixed immediate release across RTP sequence holes, added bounded frame-silence concealment and a large-discontinuity skip cap, added deterministic tests, and updated project roadmap/test/risk/report context. Package defaults advance from v0.1.38 to v0.1.39.

## Why this pass was needed

The physical v0.1.35 capture recorded 195 RTP gaps, 214 late packets, no retained recovery, no ALAC failures, and 86,106 active-stream PCM underflow frames. Code inspection found that the sorted queue released the first later packet without waiting for a missing sequence, making a subsequently received resend late. This is a plausible contributor to dropouts, not a proven sole cause.

## Files changed

- `prototype/standalone-receiver/Makefile`
- `prototype/standalone-receiver/README.md`
- `prototype/standalone-receiver/source/main.c`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `src/protocol/raop_receiver.c`
- `src/protocol/raop_rtp.c`
- `src/protocol/raop_rtp.h`
- `tests/protocol/raop_receiver_test.c`
- `tests/protocol/raop_rtp_test.c`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/RISKS.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_rtp_gap_and_reference_review.md`
- `docs/decisions/2026-11-07_1809_rtp-gap-playout.md`

## Architecture / logic changes

RTP queue pop now detects a positive sequence hole after the last played sequence, starts an 80 ms monotonic deadline, and leaves the queue intact so the negotiated resend receiver can insert the missing frame. If the sequence arrives before timeout, sorted playout continues normally. At timeout, each missing packet up to 128 becomes a zero-payload marker that the standalone app converts into exactly one negotiated ALAC frame of silence. A larger jump increments the skipped-gap counter and releases the next queued real frame instead of producing a prolonged silence backlog. All counters remain numeric and privacy-safe.

Reference receivers show two useful patterns: sequence-keyed reorder queues with explicit loss markers; and larger buffers that protect recovery at a latency/memory cost. We selected a short, fixed wait plus an explicit concealment limit because the PS3 queue and GameOS audio path are memory/latency constrained. Actual iPhone resend timing has not been measured, so 80 ms is an initial tuning value only.

## Runtime behavior changes

For a one or small multi-packet hole, receiver playout waits up to 80 ms for recovery; after that it feeds frame-duration silence to maintain output timing. For more than 128 missing sequence numbers, it skips to the next received frame after waiting rather than building unbounded silence latency. New log counters report gap waits, concealed frames, and skipped frames.

## Validation performed

- `git diff --check` passed before the final documentation update.
- `limactl shell s84d-vdfs ... bash scripts/build-lima-crypto.sh` passed with ASan/UBSan halt-on-error: crypto/session/SDP/mDNS, receiver socket and slow-reader tests, composed encrypted RTP→ALAC→PCM, ALAC reference and 4,352 malformed inputs, PCM ring/resampling/gain/tone, and PPU component compilation.
- RTP tests cover resend-before-deadline, deadline silence markers, late recovery rejection, and a 299-packet jump cap. Receiver socket integration tests verify the wait and silence marker.
- v0.1.38 emulator package test (source tree before the large-gap cap): finalized AIRPLAY02 / 00.38 package installed/launched, visible background, tone completion, encrypted RTSP/RAOP, 128 RTP fixture, decrypt, ALAC, PCM submissions, and clean START exit. The package SHA-256 is `882e68768901bdbb4f36775c45d69387250364e3e3a0eeb971bdc10e31d17596`.
- RPCS3 mDNS failed with error 125. Its no-loss fixture did not exercise gap concealment. No physical PS3 action occurred.
- Standalone v0.1.39 link/package/replay still pending after the source revision is committed.

## Immediate results

- The baseline synthetic fixture proves the previous version's package executes the negotiated encrypted receive/decode/PCM path and cleanly exits in RPCS3.
- Deterministic tests verify the new hole handling and bound.
- The real-world dropout cause remains uncertain until hardware metrics change and the user confirms listening results.

## Known limitations / follow-ups

- 80 ms is not measured against iPhone retry latency.
- RPCS3 test stream has no injected loss; no actual audible output has been confirmed.
- Full RTP timestamp scheduling, sender-clock mapping, and sample-clock drift correction remain incomplete.
- PS3 display/flicker, XMB resume, mDNS and iPhone discovery, physical retransmission, Spotify audio quality, and sustained playback remain hardware gates.

## Risks introduced

Silence concealment converts decoder corruption into bounded audible gaps; it does not recover content. Large-jump skipping may lose more audio than a larger wait but prevents stale backlog and excessive latency. A poor wait choice can still trade dropouts against latency. RPCS3 does not model the PS3 HDMI/display stack or hardware sound chain.

## Decision record

### Decision 1: Hold RTP sequence holes before advancing playout

- **Context:** Retransmitted packets can arrive after their originals; immediate pop turns valid replies into late drops.
- **Options considered:** Keep immediate release; increase the global prebuffer; hold each hole for a measured/timed interval.
- **Chosen:** Hold a hole for an initial fixed 80 ms while the existing resend receiver runs.
- **Why:** Targeted fix that permits recovery without increasing memory or changing all startup latency.
- **Tradeoff:** Adds up to 80 ms at each encountered gap; the actual retry timing must be measured.

### Decision 2: Substitute silence for expired missing ALAC frames and cap large gaps

- **Context:** A missing encrypted ALAC payload cannot be passed to the decoder; unbounded silence could create long delay after a sequence discontinuity.
- **Options considered:** Drop output time immediately; keep silence forever until the next packet; synthesize exact frame silence and cap concealment.
- **Chosen:** Output one frame-duration silence per missing frame, up to 128, then skip to the next real packet.
- **Why:** Preserves timing for ordinary losses and bounds latency during discontinuities.
- **Tradeoff:** Real audio is lost for each concealed/skipped frame; this cannot make a lossy network lossless.

## Next recommended step

Commit/push this source+documentation pass, then build v0.1.39 from that exact revision in Lima, validate the finalized package, install and replay it in RPCS3, and add a lossy target replay before declaring recovery behavior emulator-verified.
