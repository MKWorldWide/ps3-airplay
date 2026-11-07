# Preserve retransmission gaps and record first audible playback

## Timestamp in EDT

2026-11-07 14:58 EST (local Eastern time)

## Scope of the pass

Record fresh v0.1.33 hardware evidence and prepare the v0.1.34 source candidate. The first user-confirmed AirPlay music is audible, but crackly/unstable; the image flickers after XMB resume. Add bounded RTP resend queuing/tracking and lower high-volume log cadence while preserving aggregate health metrics.

## Why this pass was needed

The v0.1.33 retained PS3 log shows real encrypted RTP, successful ALAC decode and PCM generation, plus repeated RTP sequence gaps. Code inspection found that the 50 ms resend throttle discarded gaps observed during the interval and that only one outstanding range could be recovered. This is a concrete weakness and plausible crackle contributor, not proven sole cause.

## Files changed

- `src/protocol/raop_receiver.c`
- `src/protocol/raop_receiver.h`
- `tests/protocol/raop_receiver_test.c`
- `prototype/standalone-receiver/source/main.c`
- `prototype/standalone-receiver/Makefile`
- `scripts/build-lima-standalone-receiver.sh`
- `scripts/package-lima-standalone-receiver.sh`
- `docs/BUILD_PIPELINE.md`
- `docs/RISKS.md`
- `docs/ROADMAP.md`
- `docs/TEST_MATRIX.md`
- `docs/VERSIONING.md`
- `docs/memory.md`
- `docs/lessons-learned.md`
- `docs/scratchpad.md`
- `docs/reports/2026-11-07_v0133_first_airplay_playback.md`
- This decision log

## Architecture / logic changes

- Queue up to 16 pending missing sequence ranges while retaining the 50 ms resend pacing.
- Track up to 16 outstanding requests for one second; match retransmitted RTP packets against any active range and recover them into the ordered queue.
- Clear pending and tracked resend state on FLUSH, TEARDOWN, and disconnect.
- Reduce RTP, decrypt, and ALAC checkpoints from every 128 to every 512 packets; emit five-second missing and resend recovery deltas to keep a 16 KiB log useful longer.
- Advance package label to v0.1.34 and PS3 SFO to 00.34.

## Runtime behavior changes

A gap that arrives during the send throttle remains pending and is sent when the interval expires. Multiple responses may complete out of order while their request ranges remain valid. If all 16 pending slots fill, additional gaps are not queued; outstanding requests expire after one second. No screen or audio-output behavior is otherwise changed.

## Validation performed

- Ran the full Lima `scripts/build-lima-crypto.sh` suite from an isolated source snapshot.
- ASan/UBSan crypto, session, SDP, discovery, receiver socket, encrypted RTP-to-PCM, ALAC malformed-input, PCM ring/resampling/gain, and diagnostic-tone checks passed.
- Receiver socket regression sent two close gaps across the 50 ms throttle, waited for both request ranges, returned delayed packets, and verified ordered queue recovery.
- PPU receiver, crypto, session, ALAC, PCM and gain component objects compiled as big-endian PowerPC64 relocatables.
- Full standalone application ELF and PKG have not yet been built from the committed v0.1.34 source. No v0.1.34 install or hardware result is claimed.

## Immediate results

Host queue and delayed-response regression passes. The v0.1.33 hardware run produced the first user-confirmed AirPlay music and clean application exit, but crackling and screen flicker remain. Its wrapped log shows 106 sequence gaps in the visible window of at least 1,792 packets, zero sampled ALAC failures, and successful PCM frame flow.

## Known limitations / follow-ups

- Need commit and build the full v0.1.34 CellOS ELF and PKG, inspect SFO 00.34 and package contents, then install under standing authorization.
- Hardware-test a short song and compare RTP gaps, resend requests/recoveries, queue depth, underflows and perceived audio quality.
- Test PS-menu resume and stable display separately. The source log did not report presentation failure, but flicker cause is unknown.
- Do not conduct long playback soak until short-run audio and resume improve.

## Risks introduced

The pending and outstanding buffers are fixed-size and bounded, but a burst larger than 16 pending ranges may still lose recovery opportunities. One-second request expiry may be too short or too long for the sender. Retry behavior can increase duplicate RTP traffic. Keep sequence-wrap, FLUSH, teardown, and repeated gap/reordering coverage.

## Decision record (embedded)

- **Decision:** Queue gaps suppressed by rate limiting. **Context:** The hardware log shows many sequence gaps and the code discarded a gap if the send interval was not due. **Options:** Remove pacing, ignore the gap, or retain it in bounded memory. **Chosen:** Bounded queue with existing pacing. **Tradeoff:** A full queue rejects new gaps; pacing remains capped.
- **Decision:** Track multiple outstanding resend ranges. **Context:** Delayed replies could be rejected after a later request overwrote the single tracked range. **Options:** Keep one range or retain a fixed set. **Chosen:** 16 ranges with one-second expiry. **Tradeoff:** Fixed memory and possible expiry before late sender responses.
- **Decision:** Reduce per-packet diagnostic cadence and add five-second totals. **Context:** The 16 KiB log wrapped and removed connection setup context. **Options:** Increase the cap or retain lower-volume aggregates. **Chosen:** Lower event cadence with loss/recovery totals. **Tradeoff:** Less detail on individual packets.
- **Decision:** Keep display work separate. **Context:** Flicker appears after XMB resume, while audio failure correlates with RTP gaps. **Options:** Change framebuffer logic in the same build or isolate the transport patch. **Chosen:** Do not modify graphics in this pass. **Tradeoff:** Flicker remains unresolved pending a distinct measurement/fix.

## Next recommended step

Commit this source-and-evidence pass, run the privacy gate, then build/validate/install AIRPLAY02 v0.1.34 / SFO 00.34 and repeat one short AirPlay stream plus the PS-menu resume test.
